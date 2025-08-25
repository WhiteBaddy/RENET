#include "SigServer/SigConnection.h"
#include "SigServer/ConnectionManager.h"
#include <Tools/STLTools.h>

SigConnection::SPtr SigConnection::Create(SOCKET sockfd, const EventLoopSPtr &loop)
{
    auto ins = std::shared_ptr<SigConnection>(new SigConnection());
    ins->Init(sockfd, loop);
    return ins;
}

SigConnection::SigConnection()
    : TcpConnection(),
      role_state_(RoleState::None),
      code_(""),
      stream_address_(""),
      tcp_connection_(nullptr)
{
}

SigConnection::~SigConnection()
{
    Clear();
}
void SigConnection::Init(SOCKET sockfd, const EventLoopSPtr &loop)
{
    TcpConnection::Init(sockfd, loop);
    // 这里可以添加其他初始化逻辑
    SetReadCallback([this](const TcpConnection::SPtr &conn, std::unique_ptr<ReadBuffer> &buffer)
                    { return OnRead(buffer); });
    SetCloseCallback([this](const TcpConnection::SPtr &conn)
                     { DisConnect(); });
}

void SigConnection::DisConnect()
{
    fmt::print("SigConnection::DisConnect, code: {}, role_state: {}\n", code_, static_cast<int>(role_state_));
    Clear();
}

void SigConnection::AddCustomObject(const std::string &object)
{
    // 用户可能是拉流端，也可能是推流端
    // 这里不区分角色，直接添加
    // 避免重复添加
    if (objects_.find(object) != objects_.end())
        return;
    objects_.insert(object);
}

void SigConnection::RemoveCustomObject(const std::string &object)
{
    objects_.erase(object);
    // 如果没有对象了，说明当前没有控制端
    // 自己处于空闲状态
    if (objects_.empty())
    {
        role_state_ = RoleState::Idle;
    }
}

bool SigConnection::OnRead(std::unique_ptr<ReadBuffer> &buffer)
{
    // 处理读取到的数据
    while (buffer->ReadableBytes() > 0)
    {
        HandleMessage(buffer);
    }
    return true;
}

void SigConnection::HandleMessage(std::unique_ptr<ReadBuffer> &buffer)
{

    auto packet = Packet::Create(buffer->Peek(), buffer->ReadableBytes());
    if (!packet)
    {
        // 无法解析包，可能是数据不完整
        return;
    }
    buffer->Retrieve(packet->size()); // 从缓冲区中移除已处理的数据
    switch (packet->head.cmd)
    {
    case Cmd::Join:
        HandleJoin(packet);
        fmt::print("SigConnection::HandleMessage, Join command received\n");
        break;
    case Cmd::ObtainStream:
        HandleObtainStream(packet);
        fmt::print("SigConnection::HandleMessage, ObtainStream command received\n");
        break;
    case Cmd::CreateStreamReply:
        // 这里是推流端发送过来的应答
        HandleCreateStreamReply(packet);
        fmt::print("SigConnection::HandleMessage, CreateStreamReply command received\n");
        break;
    case Cmd::DeleteStream:
        HandleDeleteStream(packet);
        fmt::print("SigConnection::HandleMessage, DeleteStream command received\n");
        break;
    case Cmd::MouseButton:
    case Cmd::MouseMove:
    case Cmd::WheelScroll:
    case Cmd::KeyboardInput:
        HandleOtherMessage(packet);
        fmt::print("SigConnection::HandleMessage, Other command received\n");
        break;
    default:
        // 其他指令暂时不处理
        fmt::print("SigConnection::HandleMessage, Unknown command: {}\n", static_cast<int>(packet->head.cmd));
        break; // 未知命令，直接返回
    }
}

void SigConnection::Clear()
{
    role_state_ = RoleState::Closed;
    tcp_connection_.reset();
    DeleteStreamPacket deletePacket;
    for (auto &obj : objects_)
    {
        auto conn = ConnectionManager::GetInstance().GetConnection(obj);
        if (conn)
        {
            conn->RemoveCustomObject(code_);
            deletePacket.SetPlayerCount(conn->objects_.size());
            conn->Send(DataPayload::Create(deletePacket.Encode()));
        }
    }
    objects_.clear();
    ConnectionManager::GetInstance().RemoveConnection(code_);
    fmt::print("conn size: {}\n", ConnectionManager::GetInstance().GetConnectionCount());
}

void SigConnection::HandleJoin(const Packet::SPtr &packet)
{
    auto join_packet = std::dynamic_pointer_cast<JoinPacket>(packet);
    if (!join_packet)
        return;
    auto reply_packet = std::make_shared<JoinReplyPacket>();
    if (IsNoJion())
    {
        auto code = join_packet->GetRoomId();
        // 查询是否已经存在相同code的连接
        auto conn = ConnectionManager::GetInstance().GetConnection(code);
        if (conn)
        {
            reply_packet->SetResultState(ResultState::Error);
        }
        else
        {
            code_ = code;
            role_state_ = RoleState::Idle;
            // 添加
            ConnectionManager::GetInstance().AddConnection(code_, std::dynamic_pointer_cast<SigConnection>(shared_from_this()));
            fmt::print("Join count : {}\n", ConnectionManager::GetInstance().GetConnectionCount());
            reply_packet->SetResultState(ResultState::Successful);
        }
    }
    else
    {
        // 如果已经加入过了，直接返回
        reply_packet->SetResultState(ResultState::AlreadyRedistered);
    }
    Send(DataPayload::Create(reply_packet->Encode()));
}

void SigConnection::HandleCreateStreamReply(const Packet::SPtr &packet)
{
    auto create_reply_packet = std::dynamic_pointer_cast<CreateStreamReplyPacket>(packet);
    if (!create_reply_packet)
        return;
    return DoCreateStreamReply(create_reply_packet);
}

void SigConnection::HandleObtainStream(const Packet::SPtr &packet)
{
    auto obtain_packet = std::dynamic_pointer_cast<ObtainStreamPacket>(packet);
    if (!obtain_packet)
        return;
    return DoObtainStream(obtain_packet);
}

void SigConnection::HandleDeleteStream(const Packet::SPtr &packet)
{
    auto delete_packet = std::dynamic_pointer_cast<DeleteStreamPacket>(packet);
    if (!delete_packet)
        return;
    if (IsWorking())
    {
        Clear();
    }
}

void SigConnection::HandleOtherMessage(const Packet::SPtr &packet)
{
    // 处理其他消息
    fmt::print("SigConnection::HandleOtherMessage, command: {}\n", static_cast<int>(packet->head.cmd));
    // 这里可以添加其他处理逻辑
    // 如果是推流状态，对于这些消息直接进行转发
    if (tcp_connection_ && role_state_ == RoleState::Publishing)
    {
        Send(DataPayload::Create(packet->Encode()));
    }
}

void SigConnection::DoCreateStreamReply(const Packet::SPtr &packet)
{
    auto reply_packet = std::dynamic_pointer_cast<CreateStreamReplyPacket>(packet);
    if (!reply_packet)
        return;
    auto play_packet = std::make_shared<PlayStreamPacket>();
    stream_address_ = reply_packet->GetStreamUrl();
    for (auto idefy : iter_advance_range(objects_))
    {
        auto conn = ConnectionManager::GetInstance().GetConnection(idefy.value());
        if (!conn)
        {
            idefy.erase();
            continue;
        }
        if (stream_address_ == "")
        {
            fmt::print("stream_address_ is empty\n");
            conn->role_state_ = RoleState::Idle;
            reply_packet->SetResultState(ResultState::Error);
            conn->Send(DataPayload::Create(reply_packet->Encode()));
            continue;
        }

        switch (conn->GetRoleState())
        {
        case RoleState::None:
        case RoleState::Idle:
        case RoleState::Closed:
        case RoleState::Publishing:
            play_packet->SetResultState(ResultState::Error);
            RemoveCustomObject(conn->GetCode());
            conn->Send(DataPayload::Create(play_packet->Encode()));
            break;
        case RoleState::Playing:
            role_state_ = RoleState::Publishing;
            play_packet->SetResultState(ResultState::Successful);
            play_packet->SetStreamUrl(stream_address_);
            fmt::print("stream_address_ : {}\n", stream_address_);
            conn->Send(DataPayload::Create(play_packet->Encode()));
            break;
        default:
            break;
        }
    }

    // auto create_packet = std::dynamic_pointer_cast<CreateStreamPacket>(packet);
    // if (!create_packet)
    //     return;
    // auto play_packet = std::make_shared<PlayStreamPacket>();
    // auto reply_packet = std::make_shared<CreateStreamReplyPacket>();
    // fmt::print("reply_packet size : {}, stream_addr : {}\n", reply_packet->size(), stream_address_);

    // for (auto idefy : iter_advance_range(objects_))
    // {
    //     auto conn = ConnectionManager::GetInstance().GetConnection(idefy.value());
    //     if (!conn)
    //     {
    //         RemoveCustomObject(idefy.value());
    //         continue;
    //     }
    //     auto sig_conn = std::dynamic_pointer_cast<SigConnection>(conn);
    //     if (!sig_conn)
    //     {
    //         return;
    //     }
    //     // 流地址异常
    //     if (stream_address_.empty())
    //     {
    //         fmt::print("stream_address_ is empty\n");
    //         sig_conn->role_state_ = RoleState::Idle;
    //         reply_packet->SetResultState(ResultState::Error);
    //         sig_conn->Send(DataPayload::Create(reply_packet->Encode()));
    //         continue;
    //     }
    //     switch (sig_conn->GetRoleState())
    //     {
    //     case RoleState::None:
    //     case RoleState::Idle:
    //     case RoleState::Closed:
    //     case RoleState::Publishing:
    //         play_packet->SetResultState(ResultState::Error);
    //         RemoveCustomObject(sig_conn->GetCode());
    //         sig_conn->Send(DataPayload::Create(play_packet->Encode()));
    //         break;
    //     case RoleState::Playing:
    //         role_state_ = RoleState::Publishing;
    //         play_packet->SetResultState(ResultState::Successful);
    //         play_packet->SetStreamUrl(stream_address_);
    //         fmt::print("stream_address_ : {}\n", stream_address_);
    //         sig_conn->Send(DataPayload::Create(play_packet->Encode()));
    //         break;
    //     default:
    //         break;
    //     }
    // }

    // for (auto &idefy : objects_)
    // {
    //     auto conn = ConnectionManager::GetInstance().GetConnection(idefy);
    //     if (!conn)
    //     {
    //         RemoveCustomObject(idefy);
    //         continue;
    //     }
    //     auto sig_conn = std::dynamic_pointer_cast<SigConnection>(conn);
    //     if (!sig_conn)
    //     {
    //         return;
    //     }
    //     // 流地址异常
    //     if (stream_address_.empty())
    //     {
    //         fmt::print("stream_address_ is empty\n");
    //         sig_conn->role_state_ = RoleState::Idle;
    //         play_packet->SetResultState(ResultState::Error);
    //         sig_conn->Send(DataPayload::Create(play_packet->Encode()));
    //         continue;
    //     }
    //     switch (sig_conn->GetRoleState())
    //     {
    //     case RoleState::None:
    //     case RoleState::Idle:
    //     case RoleState::Closed:
    //     case RoleState::Publishing:
    //         play_packet->SetResultState(ResultState::Error);
    //         RemoveCustomObject(sig_conn->GetCode());
    //         sig_conn->Send(DataPayload::Create(play_packet->Encode()));
    //         break;
    //     case RoleState::Playing:
    //         role_state_ = RoleState::Publishing;
    //         play_packet->SetResultState(ResultState::Successful);
    //         play_packet->SetStreamUrl(stream_address_);
    //         fmt::print("stream_address_ : {}\n", stream_address_);
    //         sig_conn->Send(DataPayload::Create(play_packet->Encode()));
    //         break;
    //     default:
    //         break;
    //     }
    // }
}

void SigConnection::DoObtainStream(const Packet::SPtr &packet)
{
    auto obtain_packet = std::dynamic_pointer_cast<ObtainStreamPacket>(packet);
    if (!obtain_packet)
        return;

    auto reply_packet = std::make_shared<ObtainStreamReplyPacket>();
    auto create_packet = std::make_shared<CreateStreamPacket>();
    auto code = obtain_packet->GetStreamId();
    auto conn = ConnectionManager::GetInstance().GetConnection(code);
    if (!conn)
    {
        // 如果不存在，说明流ID错误
        reply_packet->SetResultState(ResultState::Error);
        Send(DataPayload::Create(reply_packet->Encode()));
        return;
    }
    if (conn == shared_from_this())
    {
        fmt::print("控制对象为自己，拒绝拉流请求, stream_id: {}\n", code);
        reply_packet->SetResultState(ResultState::Error);
        Send(DataPayload::Create(reply_packet->Encode()));
        return;
    }
    if (IsIdle())
    {
        // 如果当前是空闲状态，说明可以拉流
        auto sig_conn = std::dynamic_pointer_cast<SigConnection>(conn);
        switch (sig_conn->GetRoleState())
        {
        case RoleState::None:
            fmt::print("目标未建立房间，拒绝拉流请求\n");
            reply_packet->SetResultState(ResultState::Error);
            Send(DataPayload::Create(reply_packet->Encode()));
            break;
        case RoleState::Idle:
            // 目标空闲，可以推流
            fmt::print("目标空闲\n");
            role_state_ = RoleState::Playing;
            AddCustomObject(code);            // 添加被控对象
            sig_conn->AddCustomObject(code_); // 被控端添加控制对象
            reply_packet->SetResultState(ResultState::Successful);
            tcp_connection_ = conn;
            // 通知被控端创建流
            sig_conn->Send(DataPayload::Create(create_packet->Encode()));
            Send(DataPayload::Create(reply_packet->Encode()));
            fmt::print("SigConnection::DoObtainStream, stream_id: {}, role_state: {}\n", code, static_cast<int>(role_state_));
            break;
        case RoleState::Publishing:
            // 目标正在推流，说明是被控端，可以进行拉流
            if (sig_conn->GetStreamAddress().empty())
            {
                fmt::print("目标正在推流，但流地址为空，拒绝拉流请求\n");
                reply_packet->SetResultState(ResultState::Error);
                Send(DataPayload::Create(reply_packet->Encode()));
                return;
            }
            else
            {
                fmt::print("目标正在推流\n");
                role_state_ = RoleState::Playing;
                AddCustomObject(code);            // 添加被控对象
                sig_conn->AddCustomObject(code_); // 被控端添加控制对象
                // 流已经存在，可以直接进行拉流播放
                auto play_packet = std::make_shared<PlayStreamPacket>();
                play_packet->SetResultState(ResultState::Successful);
                play_packet->SetStreamUrl(sig_conn->GetStreamAddress());
                fmt::print("SigConnection::DoObtainStream, stream_id: {}, stream_address: {}\n", code, sig_conn->GetStreamAddress());
                Send(DataPayload::Create(play_packet->Encode()));
            }
        case RoleState::Playing:
            // 目标正在拉流，说明是控制端，不能进行拉流
            fmt::print("目标正在拉流，拒绝拉流请求\n");
            reply_packet->SetResultState(ResultState::Error);
            Send(DataPayload::Create(reply_packet->Encode()));
            break;
        case RoleState::Closed:
            // 目标已经关闭，拒绝拉流请求
            fmt::print("目标已经关闭，拒绝拉流请求\n");
            reply_packet->SetResultState(ResultState::Error);
            Send(DataPayload::Create(reply_packet->Encode()));
            break;
        default:
            // 其他状态，拒绝拉流请求
            fmt::print("未知状态，拒绝拉流请求\n");
            reply_packet->SetResultState(ResultState::Error);
            Send(DataPayload::Create(reply_packet->Encode()));
            break;
        }
    }
    else
    {
        // 如果自身不是空闲状态，说明已经在推流或者拉流，拒绝请求
        reply_packet->SetResultState(ResultState::Error);
        Send(DataPayload::Create(reply_packet->Encode()));
        return;
    }
}

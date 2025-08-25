
#include "RtmpConnection.h"
#include "RtmpServer.h"
#include "Session.h"

RtmpConnection::SPtr RtmpConnection::Create(const RtmpServerSPtr &server, const EventLoopSPtr &eventloop, SOCKET sockfd)
{
    auto ins = std::make_shared<RtmpConnection>();
    ins->Init(server, eventloop, sockfd);
    return ins;
}
RtmpConnection::RtmpConnection()
    : peer_width_(5 * 1000 * 1000), state_(State::Handshake),
      acknowledgement_size_(5 * 1000 * 1000),
      max_chunk_size_(128), stream_id_(0),
      is_audio_playing_(false), is_video_playing_(false), is_publishing_(false),
      has_key_frame_(false) {}
RtmpConnection::~RtmpConnection() = default;

void RtmpConnection::Init(const RtmpServerSPtr &server, const EventLoopSPtr &eventloop, SOCKET sockfd)
{
    TcpConnection::Init(sockfd, eventloop);
    handshake_ = ServerHandshake::Create();
    rtmp_message_manager_ = RtmpMessageManager::Create();
    rtmp_server_ = server;

    peer_width_ = server->GetPeerBandwidth();
    acknowledgement_size_ = server->GetAcknowledgementSize();
    max_chunk_size_ = server->GetChunkSize();
    stream_path_ = server->GetStreamPath();
    stream_name_ = server->GetStreamName();
    app_ = server->GetApp();

    SetReadCallback([this](const TcpConnection::SPtr &conn, std::unique_ptr<ReadBuffer> &buffer)
                    { return OnRead(buffer); });
    SetCloseCallback([this](const TcpConnection::SPtr &conn)
                     { OnClose(); });

    // 握手
    // Send(std::make_shared<std::vector<char>>(handshake_->BuildC0C1()));
}

bool RtmpConnection::IsPlayer() const { return state_ == State::Playing; }
bool RtmpConnection::IsPublisher() const { return state_ == State::Publishing; }
bool RtmpConnection::IsAudioPlaying() const { return is_audio_playing_; }
bool RtmpConnection::IsVideoPlaying() const { return is_video_playing_; }
bool RtmpConnection::IsPublishing() const { return is_publishing_; }
uint32_t RtmpConnection::GetId() const { return GetSocket(); }

bool RtmpConnection::OnRead(std::unique_ptr<ReadBuffer> &readBuf)
{
    bool ret = true;
    // 是否握手完成，完成后才能发送msg
    // if (handshake_->IsComplete())
    // {
    //     ret = HandleChunk(readBuf);
    // }
    // else
    // {
    //     int resSize = handshake_->Parse((uint8_t *)readBuf->Peek(), readBuf->ReadableBytes());
    //     if (resSize < 0)
    //     {
    //         ret = false;
    //     }
    //     else if (resSize > 0)
    //     {
    //         readBuf->Retrieve(resSize);
    //         Send(std::make_shared<std::vector<char>>(handshake_->BuildC2()));
    //     }
    //     if (handshake_->IsComplete())
    //     {
    //         if (readBuf->ReadableBytes() > 0)
    //         {
    //             ret = HandleChunk(readBuf);
    //         }
    //     }
    // }
    if (handshake_->IsError())
    {
        return false;
    }
    if (!handshake_->IsComplete())
    {
        int resSize = handshake_->Parse((uint8_t *)readBuf->Peek(), readBuf->ReadableBytes());
        if (resSize < 0)
        {
            ret = false;
        }
        else if (resSize > 0)
        {
            readBuf->Retrieve(resSize);
            if (!handshake_->IsError() && !handshake_->IsComplete())
            {
                auto tmp = handshake_->BuildS0S1S2();
                Send(DataPayload::Create(tmp));
            }
        }
    }
    if (handshake_->IsComplete())
    {
        if (readBuf->ReadableBytes() > 0)
        {
            ret = HandleChunk(readBuf);
        }
    }
    return ret;
}
void RtmpConnection::OnClose() { HandleDeleteStream(stream_id_, 0); }

bool RtmpConnection::HandleChunk(std::unique_ptr<ReadBuffer> &readBuf)
{
    for (int ret = -1; readBuf->ReadableBytes() > 0;)
    {
        ret = rtmp_message_manager_->ParseOnChunk((uint8_t *)readBuf->Peek(), readBuf->ReadableBytes());
        if (ret < 0)
            return false;
        else if (ret == 0)
            break;
        else
        {
            readBuf->Retrieve(ret); // 刷新一下缓冲区
            if (rtmp_message_manager_->HasMsg())
            {
                auto msg = rtmp_message_manager_->GetMsg();
                if (!HandleMessage(msg))
                {
                    return false;
                }
            }
        }
    }
    return true;
}

bool RtmpConnection::HandleMessage(RtmpMessage &rtmpMsg)
{
    bool ret = true;
    switch (rtmpMsg.header.typeId)
    {
    case RtmpMessage::Type::Video:
        ret = HandleVideo(rtmpMsg);
        break;
    case RtmpMessage::Type::Audio:
        ret = HandleAudio(rtmpMsg);
        break;
    case RtmpMessage::Type::InvokeAMF0:
        ret = HandleInvoke(rtmpMsg);
        break;
    case RtmpMessage::Type::DataAMF0:
        ret = HandleNotify(rtmpMsg);
        break;
    case RtmpMessage::Type::SetChunkSize:
    {
        int offset = 0;
        rtmp_message_manager_->SetInChunkSize(NumberCodec::DecodeU32(rtmpMsg.body->data(), rtmpMsg.body->size(), offset));
        break;
    }
    case RtmpMessage::Type::UserControl:
        break;
    }
    return ret;
}
bool RtmpConnection::HandleInvoke(RtmpMessage &rtmpMsg)
{
    bool ret = true;
    int offset = 0;
    auto array = AmfValue::DecodeArray(rtmpMsg.body->data(), rtmpMsg.body->size(), offset);
    if (array.size() <= 0 ||
        array[0]->type != AmfValue::AmfType::String)
    {
        return false;
    }
    for (auto &amf : array)
    {
        // fmt::print("{}\n", amf->to_string());
    }
    auto method = std::dynamic_pointer_cast<AmfString>(array[0])->value;
    // 判断流是否创建？为什么判断是否为0？
    // -->streamid由服务器创建并发送给客户端，在这之前，使用0通道
    if (rtmpMsg.header.streamId == 0)
    {
        if (method == "connect")
        {
            auto transaction_id = std::dynamic_pointer_cast<AmfNumber>(array[1])->value;
            connect_metadata_ = std::dynamic_pointer_cast<AmfObject>(array[2]);
            ret = HandleConnect(static_cast<uint32_t>(transaction_id));
        }
        // 客户端请求服务端进行 创建流前的清理，以及预创建流,只要回应表示收到消息即可
        else if (method == "releaseStream" || method == "FCPublish")
        {
            auto transaction_id = std::dynamic_pointer_cast<AmfNumber>(array[1])->value;
            // ret = HandleReleaseStream(transaction_id);
            ret = HandleSendResult(transaction_id);
        }
        else if (method == "createStream")
        {
            auto transaction_id = std::dynamic_pointer_cast<AmfNumber>(array[1])->value;
            ret = HandleCreateStream(static_cast<uint32_t>(transaction_id));
        }
        else if (method == "getStreamLength")
        {
            auto transaction_id = std::dynamic_pointer_cast<AmfNumber>(array[1])->value;
            ret = HandleGetStreamLength(transaction_id);
        }
    }
    else if (rtmpMsg.header.streamId == stream_id_)
    {
        // 下面注释掉的代码解析逻辑完全错误，需要重写
        // if (array.size() <= 1 ||
        //     array[1]->type != AmfValue::AmfType::String)
        // {
        //     return false;
        // }
        // stream_name_ = std::dynamic_pointer_cast<AmfString>(array[1])->value;
        // stream_path_ = fmt::format("/{0}/{1}", app_, stream_name_);
        // if (method == "publish")
        // {
        //     ret = HandlePublish();
        // }
        // else if (method == "play")
        // {
        //     ret = HandlePlay();
        // }
        // else if (method == "DeleteStream")
        // {
        //     ret = HandleDeleteStream();
        // }
        if (method == "publish")
        {
            auto transaction_id = std::dynamic_pointer_cast<AmfNumber>(array[1])->value;
            // array[2] 假设为空
            stream_name_ = std::dynamic_pointer_cast<AmfString>(array[3])->value;
            stream_path_ = fmt::format("/{0}/{1}", app_, stream_name_);
            // 忽略 array[4] : publishtype
            ret = HandlePublish(transaction_id);
        }
        else if (method == "play")
        {
            auto transaction_id = std::dynamic_pointer_cast<AmfNumber>(array[1])->value;
            stream_name_ = std::dynamic_pointer_cast<AmfString>(array[3])->value;
            stream_path_ = fmt::format("/{0}/{1}", app_, stream_name_);
            ret = HandlePlay(transaction_id);
        }
        else if (method == "deleteStream")
        {
            auto transaction_id = std::dynamic_pointer_cast<AmfNumber>(array[1])->value;
            // array[2] 假设为空
            auto deleted_stream_id = std::dynamic_pointer_cast<AmfNumber>(array[3])->value;
            ret = HandleDeleteStream(transaction_id, deleted_stream_id);
        }
    }
    return ret;
}
bool RtmpConnection::HandleNotify(RtmpMessage &rtmpMsg)
{
    auto server = rtmp_server_.lock();
    auto session = rtmp_session_.lock();
    if (server == nullptr || session == nullptr)
    {
        return false;
    }

    bool ret = true;
    int offset = 0;
    auto array = AmfValue::DecodeArray(rtmpMsg.body->data(), rtmpMsg.body->size(), offset);
    if (array.size() <= 0 ||
        array[0]->type != AmfValue::AmfType::String)
    {
        return false;
    }
    auto method = std::dynamic_pointer_cast<AmfString>(array[0])->value;
    if (method == "@setDataFrame")
    {
        if (array.size() <= 1 ||
            array[1]->type != AmfValue::AmfType::String)
        {
            return false;
        }
        auto ctrl = std::dynamic_pointer_cast<AmfString>(array[1])->value;
        if (ctrl == "onMetaData")
        {
            if (array.size() <= 2 ||
                (array[2]->type != AmfValue::AmfType::Object &&
                 array[2]->type != AmfValue::AmfType::EcmaArray))
            {
                return false;
            }
            if (array[2]->type == AmfValue::AmfType::Object)
            {
                auto metadata = std::dynamic_pointer_cast<AmfObject>(array[2]);
                stream_metadata_ = std::make_shared<AmfEcmaArray>();
                for (auto &it : metadata->value)
                {
                    stream_metadata_->value.insert(it);
                }
            }
            else
            {
                stream_metadata_ = std::dynamic_pointer_cast<AmfEcmaArray>(array[2]);
            }
            // fmt::print("stream_metadata_ : {}\n", stream_metadata_->to_string());
        }
        session->SendMetaData(stream_metadata_);
    }
    return true;
}
bool RtmpConnection::HandleAudio(RtmpMessage &rtmpMsg)
{ // 只考虑aac
    auto server = rtmp_server_.lock();
    auto session = rtmp_session_.lock();
    if (server == nullptr || session == nullptr)
    {
        return false;
    }
    // static int cnt = 0;
    // if (++cnt % 1 == 0)
    // {
    //     fmt::print("cnt:{}, HandleAudio, rtmpMsg.body size: {}, timestamp: {}\n", cnt, rtmpMsg.body->size(), rtmpMsg.header.timestamp);
    //     Debug::ShowHex(rtmpMsg.body->data(), rtmpMsg.body->size());
    // }

    auto bytes = rtmpMsg.body->at(0);
    auto sound_fmt = static_cast<RtmpAudioCodecId>((bytes >> 4) & 0x0F);
    auto type = RtmpAudioFrameType::RawData;
    if (sound_fmt == RtmpAudioCodecId::AAC && rtmpMsg.body->at(1) == 0)
    {
        aac_sequence_header_ = rtmpMsg.body;
        session->SetAacSequenceHeader(aac_sequence_header_);
        type = RtmpAudioFrameType::SequenceHeader;
    }
    session->SendAudioData(type, rtmpMsg.header.timestamp, rtmpMsg.body);
    return true;
}
bool RtmpConnection::HandleVideo(RtmpMessage &rtmpMsg)
{ // 只考虑h264
    auto server = rtmp_server_.lock();
    auto session = rtmp_session_.lock();
    if (server == nullptr || session == nullptr)
    {
        return false;
    }

    auto bytes = rtmpMsg.body->at(0);
    auto frame_type = static_cast<RtmpVideoFrameType>((bytes >> 4) & 0x0F);
    auto code_id = static_cast<RtmpVideoCodecId>(bytes & 0x0F);
    if (code_id == RtmpVideoCodecId::H264 && rtmpMsg.body->at(1) == 0)
    {
        avc_sequence_header_ = rtmpMsg.body;
        session->SetAvcSequenceHeader(avc_sequence_header_);
        frame_type = RtmpVideoFrameType::SequenceHeader;
    }

    session->SendVideoData(frame_type, rtmpMsg.header.timestamp, rtmpMsg.body);
    return true;
}

bool RtmpConnection::HandleConnect(uint32_t transaction_id)
{
    if (connect_metadata_->value.count("app") == 0 ||
        connect_metadata_->value["app"]->type != AmfValue::AmfType::String)
    {
        return false;
    }
    app_ = std::dynamic_pointer_cast<AmfString>(connect_metadata_->value["app"])->value;
    if (app_ == "")
        return false;
    SendAcknowledgement();
    SetPeerBandWidth();
    SetOutChunkSize(max_chunk_size_);

    AmfValue::Array array;
    array.push_back(std::make_shared<AmfString>("_result"));
    array.push_back(std::make_shared<AmfNumber>(transaction_id));

    auto obj1 = std::make_shared<AmfObject>();
    obj1->value["fmsVer"] = std::make_shared<AmfString>("FMS/4,5,0,297");
    obj1->value["capabilities"] = std::make_shared<AmfNumber>(255.0);
    obj1->value["mode"] = std::make_shared<AmfNumber>(1.0);
    array.push_back(obj1);

    auto obj2 = std::make_shared<AmfObject>();
    obj2->value["level"] = std::make_shared<AmfString>("status");
    obj2->value["code"] = std::make_shared<AmfString>("NetConnection.Connect.Success");
    obj2->value["description"] = std::make_shared<AmfString>("Connection succeed");
    obj2->value["objectEncoding"] = std::make_shared<AmfNumber>(0.0);
    array.push_back(obj2);

    std::vector<uint8_t> arrayEncode;
    AmfValue::EncodeArray(arrayEncode, array);
    auto ret = SendCtrlMsg(RtmpMessage::Type::InvokeAMF0, arrayEncode);
    AmfValue::to_string(array);
    // fmt::print("HandleConnect, \n");
    return ret;
}
// bool RtmpConnection::HandleReleaseStream(uint32_t transaction_id)
// {
//     AmfValue::Array array;
//     array.push_back(std::make_shared<AmfString>("_result"));
//     array.push_back(std::make_shared<AmfNumber>(transaction_id));
//     array.push_back(std::make_shared<AmfNull>());
//     array.push_back(std::make_shared<AmfNull>());

//     std::vector<uint8_t> arrayEncode;
//     AmfValue::EncodeArray(arrayEncode, array);
//     auto ret = SendCtrlMsg(RtmpMessage::Type::InvokeAMF0, arrayEncode);

//     // fmt::print("HandleReleaseStream\n");
//     return ret;
// }
bool RtmpConnection::HandleCreateStream(uint32_t transaction_id)
{
    auto server = rtmp_server_.lock();
    if (server == nullptr)
    {
        return false;
    }
    // 开始创建流，由服务器给分配一个非0流id
    stream_id_ = server->ApplyStreamId();
    AmfValue::Array array;
    array.push_back(std::make_shared<AmfString>("_result"));
    array.push_back(std::make_shared<AmfNumber>(transaction_id));
    array.push_back(std::make_shared<AmfNull>());
    array.push_back(std::make_shared<AmfNumber>(stream_id_));

    std::vector<uint8_t> arrayEncode;
    AmfValue::EncodeArray(arrayEncode, array);
    auto ret = SendCtrlMsg(RtmpMessage::Type::InvokeAMF0, arrayEncode);
    AmfValue::to_string(array);
    // fmt::print("HandleCreateStream\n");
    return ret;
}
bool RtmpConnection::HandlePublish(uint32_t transaction_id)
{
    auto server = rtmp_server_.lock();
    // auto session = rtmp_session_.lock();
    if (server == nullptr /* || session == nullptr*/)
    {
        return false;
    }

    AmfValue::Array array;
    array.push_back(std::make_shared<AmfString>("onStatus"));
    array.push_back(std::make_shared<AmfNumber>(transaction_id));
    array.push_back(std::make_shared<AmfNull>());

    bool isError = false;
    auto obj = std::make_shared<AmfObject>();
    if (server->HasPublisher(stream_path_))
    { // 流已经存在
        // fmt::print("HasPublisher\n");
        isError = true;
        obj->value["level"] = std::make_shared<AmfString>("error");
        obj->value["code"] = std::make_shared<AmfString>("NetStream.Publish.BadName");
        obj->value["description"] = std::make_shared<AmfString>("Stream already publishing.");
    }
    else if (state_ == State::Publishing)
    { // 已经开始推流了
        // fmt::print("publishing\n");
        isError = true;
        obj->value["level"] = std::make_shared<AmfString>("error");
        obj->value["code"] = std::make_shared<AmfString>("NetStream.Publish.BadConnection");
        obj->value["description"] = std::make_shared<AmfString>("Stream already publishing.");
    }
    else
    { // 成功开始推流
        obj->value["level"] = std::make_shared<AmfString>("status");
        obj->value["code"] = std::make_shared<AmfString>("NetStream.Publish.Start");
        obj->value["description"] = std::make_shared<AmfString>("Start publishing.");

        server->AddSession(stream_path_);
        rtmp_session_ = server->GetSession(stream_path_);
        server->NotifyEvent("publish.start", stream_path_);
    }

    array.push_back(obj);
    AmfValue::to_string(array);
    // 返回回应消息
    std::vector<uint8_t> arrayEncode;
    AmfValue::EncodeArray(arrayEncode, array);
    auto ret = SendCtrlMsg(RtmpMessage::Type::InvokeAMF0, arrayEncode);
    if (isError || !ret)
        return false;

    // 更新推流状态记录
    state_ = State::Publishing;
    is_publishing_ = true;

    // 把推流者添加到 session
    auto session = rtmp_session_.lock();
    if (session == nullptr)
    {
        return false;
    }
    session->AddSink(std::dynamic_pointer_cast<RtmpSink>(shared_from_this()));
    AmfValue::to_string(array);
    // fmt::print("HandlePublish\n");
    return true;
}
bool RtmpConnection::HandlePlay(uint32_t transaction_id)
{
    auto server = rtmp_server_.lock();
    // auto session = rtmp_session_.lock();
    if (server == nullptr /* || session == nullptr*/)
    {
        return false;
    }

    // 更新状态
    state_ = State::Playing;
    // 添加到 session
    rtmp_session_ = server->GetSession(stream_path_); // 获取到指定 session
    auto session = rtmp_session_.lock();
    if (session == nullptr)
    {
        return false;
    }

    // 回应一个 streamBegin
    auto msg = std::make_shared<RtmpMessage>();
    msg->header.timestamp = 0;
    msg->header.length = 6;
    msg->header.typeId = RtmpMessage::Type::UserControl;
    msg->header.streamId = 0;
    std::vector<uint8_t> data;
    NumberCodec::EncodeU16LE(data, 0);
    NumberCodec::EncodeU32LE(data, stream_id_);
    msg->body = DataPayload::Create(data);
    SendMsg(msg);

    AmfValue::Array array;
    array.push_back(std::make_shared<AmfString>("onStatus"));
    array.push_back(std::make_shared<AmfNumber>(0));
    array.push_back(std::make_shared<AmfNull>());
    auto obj = std::make_shared<AmfObject>();
    obj->value["level"] = std::make_shared<AmfString>("status");
    obj->value["code"] = std::make_shared<AmfString>("NetStream.Play.Reset");
    obj->value["description"] = std::make_shared<AmfString>("Resetting and playing stream.");
    array.push_back(obj);
    AmfValue::to_string(array);
    std::vector<uint8_t> arrayEncode;
    AmfValue::EncodeArray(arrayEncode, array);
    if (!SendCtrlMsg(RtmpMessage::Type::InvokeAMF0, arrayEncode))
        return false;

    obj->value["code"] = std::make_shared<AmfString>("NetStream.Play.Start");
    obj->value["description"] = std::make_shared<AmfString>("Start playing.");
    AmfValue::to_string(array);
    arrayEncode.clear();
    AmfValue::EncodeArray(arrayEncode, array);
    if (!SendCtrlMsg(RtmpMessage::Type::InvokeAMF0, arrayEncode))
        return false;

    array.clear();
    array.push_back(std::make_shared<AmfString>("|RtmpSampleAccess"));
    array.push_back(std::make_shared<AmfBoolean>(true)); // 允许读
    array.push_back(std::make_shared<AmfBoolean>(true)); // 允许写
    AmfValue::to_string(array);
    arrayEncode.clear();
    AmfValue::EncodeArray(arrayEncode, array);
    if (!SendCtrlMsg(RtmpMessage::Type::InvokeAMF0, arrayEncode))
        return false;

    // 发送元信息
    array.clear();
    array.push_back(std::make_shared<AmfString>("onMetaData"));
    array.push_back(session->GetStreamMetadata());
    data.clear();
    AmfValue::EncodeArray(data, array);
    SendCtrlMsg(RtmpMessage::Type::DataAMF0, data);
    // 发送完元信息后再添加到session
    session->AddSink(std::dynamic_pointer_cast<RtmpSink>(shared_from_this()));
    server->NotifyEvent("Play.start", stream_path_);
    return true;
}
bool RtmpConnection::HandleDeleteStream(uint32_t transaction_id, uint32_t deleted_stream_id)
{
    auto server = rtmp_server_.lock();
    if (server == nullptr)
        return false;
    if (stream_path_ != "")
    {
        auto session = rtmp_session_.lock();
        if (session)
        {
            auto conn = std::dynamic_pointer_cast<RtmpSink>(shared_from_this());
            GetEventLoop()->AddTimer([session, conn]
                                     {
                    session->RemoveSink(conn);
                    return false; }, 1); // 从 session 中移除这个客户端

            if (is_publishing_)
            {
                server->NotifyEvent("publish.stop", stream_path_);
            }
            else if (is_audio_playing_ || is_video_playing_)
            {
                server->NotifyEvent("play.stop", stream_path_);
            }
        }

        is_audio_playing_ = false;
        is_video_playing_ = false;
        is_publishing_ = false;
        has_key_frame_ = false;
    }
    rtmp_message_manager_->ClearDecode();
    rtmp_message_manager_->ClearEncode();
    return true;
}
bool RtmpConnection::HandleSendResult(uint32_t transaction_id)
{
    AmfValue::Array array;
    array.push_back(std::make_shared<AmfString>("_result"));
    array.push_back(std::make_shared<AmfNumber>(transaction_id));
    array.push_back(std::make_shared<AmfNull>());
    array.push_back(std::make_shared<AmfNull>());
    AmfValue::to_string(array);
    std::vector<uint8_t> arrayEncode;
    AmfValue::EncodeArray(arrayEncode, array);
    auto ret = SendCtrlMsg(RtmpMessage::Type::InvokeAMF0, arrayEncode);
    // fmt::print("HandleSendResult\n");
    return ret;
}
bool RtmpConnection::HandleGetStreamLength(uint32_t transaction_id)
{
    AmfValue::Array array;
    array.push_back(std::make_shared<AmfString>("_result"));
    array.push_back(std::make_shared<AmfNumber>(transaction_id));
    array.push_back(std::make_shared<AmfNull>());
    array.push_back(std::make_shared<AmfNumber>(0));

    std::vector<uint8_t> arrayEncode;
    AmfValue::EncodeArray(arrayEncode, array);
    auto ret = SendCtrlMsg(RtmpMessage::Type::InvokeAMF0, arrayEncode);
    AmfValue::to_string(array);
    // fmt::print("HandleGetStreamLength\n");
    return ret;
}

bool RtmpConnection::SendCtrlMsg(RtmpMessage::Type tpid, const std::vector<uint8_t> &data)
{
    RtmpMessage::SPtr msg = std::make_shared<RtmpMessage>();
    msg->header = {
        .timestamp = 0,
        .length = (uint32_t)data.size(),
        .typeId = tpid,
        .streamId = stream_id_};
    msg->body = DataPayload::Create(data);
    return SendMsg(msg);
}

void RtmpConnection::SetPeerBandWidth()
{
    std::vector<uint8_t> data;
    NumberCodec::EncodeU32(data, peer_width_);
    NumberCodec::EncodeU8(data, 2);
    SendCtrlMsg(RtmpMessage::Type::PeerBandwidth, data);
}
void RtmpConnection::SendAcknowledgement()
{
    std::vector<uint8_t> data;
    NumberCodec::EncodeU32(data, acknowledgement_size_);
    SendCtrlMsg(RtmpMessage::Type::WindowAckSize, data);
}
void RtmpConnection::SetOutChunkSize(uint32_t size)
{
    rtmp_message_manager_->SetOutChunkSize(size);
    std::vector<uint8_t> data;
    NumberCodec::EncodeU32(data, size);
    SendCtrlMsg(RtmpMessage::Type::SetChunkSize, data);
}

bool RtmpConnection::IsKeyFrame(const DataPayload::SPtr &data)
{
    if (data->size() < 1)
        return false;
    auto byte = data->at(0);
    uint8_t frameType = (byte >> 4) & 0x0F;
    uint8_t codecId = byte & 0x0F;

    // static uint64_t cnt = 0;
    // ++cnt;
    // if (frameType == 1)
    // {
    //     auto type = frameType;
    //     fmt::print("{}关键帧出现了\n", cnt);
    // }

    if (frameType != 1)
        return false;
    switch (static_cast<RtmpVideoCodecId>(codecId))
    {
    case RtmpVideoCodecId::H264:
    case RtmpVideoCodecId::HEVC:
    case RtmpVideoCodecId::AV1:
    case RtmpVideoCodecId::VP6:
    case RtmpVideoCodecId::H263:
        return true;
    }
    return false; // 格式不支持
}

bool RtmpConnection::SendMsg(const RtmpMessage::SPtr &rtmpMsg)
{
    if (IsShutdown())
        return false;
    auto chunks = rtmp_message_manager_->EncodeMessage(rtmpMsg); // msg分块
    std::shared_ptr<std::vector<char>> chdata;
    // 发送每个块
    for (auto &ck : chunks)
    {
        Send(ck.Encode());
    }
    return true;
}

bool RtmpConnection::SendMetaData(const AmfObject::SPtr &metaData)
{ // 发送消息，通知后面的数据为元数据
    if (IsShutdown())
        return false;
    if (stream_metadata_->value.size() == 0)
        return false;

    AmfValue::Array array;
    array.push_back(std::make_shared<AmfString>("onMetaData"));
    array.push_back(metaData);
    std::vector<uint8_t> payload;
    AmfValue::EncodeArray(payload, array);
    return SendCtrlMsg(RtmpMessage::Type::DataAMF0, payload);
}
bool RtmpConnection::SendMetaData(const AmfEcmaArray::SPtr &metaData)
{ // 发送消息，通知后面的数据为元数据
    if (IsShutdown())
        return false;
    if (stream_metadata_->value.size() == 0)
        return false;

    AmfValue::Array array;
    array.push_back(std::make_shared<AmfString>("onMetaData"));
    array.push_back(metaData);
    std::vector<uint8_t> payload;
    AmfValue::EncodeArray(payload, array);
    return SendCtrlMsg(RtmpMessage::Type::DataAMF0, payload);
}

bool RtmpConnection::SendVideoData(RtmpVideoFrameType type, uint64_t timestamp, DataPayload::SPtr &payload)
{
    if (IsShutdown() ||
        payload == nullptr ||
        payload->size() == 0)
        return false;
    is_video_playing_ = true;
    if (type == RtmpVideoFrameType::SequenceHeader)
    {
        avc_sequence_header_ = payload;
    }
    else if (!has_key_frame_ && avc_sequence_header_->size())
    {
        if (IsKeyFrame(payload))
        {
            has_key_frame_ = true;
        }
        else
        {
            return true;
        }
    }
    // if (type != RtmpVideoFrameType::SequenceHeader)
    // {
    //     static int kf = 0;
    //     static int tf = 0;
    //     if (IsKeyFrame(payload))
    //     {
    //         ++kf;
    //     }
    //     ++tf;
    //     fmt::print("关键帧数量 : {}, 总帧数 : {} \n", kf, tf);
    // }

    // 收到关键帧后开始发送 msg
    RtmpMessage::SPtr msg = std::make_shared<RtmpMessage>();
    msg->header.timestamp = timestamp;
    msg->header.streamId = stream_id_;
    msg->header.length = payload->size();
    msg->body = payload;

    msg->header.typeId = RtmpMessage::Type::Video;
    SendMsg(msg);
    fmt::print("=========拉流端视频时间戳: {}========\n", msg->header.timestamp);
    if (type == RtmpVideoFrameType::SequenceHeader)
    {
        // fmt::print("发送的是视频序列头\n");
    }
    else
    {
        // fmt::print("发送的是视频帧\n");
    }
    return true;
}
bool RtmpConnection::SendAudioData(RtmpAudioFrameType type, uint64_t timestamp, DataPayload::SPtr &payload)
{
    if (IsShutdown() ||
        payload == nullptr ||
        payload->size() == 0)
        return false;
    is_audio_playing_ = true;
    if (type == RtmpAudioFrameType::SequenceHeader)
    {
        aac_sequence_header_ = payload;
        // fmt::print("保存音频序列头\n");
    }

    // 音频帧不用等关键帧，直接发送
    RtmpMessage::SPtr msg = std::make_shared<RtmpMessage>();
    msg->header.timestamp = timestamp;
    msg->header.streamId = stream_id_;
    msg->header.length = payload->size();
    msg->body = payload;

    msg->header.typeId = RtmpMessage::Type::Audio;
    SendMsg(msg);
    fmt::print("=========拉流端音频时间戳: {}========\n", msg->header.timestamp);
    if (type == RtmpAudioFrameType::SequenceHeader)
    {
        // fmt::print("发送的是音频序列头\n");
    }
    else
    {
        // fmt::print("发送的是音频帧\n");
    }
    return true;
}

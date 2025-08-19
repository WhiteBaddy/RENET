#include "RtmpServer.h"
#include <regex>
#include <RNet/TcpConnection.h>
#include "RtmpConnection.h"

Rtmp::~Rtmp() = default;
void Rtmp::SetChunkSize(uint32_t size)
{
    if (size > 0 && size <= 60 * 1000)
    {
        max_chunk_size_ = size;
    }
}
void Rtmp::SetPeerBandwidth(uint32_t size)
{
    peer_bandwidth_ = size;
}
uint32_t Rtmp::GetChunkSize() const { return max_chunk_size_; }
uint32_t Rtmp::GetAcknowledgementSize() const { return acknowledgement_; }
uint32_t Rtmp::GetPeerBandwidth() const { return peer_bandwidth_; }
std::string Rtmp::GetApp() const { return app_; }
std::string Rtmp::GetStreamPath() const { return stream_path_; }
std::string Rtmp::GetStreamName() const { return stream_name_; }

int Rtmp::ParseRtmpUrl(const std::string &url)
{
    // 检查是否是 RTMP URL, 如果不是以这个开头，就直接返回错误
    if (url.find("rtmp://") != 0)
    {
        return -1;
    }

    // 正则表达式匹配 rtmp://<ip>[:<port>]/<app>/<streamName>
    std::regex rtmp_rgx(R"(rtmp://([^:/]+)(?::(\d+))?(/.*))");
    std::smatch matches;
    if (!std::regex_match(url, matches, rtmp_rgx) || matches.size() < 4)
    {
        return -1;
    }
    // 提取ip
    ip_ = matches[1].str();
    // 提取端口, 默认 1935
    port_ = matches[2].matched ? std::stoi(matches[2].str()) : 1935;
    // 提取流路径
    stream_path_ = matches[3].str();

    // 解析流路径
    std::regex path_rgx(R"(/([^/]+)/(.+))");
    std::smatch path_matches;
    if (!std::regex_match(stream_path_, path_matches, path_rgx) || path_matches.size() < 3)
    {
        return -1;
    }
    app_ = path_matches[1].str();
    stream_name_ = path_matches[2].str();
    return 0;
}

RtmpServer::SPtr RtmpServer::Create()
{
    auto ins = std::make_shared<RtmpServer>();
    ins->Init();
    return ins;
}
RtmpServer::RtmpServer() : TcpServer() {}
RtmpServer::~RtmpServer() = default;
void RtmpServer::SetEventCallback(const EventCallback &cb)
{
    std::lock_guard<std::mutex> lck(mtx_sessons_);
    event_cbs_.push_back(cb);
}
void RtmpServer::Init()
{
    TcpServer::Init();
    auto loop = m_loopPool->GetEventLoop();
    // 每3秒清理一次空房间
    loop->AddTimer([this]
                   {
        std::lock_guard<std::mutex> lck(mtx_sessons_);
        for (auto it = sessions_.begin(); it != sessions_.end();) {
            if (it->second->GetClientNumber() == 0) {
                it = sessions_.erase(it);
            }
            else {
                ++it;
            }
        }
        return true; }, 3 * 1000);
}

void RtmpServer::AddSession(std::string stream_path)
{ // 每个session 由流路径来区分
    std::lock_guard<std::mutex> lck(mtx_sessons_);
    if (sessions_.find(stream_path) == sessions_.end())
    {
        sessions_[stream_path] = std::make_shared<Session>();
    }
}
void RtmpServer::RemoveSession(std::string stream_path)
{
    std::lock_guard<std::mutex> lck(mtx_sessons_);
    sessions_.erase(stream_path);
}
Session::SPtr RtmpServer::GetSession(std::string stream_path)
{
    std::lock_guard<std::mutex> lck(mtx_sessons_);
    if (sessions_.find(stream_path) == sessions_.end())
    {
        return nullptr;
    }
    return sessions_[stream_path];
}
bool RtmpServer::HasPublisher(std::string stream_path)
{
    auto session = GetSession(stream_path);
    if (session == nullptr)
    {
        return false;
    }
    return session->GetPublisher() != nullptr;
}
bool RtmpServer::HasSession(std::string stream_path)
{
    std::lock_guard<std::mutex> lck(mtx_sessons_);
    return (sessions_.find(stream_path) == sessions_.end());
}
void RtmpServer::NotifyEvent(std::string type, std::string stream_path)
{
    std::lock_guard<std::mutex> lck(mtx_sessons_);
    for (auto cb : event_cbs_)
    {
        if (cb != nullptr)
        {
            cb(type, stream_path);
        }
    }
}
TcpConnection::SPtr RtmpServer::OnConnect(int sockfd)
{
    // auto ins = std::make_shared<RtmpConnection>(shared_from_this(), m_loopPool->GetEventLoop(), sockfd);
    auto ins = RtmpConnection::Create(shared_from_this(), m_loopPool->GetEventLoop(), sockfd);
    return std::dynamic_pointer_cast<TcpConnection>(ins);
}
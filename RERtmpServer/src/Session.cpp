#include "Session.h"
#include "RtmpConnection.h"

Session::Session() : has_publisher_(false) {}
Session::~Session() = default;
// 设置音视频的序列头
void Session::SetAvcSequenceHeader(const DataPayload::SPtr &header)
{
    std::lock_guard<std::mutex> lck(mtx_);
    avc_sequence_header_ = header;
}
void Session::SetAacSequenceHeader(const DataPayload::SPtr &header)
{
    std::lock_guard<std::mutex> lck(mtx_);
    aac_sequence_header_ = header;
}
// 添加客户端
void Session::AddSink(const RtmpSink::SPtr &sink)
{
    std::lock_guard<std::mutex> lck(mtx_);
    rtmpsinks_[sink->GetId()] = sink;
    // 如果是主播，说明直播间刚刚创建，没有设置元数据
    if (sink->IsPublisher())
    {
        SetPublisher(sink);
    }
}
// 移除客户端
void Session::RemoveSink(const RtmpSink::SPtr &sink)
{
    std::lock_guard<std::mutex> lck(mtx_);
    if (sink->IsPublisher())
    {
        RemovePublisher(sink);
    }
    else
    {
        rtmpsinks_.erase(sink->GetId());
    }
}
// 获取当前直播间人数，包括主播和观众
int Session::GetClientNumber()
{
    std::lock_guard<std::mutex> lck(mtx_);
    int clientNum = 0;
    for (auto &[_, sink] : rtmpsinks_)
    {
        if (auto conn = sink.lock(); conn != nullptr)
        {
            ++clientNum;
        }
    }
    return clientNum;
}
RtmpConnection::SPtr Session::GetPublisher()
{
    std::lock_guard<std::mutex> lck(mtx_);
    if (auto publisher = publisher_.lock(); publisher != nullptr)
    {
        return std::dynamic_pointer_cast<RtmpConnection>(publisher);
    }
    return nullptr;
}
AmfEcmaArray::SPtr Session::GetStreamMetadata()
{
    return stream_metadata_;
}

// 发送元数据
void Session::SendMetaData(const AmfObject::SPtr &metadata)
{
    std::lock_guard<std::mutex> lck(mtx_);
    for (auto it = rtmpsinks_.begin(); it != rtmpsinks_.end();)
    {
        if (auto conn = it->second.lock(); conn != nullptr)
        {
            // 只发送给拉流者
            if (conn->IsPlayer())
            {
                conn->SendMetaData(metadata);
            }
            ++it;
        }
        else
        {
            it = rtmpsinks_.erase(it);
        }
    }
}
void Session::SendMetaData(const AmfEcmaArray::SPtr &metadata)
{
    stream_metadata_ = metadata;
    std::lock_guard<std::mutex> lck(mtx_);
    for (auto it = rtmpsinks_.begin(); it != rtmpsinks_.end();)
    {
        if (auto conn = it->second.lock(); conn != nullptr)
        {
            // 只发送给拉流者
            if (conn->IsPlayer())
            {
                conn->SendMetaData(metadata);
            }
            ++it;
        }
        else
        {
            it = rtmpsinks_.erase(it);
        }
    }
}
void Session::SendVideoData(RtmpVideoFrameType type, uint64_t timestamp, DataPayload::SPtr &data)
{
    std::lock_guard<std::mutex> lck(mtx_);
    if (rtmpsinks_.size() > 1)
    {
        auto size = rtmpsinks_.size();
        // fmt::print("当前有{}个客户端，应该转发数据了\n", size);
    }
    for (auto it = rtmpsinks_.begin(); it != rtmpsinks_.end();)
    {
        if (auto conn = it->second.lock(); conn != nullptr)
        {
            if (conn->IsPlayer())
            {
                if (!conn->IsPlaying())
                {
                    conn->SendVideoData(RtmpVideoFrameType::SequenceHeader, timestamp, avc_sequence_header_);
                }
                conn->SendVideoData(type, timestamp, data);
            }
            ++it;
        }
        else
        {
            it = rtmpsinks_.erase(it);
        }
    }
}
void Session::SendAudioData(RtmpAudioFrameType type, uint64_t timestamp, DataPayload::SPtr &data)
{
    std::lock_guard<std::mutex> lck(mtx_);
    for (auto it = rtmpsinks_.begin(); it != rtmpsinks_.end();)
    {
        if (auto conn = it->second.lock(); conn != nullptr)
        {
            if (conn->IsPlayer())
            {
                if (!conn->IsPlaying())
                {
                    conn->SendAudioData(RtmpAudioFrameType::SequenceHeader, timestamp, avc_sequence_header_);
                }
                conn->SendAudioData(type, timestamp, data);
            }
            ++it;
        }
        else
        {
            it = rtmpsinks_.erase(it);
        }
    }
}

void Session::SetPublisher(const RtmpSink::SPtr &sink)
{
    avc_sequence_header_.reset();
    aac_sequence_header_.reset();
    has_publisher_ = true;
    publisher_ = sink;
}
void Session::RemovePublisher(const RtmpSink::SPtr &sink)
{
    // 推流者退出，会话状态清理
    avc_sequence_header_.reset();
    aac_sequence_header_.reset();
    has_publisher_ = false;
    rtmpsinks_.erase(sink->GetId());
}

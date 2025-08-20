#pragma once
#include "Message.h"
#include "Chunk.h"
#include <functional>
#include <queue>
#include <unordered_map>
#include <fmt/core.h>

class ChunkDecodeStream
{
public:
    using SPtr = std::shared_ptr<ChunkDecodeStream>;
    friend class RtmpMessageManager;

public:
    ChunkDecodeStream() : is_waiting_for_first_chunk_(true), body_buffer_(DataPayload::Create()) {}
    // 判断是否有完整的消息
    inline bool HasCompleteMessage() const { return exected_length_ <= 0; }
    // 设置消息提交的回调函数
    void SetSubmitMsgFunc(const std::function<void(RtmpMessage &)> &cb) { submit_msg_callback_ = cb; }
    // 接收到一个 RtmpChunk, 更新状态
    void OnChunkReceived(const RtmpChunk &chunk)
    {
        RtmpMessage::Header chunkMsgHeader = chunk.header.rtmpMsgHeader;
        if (is_waiting_for_first_chunk_)
        {
            exected_length_ = chunk.header.rtmpMsgHeader.length;
            is_waiting_for_first_chunk_ = false;
        }
        chunkMsgHeader.CalibrateTimeStamp(last_header_, static_cast<uint8_t>(chunk.header.basicHeader.fmt)); // 时间戳校准
        last_header_ = chunkMsgHeader;                                                                       // 更新
        body_buffer_->push_back(chunk.payload);
        exected_length_ -= chunk.payload->size();
        // 如果有已经完整的消息，就进行提取
        if (HasCompleteMessage())
            ExtractMessage();
    }
    // 提取完整消息，同时清空内部状态
    void ExtractMessage()
    {
        RtmpMessage msg;
        msg.header = last_header_;
        msg.body = DataPayload::Create(body_buffer_, 0, last_header_.length);
        if (submit_msg_callback_)
        { // 提交
            submit_msg_callback_(msg);
        }
        // 状态清理
        is_waiting_for_first_chunk_ = true;
        body_buffer_->pop_front(last_header_.length);
    }

private:
    RtmpMessage::Header last_header_;                        // 最后解析出的消息头，用于 fmt=1/2/3 的 header 继承
    uint32_t exected_length_;                                // 预期长度
    bool is_waiting_for_first_chunk_;                        // 当前是否正在等待第一个块
    DataPayload::SPtr body_buffer_;                          // rtmpMsgBody拼接缓冲区
    std::function<void(RtmpMessage &)> submit_msg_callback_; // 把解析出的msg提交给 manager
};

class ChunkEncodeStream
{
public:
    using SPtr = std::shared_ptr<ChunkEncodeStream>;
    friend class RtmpMessageManager;

public:
    ChunkEncodeStream(RtmpChunk::StreamId csid) : is_waiting_for_first_msg_(true), csid_(csid) {}

    std::vector<RtmpChunk> EncodeMessage(const RtmpMessage::SPtr &msg, uint32_t out_chunk_size)
    {
        std::vector<RtmpChunk> chunks;
        chunks.reserve((msg->header.length + out_chunk_size - 1) / out_chunk_size); // 预先分配好空间

        RtmpChunk::FmtType firstFmt = RtmpChunk::FmtType::Full;
        RtmpMessage::Header chunkMsgHeader = msg->header;
        // 确认第一个块的fmt
        if (is_waiting_for_first_msg_)
        {
            is_waiting_for_first_msg_ = false;
            firstFmt = RtmpChunk::FmtType::Full;
            last_header_ = msg->header;
        }
        else if (msg->header.length != last_header_.length || msg->header.typeId != last_header_.typeId)
        {
            firstFmt = RtmpChunk::FmtType::Medium;
        }
        else
        {
            firstFmt = RtmpChunk::FmtType::Small;
        }
        // 第一个块
        chunkMsgHeader.DecalibrateTimeStamp(last_header_, static_cast<uint8_t>(firstFmt));
        RtmpChunk chunk({{RtmpChunk::Header::BasicHeader{firstFmt, csid_}, chunkMsgHeader},
                         DataPayload::Create(msg->body, 0, std::min(out_chunk_size, msg->header.length))});
        chunks.push_back(chunk);
        // 剩余块
        chunk.header.basicHeader.fmt = RtmpChunk::FmtType::Minimal; // 剩余的chunk fmt值都是3
        chunk.header.rtmpMsgHeader.DecalibrateTimeStamp(last_header_, static_cast<uint8_t>(RtmpChunk::FmtType::Minimal));
        for (int idx = out_chunk_size, remainder = msg->header.length - out_chunk_size;
             remainder > 0;
             idx += out_chunk_size, remainder -= out_chunk_size)
        {
            chunk.payload = DataPayload::Create(msg->body, idx, std::min(out_chunk_size, (uint32_t)remainder));
            chunks.push_back(chunk);
        }
        return chunks;
    }

private:
    RtmpChunk::StreamId csid_;        // 这个流的id
    RtmpMessage::Header last_header_; // 上一个消息头，用于判断当前编码时的 fmt 选择
    bool is_waiting_for_first_msg_;   // 当前是否在等待第一个消息
};

class RtmpMessageManager
{
public:
    using SPtr = std::shared_ptr<RtmpMessageManager>;

public:
    static SPtr Create()
    {
        auto ins = std::make_shared<RtmpMessageManager>();
        ins->Init();
        return ins;
    }

public: // 解码功能
    void SetInChunkSize(uint32_t size) { in_chunk_size_ = size; }
    void SetOutChunkSize(uint32_t size) { out_chunk_size_ = size; }
    // 尝试从接收缓冲区解析出一个块
    int ParseOnChunk(const uint8_t *in, int len)
    {
        int offset = 0;
        int ret = 0;
        RtmpChunk chunk;
        RtmpChunk::Header::BasicHeader basicHeader;
        ret = basicHeader.Decode(in, len); // 解析基础头部
        if (ret <= 0)
            return false; // 解析失败或数据不完整
        offset += ret;
        if (chunk_decode_streams_.find(basicHeader.csid) == chunk_decode_streams_.end())
        {
            chunk_decode_streams_[basicHeader.csid] = std::make_shared<ChunkDecodeStream>();
            chunk_decode_streams_[basicHeader.csid]->SetSubmitMsgFunc([this](RtmpMessage &msg)
                                                                      { SubmitMessage(msg); });
        }
        auto &lastStream = chunk_decode_streams_[basicHeader.csid]; // 获取对应的流
        auto &lastHeader = lastStream->last_header_;                // 上一个块的头部

        /*
        // 当前块的预期长度
        // fmt=0/1时, 在函数内部才能知道包的具体长度，传入in_chunk_size_, 在函数内部再次校准
        // fmt=2时，需要先通过上一个msg头获取到包长度，再与in_chunk_size_进行比较后取较小的值
        // fmt=3时，取msg预期长度与in_chunk_size_的较小值
        auto chunkPayloadSize = in_chunk_size_;
        if (basicHeader.fmt == RtmpChunk::FmtType::Small)
        {
            lastStream->exected_length_ = lastHeader.length; // fmt=2时，包长度与上一个包相同
            chunkPayloadSize = std::min(lastStream->exected_length_, in_chunk_size_);
        }
        else if (basicHeader.fmt == RtmpChunk::FmtType::Minimal)
        {
            chunkPayloadSize = std::min(lastStream->exected_length_, in_chunk_size_);
        }
        */

        // 策略修改：
        // 考虑到有些实现中如果当前msg与上一msg头部相同，也会使用fmt=3的方式发送
        // 所以这里传递的值的计算较为复杂
        // 统一进入内部作二次矫正

        // 只有fmt=3且上一个msg头部的预期长度大于0时，才表示上一chunk的延续
        auto chunkPayloadSize =
            (basicHeader.fmt == RtmpChunk::FmtType::Minimal && lastStream->exected_length_ > 0)
                ? std::min(lastStream->exected_length_, in_chunk_size_)
                : in_chunk_size_;

        ret = chunk.Decode(basicHeader, lastHeader, chunkPayloadSize, in + offset, len - offset);
        if (ret <= 0)
            return false; // 解析失败或数据不完整
        offset += ret;
        lastStream->OnChunkReceived(chunk); // 解析出的块提交到对应的流中
        return offset;                      // 成功解析一个块
    }
    RtmpMessage GetMsg()
    {
        std::lock_guard<std::mutex> lck(mtx_decode_msgs_);
        auto msg = std::move(decode_messages_.front());
        decode_messages_.pop();
        static int cnt = 0;
        fmt::print("GetMsg, cnt: {}, size: {}\n", ++cnt, msg.body->size());
        if (cnt == 86)
        {
            auto str = msg.body->to_string();
            fmt::print("GetMsg, cnt: {}, body: {}\n", cnt, str);
        }
        return msg;
    }
    bool HasMsg()
    {
        std::lock_guard<std::mutex> lck(mtx_decode_msgs_);
        return !decode_messages_.empty();
    }
    void ClearDecode()
    {
        std::lock_guard<std::mutex> lck(mtx_decode_msgs_);
        chunk_decode_streams_.clear();
    }

public: // 编码功能
    static RtmpChunk::StreamId GetCsid(RtmpMessage::Type msg_type_id)
    {
        static constexpr std::array<std::pair<RtmpMessage::Type, RtmpChunk::StreamId>, 10> MessageType2ChunkStreamId = {{
            {RtmpMessage::Type::Audio, RtmpChunk::StreamId::Audio},
            {RtmpMessage::Type::Video, RtmpChunk::StreamId::Video},
            {RtmpMessage::Type::SetChunkSize, RtmpChunk::StreamId::ProtocolControl},
            {RtmpMessage::Type::Abort, RtmpChunk::StreamId::ProtocolControl},
            {RtmpMessage::Type::Acknowledgement, RtmpChunk::StreamId::ProtocolControl},
            {RtmpMessage::Type::WindowAckSize, RtmpChunk::StreamId::ProtocolControl},
            {RtmpMessage::Type::PeerBandwidth, RtmpChunk::StreamId::ProtocolControl},
            {RtmpMessage::Type::UserControl, RtmpChunk::StreamId::ProtocolControl},
            {RtmpMessage::Type::InvokeAMF0, RtmpChunk::StreamId::Command},
            {RtmpMessage::Type::DataAMF0, RtmpChunk::StreamId::Data} //
        }};

        auto it = std::find_if(
            MessageType2ChunkStreamId.begin(),
            MessageType2ChunkStreamId.end(),
            [msg_type_id](const auto &p)
            { return p.first == msg_type_id; });
        return (it != MessageType2ChunkStreamId.end())
                   ? it->second
                   : RtmpChunk::StreamId::ProtocolControl;
    }
    std::vector<RtmpChunk> EncodeMessage(const RtmpMessage::SPtr &msg)
    {
        auto csid = GetCsid(msg->header.typeId);
        if (chunk_encode_streams_.find(csid) == chunk_encode_streams_.end())
        {
            chunk_encode_streams_[csid] = std::make_shared<ChunkEncodeStream>(csid);
        }
        return chunk_encode_streams_[csid]->EncodeMessage(msg, out_chunk_size_);
    }
    void ClearEncode()
    {
        chunk_encode_streams_.clear();
    }

private:
    void Init() {}
    void SubmitMessage(RtmpMessage &msg)
    {
        std::lock_guard<std::mutex> lck(mtx_decode_msgs_);
        decode_messages_.push(std::move(msg));
    }

private:
    std::unordered_map<RtmpChunk::StreamId, ChunkDecodeStream::SPtr> chunk_decode_streams_; // 所有csid对应的解析流
    uint32_t in_chunk_size_ = 128;
    uint32_t out_chunk_size_ = 128;
    std::mutex mtx_decode_msgs_;              // 保护解析出的包队列
    std::queue<RtmpMessage> decode_messages_; // 解析出的包

private:
    std::unordered_map<RtmpChunk::StreamId, ChunkEncodeStream::SPtr> chunk_encode_streams_;
};

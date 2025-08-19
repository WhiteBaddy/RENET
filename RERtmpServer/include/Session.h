#pragma once
#include <mutex>
#include <memory>
#include <unordered_map>
#include <vector>
#include "AMF.h"
#include "Sink.h"
#include <Tools/UintCodec.h>
#include "Rtmp.h"

class RtmpConnection;
using RtmpConnectionSPtr = std::shared_ptr<RtmpConnection>;

class Session
{
public:
    using SPtr = std::shared_ptr<Session>;
    using WPtr = std::weak_ptr<Session>;

public:
    Session();
    virtual ~Session();
    // 设置音视频的序列头
    void SetAvcSequenceHeader(const DataPayload::SPtr &header);
    void SetAacSequenceHeader(const DataPayload::SPtr &header);
    // 添加客户端
    void AddSink(const RtmpSink::SPtr &sink);
    // 移除客户端
    void RemoveSink(const RtmpSink::SPtr &sink);
    // 获取当前直播间人数，包括主播和观众
    int GetClientNumber();
    RtmpConnectionSPtr GetPublisher();
    AmfEcmaArray::SPtr GetStreamMetadata();

    // 发送元数据
    void SendMetaData(const AmfObject::SPtr &metadata);
    void SendMetaData(const AmfEcmaArray::SPtr &metadata);
    void SendVideoData(RtmpVideoFrameType type, uint64_t timestamp, DataPayload::SPtr &data);
    void SendAudioData(RtmpAudioFrameType type, uint64_t timestamp, DataPayload::SPtr &data);

private:
    void SetPublisher(const RtmpSink::SPtr &sink);
    void RemovePublisher(const RtmpSink::SPtr &sink);

private:
    std::mutex mtx_;
    bool has_publisher_;                 // 当前会话是否存在推流者
    RtmpSink::WPtr publisher_;           // 推流者
    AmfEcmaArray::SPtr stream_metadata_; // 推流端发送的元数据，保存在这里，当拉流者连接时发送给对方
    DataPayload::SPtr avc_sequence_header_;
    DataPayload::SPtr aac_sequence_header_;
    std::unordered_map<uint32_t, RtmpSink::WPtr> rtmpsinks_; // 客户端
};
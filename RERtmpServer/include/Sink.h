#pragma once

#include "AMF.h"
#include "Message.h"
#include "Rtmp.h"
#include <DataPayload/DataPayload.h>

class RtmpSink
{
public:
    using SPtr = std::shared_ptr<RtmpSink>;
    using WPtr = std::weak_ptr<RtmpSink>;

public:
    RtmpSink() = default;
    virtual ~RtmpSink() = default;

public:
    // 发送元数据
    virtual bool SendMetaData(const AmfObject::SPtr &metaData) = 0;
    virtual bool SendMetaData(const AmfEcmaArray::SPtr &metaData) = 0;
    // 发送媒体数据
    virtual bool SendAudioData(RtmpAudioFrameType type, uint64_t timestamp, DataPayload::SPtr &payload) = 0;
    virtual bool SendVideoData(RtmpVideoFrameType type, uint64_t timestamp, DataPayload::SPtr &payload) = 0;
    // 是否是观众
    virtual bool IsPlayer() const { return false; }
    // 是否是主播
    virtual bool IsPublisher() const { return false; }
    // 是否正在播放
    virtual bool IsAudioPlaying() const { return false; }
    virtual bool IsVideoPlaying() const { return false; }
    // 是否正在直播
    virtual bool IsPublishing() const { return false; }
    // 获取唯一标识Id
    virtual uint32_t GetId() const = 0;
};
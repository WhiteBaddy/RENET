#pragma once
#include <cstdint>
#include <DataPayload/DataPayload.h>

struct RtmpMessage
{
    using SPtr = std::shared_ptr<RtmpMessage>;

    // RTMP消息类型
    enum class Type : uint8_t
    {
        SetChunkSize = 0x01,    // Set chunk size for subsequent messages
        Abort = 0x02,           // Abort processing of a chunk stream
        Acknowledgement = 0x03, // Acknowledge received bytes
        UserControl = 0x04,     // User control events (e.g., stream begin/end)
        WindowAckSize = 0x05,   // 服务端带宽
        PeerBandwidth = 0x06,   // 客户端带宽
        // VirtualControl = 0x07,  // 虚拟控制
        Audio = 0x08,      // 音频数据
        Video = 0x09,      // 视频数据
        DataAMF0 = 0x12,   // AMF0 data (e.g., metadata)
        InvokeAMF0 = 0x14, // AMF0 command/method invocation
    };

    struct Header
    {
        uint32_t timestamp;
        uint32_t length;
        Type typeId;
        uint32_t streamId;                                             // 小端存储
        void CalibrateTimeStamp(const Header &lastHeader, uint8_t fmt) // 时间戳校准, 需要根据fmt类型来判断校准方式
        {
            if (fmt == 0)
                return;
            else if (fmt == 3)
                timestamp = lastHeader.timestamp;
            else
                timestamp += lastHeader.timestamp;
        }
        void DecalibrateTimeStamp(const Header &lastHeader, uint8_t fmt) // 反向校准, 生成相对时间戳
        {
            if (fmt == 0)
                return;
            else if (fmt == 3)
                timestamp = 0;
            else
                timestamp -= lastHeader.timestamp;
        }
    };

    Header header;
    DataPayload::SPtr body;
};
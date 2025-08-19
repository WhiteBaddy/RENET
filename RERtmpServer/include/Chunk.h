#pragma once
#include "Message.h"
#include <DataPayload/DataPayload.h>
#include <Tools/UintCodec.h>
#include <string>

struct RtmpChunk
{

    // RTMP分片类型
    enum class FmtType : uint8_t
    {
        Full = 0,   // Full chunk header (12 bytes)
        Medium = 1, // Medium header (8 bytes, no timestamp)
        Small = 2,  // Small header (4 bytes, no message length/stream ID)
        Minimal = 3 // Minimal header (1 byte, only chunk stream ID)
    };

    // RTMP分片流ID
    enum class StreamId : uint16_t
    {
        ProtocolControl = 2, // Protocol control messages
        Command = 3,         // Command messages (e.g., connect, publish)
        Audio = 4,           // Audio data stream
        Video = 5,           // Video data stream
        Data = 6,            // Metadata or other data stream
    };

    struct Header
    {
        struct BasicHeader
        {
            FmtType fmt;
            StreamId csid; // Chunk Stream ID

            inline int Size() const { return static_cast<uint16_t>(csid) < 64 ? 1 : (static_cast<uint16_t>(csid) < 320 ? 2 : 3); }

            // basic头 解码
            int Decode(const uint8_t *in, size_t len)
            {
                int offset = 0;
                //--------------baseHeader 解析----------------
                if (len < 11)
                {
                    return 0; // 最小长度为11字节
                }
                fmt = static_cast<FmtType>(in[0] >> 6);     // 前两位为fmt
                csid = static_cast<StreamId>(in[0] & 0x3F); // 后六位, 正常值范围 3-63
                offset = 1;
                if (static_cast<uint16_t>(csid) == 0)
                { // 值范围为 64-319
                    csid = static_cast<StreamId>(in[1] + 64);
                    offset = 2;
                }
                else if (static_cast<uint16_t>(csid) == 1)
                { // 值范围为 64-65599
                    csid = static_cast<StreamId>((in[1] << 8) + in[2] + 64);
                    offset = 3;
                }
                return offset; // 返回解析后的偏移量
            }
        };
        using MsgHeader = RtmpMessage::Header;

        BasicHeader basicHeader; // 基础头部
        MsgHeader rtmpMsgHeader; // Msg头

        inline int Size() const
        {
            return basicHeader.Size() +
                   std::vector<int>{11, 7, 3, 0}[static_cast<uint8_t>(basicHeader.fmt)] +
                   (rtmpMsgHeader.timestamp == 0xFFFFFF ? 4 : 0);
        }
        // chunk头 编码
        std::vector<uint8_t> Encode()
        {
            std::vector<uint8_t> data(Size());
            int offset = 0;                                       // 偏移量
            data[0] = static_cast<uint8_t>(basicHeader.fmt) << 6; // 前两位为fmt
            if (static_cast<uint16_t>(basicHeader.csid) < 64)
            {
                data[0] |= static_cast<uint16_t>(basicHeader.csid) & 0x3F; // 后六位
                offset = 1;                                                // csid小于64时, 只需要1字节
            }
            else if (static_cast<uint16_t>(basicHeader.csid) < 320)
            {
                data[1] = (static_cast<uint16_t>(basicHeader.csid) - 64) & 0xFF; // 值范围为 64-319
                offset = 2;                                                      // csid在64-319之间, 需要2字节
            }
            else
            {
                data[0] |= 0x01;                                                        // 设置csid为1
                data[1] = ((static_cast<uint16_t>(basicHeader.csid) - 64) >> 8) & 0xFF; // 值范围为 64-65599
                data[2] = (static_cast<uint16_t>(basicHeader.csid) - 64) & 0xFF;
                offset = 3; // csid大于319, 需要3字节
            }
            //-------------rtmpMsgHeader 序列化----------------

            if (basicHeader.fmt < FmtType::Minimal)
            {
                UintCodec::EncodeU24(data, offset, std::min(rtmpMsgHeader.timestamp, (uint32_t)0xFFFFFF));
                offset += 3;
            }
            if (basicHeader.fmt < FmtType::Small)
            {
                UintCodec::EncodeU24(data, offset, rtmpMsgHeader.length);
                offset += 3;
                data[offset++] = static_cast<uint8_t>(rtmpMsgHeader.typeId);
            }
            if (basicHeader.fmt < FmtType::Medium)
            {
                UintCodec::EncodeU32LE(data, offset, rtmpMsgHeader.streamId);
                offset += 4;
            }
            //-------------拓展时间戳----------------
            if (rtmpMsgHeader.timestamp >= 0xFFFFFF) // 扩展时间戳
            {
                UintCodec::EncodeU32(data, offset, rtmpMsgHeader.timestamp);
                offset += 4;
            }
            return data; // 返回序列化后的头
        }
        // chunk头 解码
        int Decode(const Header::BasicHeader &bHeader, const uint8_t *data, size_t len)
        {
            int offset = 0;
            basicHeader = bHeader; // 基础头部
            //-------------rtmpMsgHeader 解析----------------
            if (len < offset + std::vector<int>{11, 7, 3, 0}[static_cast<uint8_t>(basicHeader.fmt)])
            {
                return 0; // fmt对应的最小长度
            }
            for (;;)
            {
                if (basicHeader.fmt == FmtType::Minimal)
                    break; // fmt为3时, 没有任何信息
                rtmpMsgHeader.timestamp = UintCodec::DecodeU24(data, len, offset);
                if (basicHeader.fmt == FmtType::Small)
                    break; // fmt为2时,仅包含相对时间戳
                rtmpMsgHeader.length = UintCodec::DecodeU24(data, len, offset);
                rtmpMsgHeader.typeId = static_cast<RtmpMessage::Type>(UintCodec::DecodeU8(data, len, offset));
                if (basicHeader.fmt == FmtType::Medium)
                    break; // fmt为1时, 包含相对时间戳、msg长度、typeId
                // streamId小端存储
                rtmpMsgHeader.streamId = UintCodec::DecodeU32LE(data, len, offset);
                break; // fmt为0时, 包含绝对时间戳、msg长度、typeId、streamId
            }

            //-------------扩展时间戳 解析----------------
            if (basicHeader.fmt < FmtType::Minimal && rtmpMsgHeader.timestamp == 0xFFFFFF)
            {
                if (len < offset + 4)
                    return 0; // 数据长度不够
                rtmpMsgHeader.timestamp = UintCodec::DecodeU32(data, len, offset);
            }
            return offset;
        }
    };

    Header header;             // chunk头
    DataPayload::SPtr payload; // 数据载荷
    // chunk 编码
    DataPayload::SPtr Encode()
    {
        auto res = DataPayload::Create(header.Encode());
        res->push_back(payload);
        return res;
    }
    // chunk 解码
    // 提前解析出的基础头， 上一个完整的msg头， 数据长度，开始解析的地址，长度
    int Decode(const Header::BasicHeader &bHeader, const Header::MsgHeader &mHeader, uint32_t payloadSize, const uint8_t *in, size_t len)
    {
        int offset = 0;
        int ret = 0;
        // 未提及的参数应该保持默认值--吗
        if (bHeader.fmt != FmtType::Full)
        {
            header.rtmpMsgHeader = mHeader;
        }
        // 解析msg头部,可能只含有部分信息，未包含信息依赖传进来的上一个头部
        ret = header.Decode(bHeader, in, len);
        // 如果数据量不足 ret 应该不会小于0，这里的判断应该是多余的
        // if (ret <= 0)
        //     return 0;
        // 需要重新校准payloadSize
        if (bHeader.fmt == FmtType::Full || bHeader.fmt == FmtType::Medium)
        {
            payloadSize = std::min(payloadSize, header.rtmpMsgHeader.length);
        }
        // 再次判断是否数据不足
        if (len < offset + ret + payloadSize)
            return 0;
        offset += ret; // 更新偏移量
        // 获取数据载荷
        // payload = DataPayload::Create(std::string(in + offset, in + offset + payloadSize));
        payload = DataPayload::Create(std::vector<uint8_t>(in + offset, in + offset + payloadSize));
        offset += payloadSize; // 再次更新偏移量
        return offset;         // 返回有效数据长度
    }
};

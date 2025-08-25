#pragma once
#include <cstdint>
#include <array>
#include <string>
#include <memory>
#include <algorithm> // std::copy_n
#include <Tools/NumberCodec.h>

// cmd 指令
enum class Cmd : uint8_t
{
    Invalied = 0x00, // 无效指令
    Join = 0x05,
    JoinReply = 0x06,
    ObtainStream = 0x07,
    ObtainStreamReply = 0x08,
    CreateStream = 0x09,
    CreateStreamReply = 0x0A,
    PlayStream = 0x0B,
    PlayStreamReply = 0x0C,
    DeleteStream = 0x0D,
    DeleteStreamReply = 0x0E,

    MouseButton = 0x10,
    MouseMove = 0x11,
    WheelScroll = 0x12,
    KeyboardInput = 0x13,
};

// 应答指令
enum class ResultState : uint8_t
{
    Successful = 0x00,
    Error = 0x01,
    RequestTimeout = 0x02,
    AlreadyRedistered = 0x03,
    UserDisappear,
    AlreadyLogin,
    VerficateFailed,
};

// 客户端状态
enum class RoleState : uint8_t
{
    None,         // 房间未建立
    Idle,         // 房间已创建，但还没有进行推流
    Publishing,   // 正在推流
    Playing,      // 正在拉流
    Stopped,      // 已经停止推拉流
    Disconnected, // 断开连接
    Closed,       // 房间已关闭
};

// 鼠标键盘信息
enum class MouseType : uint8_t
{
    NoButton = 0x00,     // 无按键
    LeftButton = 0x01,   // 左键
    RightButton = 0x02,  // 右键
    MiddleButton = 0x04, // 中键
    XButton_1 = 0x10,    // X1键
    XButton_2 = 0x20,    // X2键
};

// 按键操作类型
enum KeyType : uint8_t
{
    None = 0x00,    // 无操作
    Press = 0x01,   // 按下
    Release = 0x02, // 释放
};

struct Packet
{
    using SPtr = std::shared_ptr<Packet>; // 智能指针类型别名
    struct Head
    {
        Cmd cmd;                                                        // 包命令
        uint16_t len;                                                   // 包长度
        Head(Cmd c = Cmd::Invalied, uint16_t l = 0) : cmd(c), len(l) {} // 默认构造函数
        static size_t size() { return sizeof(cmd) + sizeof(len); }      // 包头大小
        int Decode(const uint8_t *in, size_t len)
        {
            if (len < size())
                return 0; // 数据不足
            int offset = 0;
            cmd = static_cast<Cmd>(NumberCodec::DecodeU8(in, len, offset));  // 解码命令
            len = NumberCodec::DecodeU16(in + offset, len - offset, offset); // 解码长度
            return offset;                                                   // 解码成功
        }
        std::vector<uint8_t> Encode() const
        {
            std::vector<uint8_t> buf;
            NumberCodec::EncodeU8(buf, static_cast<uint8_t>(cmd)); // 编码命令
            NumberCodec::EncodeU16(buf, len);                      // 编码长度
            return buf;                                            // 返回编码后的数据
        }
    };
    Head head;                                                                                                        // 包头
    Packet(Cmd c = Cmd::Invalied, size_t body_size = 0) : head{c, static_cast<uint16_t>(Head::size() + body_size)} {} // 构造函数，初始化包头
    virtual ~Packet() = default;                                                                                      // 虚析构函数，确保派生类可以正确析构
    size_t size() const { return head.len; }                                                                          // 获取包大小

    virtual int Decode(const uint8_t *in, size_t len) = 0;
    virtual std::vector<uint8_t> Encode() const = 0; // 编码为字节数组

    static SPtr Create(Cmd cmd);
    static SPtr Create(const uint8_t *in, size_t len);
};

struct JoinPacket : public Packet
{
    std::array<uint8_t, 10> room_id; // 房间ID
    JoinPacket() : Packet(Cmd::Join, sizeof(room_id))
    {
        room_id.fill(0); // 初始化房间ID为0
    }
    // 设置房间ID
    void SetRoomId(const std::string &id)
    {
        std::copy_n(id.begin(), std::min(id.size(), room_id.size()), room_id.begin());
    }
    // 获取房间ID
    std::string GetRoomId() const
    {
        return std::string(room_id.begin(), room_id.end());
    }

    int Decode(const uint8_t *in, size_t len) override
    {
        if (len < size())
            return 0;         // 数据不足
        head.Decode(in, len); // 头部解析
        int offset = Packet::Head::size();
        std::copy(in + offset, in + offset + room_id.size(), room_id.begin());
        return offset + room_id.size(); // 返回解码消耗的长度
    }
    std::vector<uint8_t> Encode() const override
    {
        std::vector<uint8_t> buf = head.Encode();              // 编码包头
        buf.insert(buf.end(), room_id.begin(), room_id.end()); // 添加房间ID
        return buf;                                            // 返回编码后的数据
    }
};

// 创建房间应答
struct JoinReplyPacket : public Packet
{
    ResultState result_state;                                                                                  // 应答结果
    JoinReplyPacket() : Packet(Cmd::JoinReply, sizeof(result_state)), result_state(ResultState::Successful) {} // 默认构造函数
    void SetResultState(ResultState state) { result_state = state; }                                           // 设置应答结果状态

    int Decode(const uint8_t *in, size_t len) override
    {
        if (len < size())
            return 0;         // 数据不足
        head.Decode(in, len); // 头部解析
        int offset = Packet::Head::size();
        result_state = static_cast<ResultState>(NumberCodec::DecodeU8(in, len, offset)); // 解码结果状态
        return offset;                                                                   // 返回解码消耗的长度
    }
    std::vector<uint8_t> Encode() const override
    {
        std::vector<uint8_t> buf = head.Encode();                       // 编码包头
        NumberCodec::EncodeU8(buf, static_cast<uint8_t>(result_state)); // 编码结果状态
        return buf;                                                     // 返回编码后的数据
    }
};

// 获取流
struct ObtainStreamPacket : public Packet
{
    std::array<uint8_t, 10> stream_id; // 流ID
    ObtainStreamPacket() : Packet(Cmd::ObtainStream, sizeof(stream_id))
    {
        stream_id.fill(0);
    } // 默认构造函数
    void SetStreamId(std::string id)
    {
        std::copy_n(id.begin(), std::min(id.size(), stream_id.size()), stream_id.begin());
    } // 设置流ID
    std::string GetStreamId() const { return std::string(stream_id.begin(), stream_id.end()); } // 获取流ID

    int Decode(const uint8_t *in, size_t len) override
    {
        if (len < size())
            return 0;         // 数据不足
        head.Decode(in, len); // 头部解析
        int offset = Packet::Head::size();
        std::copy(in + offset, in + offset + stream_id.size(), stream_id.begin());
        return offset + stream_id.size(); // 返回解码消耗的长度
    }
    std::vector<uint8_t> Encode() const override
    {
        std::vector<uint8_t> buf = head.Encode();                  // 编码包头
        buf.insert(buf.end(), stream_id.begin(), stream_id.end()); // 添加流ID
        return buf;                                                // 返回编码后的数据
    }
};

// 获取流应答
struct ObtainStreamReplyPacket : public Packet
{
    ResultState result_state;                                                                                             // 应答结果
    ObtainStreamReplyPacket() : Packet(Cmd::ObtainStreamReply, sizeof(result_state)), result_state(ResultState::Error) {} // 默认构造函数
    void SetResultState(ResultState state) { result_state = state; }                                                      // 设置应答结果状态

    int Decode(const uint8_t *in, size_t len) override
    {
        if (len < size())
            return 0;         // 数据不足
        head.Decode(in, len); // 头部解析
        int offset = Packet::Head::size();
        result_state = static_cast<ResultState>(NumberCodec::DecodeU8(in, len, offset)); // 解码结果状态
        return offset;                                                                   // 返回解码消耗的长度
    }
    std::vector<uint8_t> Encode() const override
    {
        std::vector<uint8_t> buf = head.Encode();                       // 编码包头
        NumberCodec::EncodeU8(buf, static_cast<uint8_t>(result_state)); // 编码结果状态
        return buf;                                                     // 返回编码后的数据
    }
};

// 创建流
struct CreateStreamPacket : public Packet
{
    CreateStreamPacket() : Packet(Cmd::CreateStream) {} // 默认构造函数
    int Decode(const uint8_t *in, size_t len) override
    {
        if (len < size())
            return 0;                // 数据不足
        return head.Decode(in, len); // 返回头部解码消耗的长度
    }
    std::vector<uint8_t> Encode() const override
    {
        return head.Encode(); // 仅编码包头
    }
};

// 创建流应答 流地址和结果
struct CreateStreamReplyPacket : public Packet
{
    ResultState result_state;
    std::array<uint8_t, 70> stream_url; // 流地址
    CreateStreamReplyPacket() : Packet(Cmd::CreateStreamReply, sizeof(result_state) + sizeof(stream_url)), result_state(ResultState::Error)
    {
        stream_url.fill(0);
    } // 默认构造函数
    void SetResultState(ResultState state) { result_state = state; } // 设置应答结果状态
    void SetStreamUrl(const std::string &url)
    {
        std::copy_n(url.begin(), std::min(url.size(), stream_url.size()), stream_url.begin());
    } // 设置流地址
    std::string GetStreamUrl() const { return std::string(stream_url.begin(), stream_url.end()); } // 获取流地址

    int Decode(const uint8_t *in, size_t len) override
    {
        if (len < size())
            return 0;         // 数据不足
        head.Decode(in, len); // 头部解析
        int offset = Packet::Head::size();
        result_state = static_cast<ResultState>(NumberCodec::DecodeU8(in, len, offset)); // 解码结果状态
        std::copy(in + offset, in + offset + stream_url.size(), stream_url.begin());
        return offset + stream_url.size(); // 返回解码消耗的长度
    }
    std::vector<uint8_t> Encode() const override
    {
        std::vector<uint8_t> buf = head.Encode();                       // 编码包头
        NumberCodec::EncodeU8(buf, static_cast<uint8_t>(result_state)); // 编码结果状态
        buf.insert(buf.end(), stream_url.begin(), stream_url.end());    // 添加流地址
        return buf;                                                     // 返回编码后的数据
    }
};

// 播放流
struct PlayStreamPacket : public Packet
{
    ResultState result_state;
    std::array<uint8_t, 70> stream_url; // 流地址
    PlayStreamPacket() : Packet(Cmd::PlayStream, sizeof(result_state) + sizeof(stream_url)), result_state(ResultState::Error)
    {
        stream_url.fill(0);
    } // 默认构造函数
    void SetResultState(ResultState state) { result_state = state; } // 设置应答结果状态
    void SetStreamUrl(const std::string &url)
    {
        std::copy_n(url.begin(), std::min(url.size(), stream_url.size()), stream_url.begin());
    } // 设置流地址
    std::string GetStreamUrl() const { return std::string(stream_url.begin(), stream_url.end()); } // 获取流地址

    int Decode(const uint8_t *in, size_t len) override
    {
        if (len < size())
            return 0;         // 数据不足
        head.Decode(in, len); // 头部解析
        int offset = Packet::Head::size();
        result_state = static_cast<ResultState>(NumberCodec::DecodeU8(in, len, offset)); // 解码结果状态
        std::copy(in + offset, in + offset + stream_url.size(), stream_url.begin());
        return offset + stream_url.size(); // 返回解码消耗的长度
    }
    std::vector<uint8_t> Encode() const override
    {
        std::vector<uint8_t> buf = head.Encode();                       // 编码包头
        NumberCodec::EncodeU8(buf, static_cast<uint8_t>(result_state)); // 编码结果状态
        buf.insert(buf.end(), stream_url.begin(), stream_url.end());    // 添加流地址
        return buf;                                                     // 返回编码后的数据
    }
};

// 播放流应答
struct PlayStreamReplyPacket : public Packet
{
    ResultState result_state;                                                                                         // 应答结果
    PlayStreamReplyPacket() : Packet(Cmd::PlayStreamReply, sizeof(result_state)), result_state(ResultState::Error) {} // 默认构造函数
    void SetResultState(ResultState state) { result_state = state; }                                                  // 设置应答结果状态

    int Decode(const uint8_t *in, size_t len) override
    {
        if (len < size())
            return 0;         // 数据不足
        head.Decode(in, len); // 头部解析
        int offset = Packet::Head::size();
        result_state = static_cast<ResultState>(NumberCodec::DecodeU8(in, len, offset)); // 解码结果状态
        return offset;                                                                   // 返回解码消耗的长度
    }
    std::vector<uint8_t> Encode() const override
    {
        std::vector<uint8_t> buf = head.Encode();                       // 编码包头
        NumberCodec::EncodeU8(buf, static_cast<uint8_t>(result_state)); // 编码结果状态
        return buf;                                                     // 返回编码后的数据
    }
};

// 删除流
struct DeleteStreamPacket : public Packet
{
    // 如果拉流者数量为0，就停止推流，否则继续推流
    uint32_t player_count;                                                                     // 获取拉流者数量
    DeleteStreamPacket() : Packet(Cmd::DeleteStream, sizeof(player_count)), player_count(0) {} // 默认构造函数
    void SetPlayerCount(int32_t count) { player_count = count; }                               // 设置拉流者数量
    uint32_t GetPlayerCount() const { return player_count; }                                   // 获取拉流者数量

    int Decode(const uint8_t *in, size_t len) override
    {
        if (len < size())
            return 0;         // 数据不足
        head.Decode(in, len); // 头部解析
        int offset = Packet::Head::size();
        player_count = NumberCodec::DecodeU32(in + offset, len - offset, offset); // 解码拉流者数量
        return offset;                                                            // 返回解码消耗的长度
    }
    std::vector<uint8_t> Encode() const override
    {
        std::vector<uint8_t> buf = head.Encode();  // 编码包头
        NumberCodec::EncodeU32(buf, player_count); // 编码拉流者数量
        return buf;                                // 返回编码后的数据
    }
};

// 键盘信息
struct KeyboardInfo : public Packet
{
    uint16_t key;
    KeyType key_type;                                                                                               // 按键操作类型
    KeyboardInfo() : Packet(Cmd::KeyboardInput, sizeof(key) + sizeof(key_type)), key(0), key_type(KeyType::None) {} // 默认构造函数

    int Decode(const uint8_t *in, size_t len) override
    {
        if (len < size())
            return 0;         // 数据不足
        head.Decode(in, len); // 头部解析
        int offset = Packet::Head::size();
        key = NumberCodec::DecodeU16(in + offset, len - offset, offset);         // 解码按键
        key_type = static_cast<KeyType>(NumberCodec::DecodeU8(in, len, offset)); // 解码按键类型
        return offset;                                                           // 返回解码消耗的长度
    }
    std::vector<uint8_t> Encode() const override
    {
        std::vector<uint8_t> buf = head.Encode();                   // 编码包头
        NumberCodec::EncodeU16(buf, key);                           // 编码按键
        NumberCodec::EncodeU8(buf, static_cast<uint8_t>(key_type)); // 编码按键类型
        return buf;                                                 // 返回编码后的数据
    }
};

// 滚轮信息
struct WheelScrollInfo : public Packet
{
    int32_t delta;                                                           // 滚轮滚动的增量
    WheelScrollInfo() : Packet(Cmd::WheelScroll, sizeof(delta)), delta(0) {} // 默认构造函数

    int Decode(const uint8_t *in, size_t len) override
    {
        if (len < size())
            return 0;         // 数据不足
        head.Decode(in, len); // 头部解析
        int offset = Packet::Head::size();
        delta = NumberCodec::DecodeU32(in + offset, len - offset, offset); // 解码滚动增量
        return offset;                                                     // 返回解码消耗的长度
    }
    std::vector<uint8_t> Encode() const override
    {
        std::vector<uint8_t> buf = head.Encode(); // 编码包头
        NumberCodec::EncodeU32(buf, delta);       // 编码滚动增量
        return buf;                               // 返回编码后的数据
    }
};

// 鼠标移动信息
struct MouseMoveInfo : public Packet
{
    // int32_t x; // 鼠标X坐标
    // int32_t y; // 鼠标Y坐标
    int32_t dx;                                                                        // 鼠标X轴移动增量
    int32_t dy;                                                                        // 鼠标Y轴移动增量
    MouseMoveInfo() : Packet(Cmd::MouseMove, sizeof(dx) + sizeof(dy)), dx(0), dy(0) {} // 默认构造函数

    int Decode(const uint8_t *in, size_t len) override
    {
        if (len < size())
            return 0;         // 数据不足
        head.Decode(in, len); // 头部解析
        int offset = Packet::Head::size();
        dx = NumberCodec::DecodeU32(in + offset, len - offset, offset); // 解码X轴增量
        dy = NumberCodec::DecodeU32(in + offset, len - offset, offset); // 解码Y轴增量
        return offset;                                                  // 返回解码消耗的长度
    }
    std::vector<uint8_t> Encode() const override
    {
        std::vector<uint8_t> buf = head.Encode(); // 编码包头
        NumberCodec::EncodeU32(buf, dx);          // 编码X轴增量
        NumberCodec::EncodeU32(buf, dy);          // 编码Y轴增量
        return buf;                               // 返回编码后的数据
    }
};

// 鼠标按键信息
struct MouseButtonInfo : public Packet
{
    MouseType mouse_type;                                                                                // 鼠标按键类型
    MouseButtonInfo() : Packet(Cmd::MouseButton, sizeof(mouse_type)), mouse_type(MouseType::NoButton) {} // 默认构造函数
    void SetMouseType(MouseType type) { mouse_type = type; }                                             // 设置鼠标按键类型
    MouseType GetMouseType() const { return mouse_type; }                                                // 获取鼠标按键类型

    int Decode(const uint8_t *in, size_t len) override
    {
        if (len < size())
            return 0;         // 数据不足
        head.Decode(in, len); // 头部解析
        int offset = Packet::Head::size();
        mouse_type = static_cast<MouseType>(NumberCodec::DecodeU8(in, len, offset)); // 解码鼠标按键类型
        return offset;                                                               // 返回解码消耗的长度
    }
    std::vector<uint8_t> Encode() const override
    {
        std::vector<uint8_t> buf = head.Encode();                     // 编码包头
        NumberCodec::EncodeU8(buf, static_cast<uint8_t>(mouse_type)); // 编码鼠标按键类型
        return buf;                                                   // 返回编码后的数据
    }
};

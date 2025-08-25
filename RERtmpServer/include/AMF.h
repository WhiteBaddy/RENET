#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <cstring>
#include <fmt/core.h>
#include <Tools/NumberCodec.h>

class AmfValue
{
public:
    using SPtr = std::shared_ptr<AmfValue>;
    using Array = std::vector<SPtr>;

public:
    enum class AmfType : uint8_t
    {
        Number = 0x00,      // 数字
        Boolean = 0x01,     // 布尔值
        String = 0x02,      // 字符串
        Object = 0x03,      // map
        Null = 0x05,        // 空
        EcmaArray = 0x08,   // 宽松数组
        StrictArray = 0x0A, // 严格数组
        Date = 0x0B,        // 日期
        LongString = 0x0C,  // 长字符串
        XmlDoc = 0x0F,      // XML文档
        TypedObject = 0x10, // 类型对象
        Amf3 = 0x11,        // 切换到AMF3
        Invalid = 0xFF      // 无效
    };

public:
    AmfType type;
    AmfValue(AmfType amf0type = AmfType::Invalid) : type(amf0type) {}
    virtual ~AmfValue() = default;
    AmfType GetType() const { return type; }
    virtual std::string to_string(int nestLevel = 0) = 0;
    virtual int Decode(const uint8_t *in, size_t len) = 0;
    virtual std::vector<uint8_t> Encode() = 0;

    static Array DecodeArray(const uint8_t *in, size_t len, int &offset);
    static SPtr DecodeValue(const uint8_t *in, size_t len, int &offset);
    static void EncodeArray(std::vector<uint8_t> &buf, const Array &array);
    static void EncodeValue(std::vector<uint8_t> &buf, const SPtr &value);

    // protected:
    static std::string DecodeString(const uint8_t *in, size_t len, int &offset);
    static void EncodeString(std::vector<uint8_t> &buf, const std::string &str);

    static bool IsObjectEndMarker(const uint8_t *in, size_t len, int &offset);

    static void to_string(const Array &array)
    {
        for (auto &amf : array)
        {
            // fmt::print("{}\n", amf->to_string());
        }
        // fmt::print("\n");
    }
};

class AmfNumber : public AmfValue
{
public:
    using SPtr = std::shared_ptr<AmfNumber>;

public:
    double value;
    AmfNumber(double v = 0) : AmfValue(AmfType::Number), value(v) {}
    std::string to_string(int nestLevel = 0) override;
    int Decode(const uint8_t *in, size_t len) override
    {
        int offset = 0;
        uint64_t bits = NumberCodec::DecodeU64(in, len, offset);
        std::memcpy(&value, &bits, sizeof(double));
        return offset;
    }
    std::vector<uint8_t> Encode() override
    {
        std::vector<uint8_t> buffer = {static_cast<uint8_t>(type)};
        uint64_t bits = 0;
        std::memcpy(&bits, &value, sizeof(double));
        NumberCodec::EncodeU64(buffer, bits);
        return buffer;
    }
};

class AmfBoolean : public AmfValue
{
public:
    using SPtr = std::shared_ptr<AmfBoolean>;

public:
    bool value;
    AmfBoolean(bool v = false) : AmfValue(AmfType::Boolean), value(v) {}
    std::string to_string(int nestLevel = 0);
    int Decode(const uint8_t *in, size_t len) override
    {
        int offset = 0;
        value = static_cast<uint8_t>(in[offset++]) != 0;
        return offset;
    }
    std::vector<uint8_t> Encode() override
    {
        std::vector<uint8_t> buffer(1 + sizeof(bool));
        buffer[0] = static_cast<uint8_t>(type);
        buffer[1] = static_cast<uint8_t>(value);
        return buffer;
    }
};

class AmfString : public AmfValue
{
public:
    using SPtr = std::shared_ptr<AmfString>;

public:
    std::string value;
    AmfString(std::string v = "") : AmfValue(AmfType::String), value(v) {}
    std::string to_string(int nestLevel = 0) override;
    int Decode(const uint8_t *in, size_t len) override
    {
        int offset = 0;
        value = DecodeString(in, len, offset);
        return offset;
    }
    std::vector<uint8_t> Encode() override
    {
        std::vector<uint8_t> buffer = {static_cast<uint8_t>(type)};
        EncodeString(buffer, value);
        return buffer;
    }
};

class AmfNull : public AmfValue
{
public:
    using SPtr = std::shared_ptr<AmfNull>;

public:
    AmfNull() : AmfValue(AmfType::Null) {}
    std::string to_string(int nestLevel = 0) override;
    int Decode(const uint8_t *in, size_t len) override { return 0; }
    std::vector<uint8_t> Encode() override { return {static_cast<uint8_t>(type)}; }
};

template <bool isEcma>
class AmfObjectLike : public AmfValue
{
public:
    using SPtr = std::shared_ptr<AmfObjectLike<isEcma>>;

public:
    std::map<std::string, AmfValue::SPtr> value;
    AmfObjectLike() : AmfValue(isEcma ? AmfType::EcmaArray : AmfType::Object) {}
    std::string to_string(int nestLevel = 0) override;
    int Decode(const uint8_t *in, size_t len) override
    {
        int offset = 0;
        if constexpr (isEcma)
        {
            /* auto size = */ NumberCodec::DecodeU32(in, len, offset);
        }
        while (!IsObjectEndMarker(in, len, offset))
        {
            auto key = DecodeString(in, len, offset); // 读取 key
            auto val = DecodeValue(in, len, offset);  // 读取 value
            value[key] = val;
        }
        return offset;
    }
    std::vector<uint8_t> Encode() override
    {
        // 类型
        std::vector<uint8_t> buffer = {static_cast<uint8_t>(type)};
        if constexpr (isEcma)
        {
            NumberCodec::EncodeU32(buffer, value.size());
        }
        // key-value
        for (auto &[key, val] : value)
        {
            EncodeString(buffer, key);
            EncodeValue(buffer, val);
        }
        // 结束标志
        buffer.insert(buffer.end(), {0x00, 0x00, 0x09});
        return buffer;
    }
};
using AmfObject = AmfObjectLike<false>;
using AmfEcmaArray = AmfObjectLike<true>;

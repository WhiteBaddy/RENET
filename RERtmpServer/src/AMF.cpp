#include "AMF.h"

template class AmfObjectLike<false>;
template class AmfObjectLike<true>;

AmfValue::Array AmfValue::DecodeArray(const uint8_t *in, size_t len, int &offset)
{
    Array array;
    while (offset < len)
    {
        auto val = DecodeValue(in, len, offset);
        if (val == nullptr)
        {
            break;
        }
        array.push_back(val);
    }
    return array;
}

AmfValue::SPtr AmfValue::DecodeValue(const uint8_t *in, size_t len, int &offset)
{
    auto amftype = static_cast<AmfType>(in[offset]);
    SPtr val;
    switch (amftype)
    {
    case AmfType::Number:
        val = std::make_shared<AmfNumber>();
        break;
    case AmfType::Boolean:
        val = std::make_shared<AmfBoolean>();
        break;
    case AmfType::String:
        val = std::make_shared<AmfString>();
        break;
    case AmfType::Object:
        val = std::make_shared<AmfObject>();
        break;
    case AmfType::Null:
        val = std::make_shared<AmfNull>();
        break;
    case AmfType::EcmaArray:
        val = std::make_shared<AmfEcmaArray>();
        break;
    default:
        break;
    }
    if (val != nullptr)
    {
        ++offset;
        auto ret = val->Decode(in + offset, len - offset);
        offset += ret;
    }
    return val;
}

void AmfValue::EncodeArray(std::vector<uint8_t> &buf, const AmfValue::Array &array)
{
    for (const auto &val : array)
    {
        EncodeValue(buf, val);
    }
}

void AmfValue::EncodeValue(std::vector<uint8_t> &buf, const AmfValue::SPtr &value)
{
    auto buffer = value->Encode();
    buf.insert(buf.end(), buffer.begin(), buffer.end());
}

std::string AmfValue::DecodeString(const uint8_t *in, size_t len, int &offset)
{
    uint16_t strLen = UintCodec::DecodeU16(in, len, offset);
    std::string str(in + offset, in + offset + strLen);
    offset += strLen;
    return str;
}

void AmfValue::EncodeString(std::vector<uint8_t> &buf, const std::string &str)
{
    UintCodec::EncodeU16(buf, str.size());
    buf.insert(buf.end(), str.begin(), str.end());
}

bool AmfValue::IsObjectEndMarker(const uint8_t *in, size_t len, int &offset)
{

    if (offset + 3 <= len &&
        in[offset] == 0x00 &&
        in[offset + 1] == 0x00 &&
        in[offset + 2] == 0x09)
    {
        offset += 3;
        return true;
    }
    return false;
}

std::string AmfNumber::to_string(int nestLevel)
{
    std::string tab(nestLevel, '\t');
    std::string ret = fmt::format(
        "{{\n"
        "{0}\tType : Number,\n"
        "{0}\tValue : {1}\n"
        "{0}}}",
        tab, value);
    return ret;
}

std::string AmfBoolean::to_string(int nestLevel)
{
    std::string tab(nestLevel, '\t');
    std::string ret = fmt::format(
        "{{\n"
        "{0}\tType : Boolean,\n"
        "{0}\tValue : {1}\n"
        "{0}}}",
        tab, value ? "True" : "False");
    return ret;
}

std::string AmfString::to_string(int nestLevel)
{
    std::string tab(nestLevel, '\t');
    std::string ret = fmt::format(
        "{{\n"
        "{0}\tType : String,\n"
        "{0}\tValue : {1}\n"
        "{0}}}",
        tab, value);
    return ret;
}

std::string AmfNull::to_string(int nestLevel)
{
    std::string tab(nestLevel, '\t');
    std::string ret = fmt::format(
        "{{\n"
        "{0}\tType : Null\n"
        "{0}}}",
        tab);
    return ret;
}

template <bool isEcma>
std::string AmfObjectLike<isEcma>::to_string(int nestLevel)
{
    std::string tab(nestLevel, '\t');
    std::string valueString;
    valueString += "[\n";
    for (auto &[k, v] : value)
    {
        valueString += fmt::format(
            "{0}\t{{\n"
            "{0}\t\tKey : {1},\n"
            "{0}\t\tValue : \n"
            "{0}\t\t{2}\n"
            "{0}\t}},\n",
            tab, k, v->to_string(nestLevel + 2));
    }
    valueString += tab + "\t]";
    std::string ret = fmt::format(
        "{{\n"
        "{0}\tType : {1},\n"
        "{0}\tSize : {2},\n"
        "{0}\tValue : {3}\n"
        "{0}}}",
        tab,
        isEcma ? "EcmaArray" : "Object",
        value.size(),
        valueString);
    return ret;
}

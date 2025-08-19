#include "Tools/UintCodec.h"

uint8_t UintCodec::DecodeU8(const uint8_t *in, size_t len, int &offset)
{
    return static_cast<uint16_t>(in[offset++]);
}

uint16_t UintCodec::DecodeU16(const uint8_t *in, size_t len, int &offset)
{
    return (static_cast<uint16_t>(in[offset++]) << 8) |
           static_cast<uint16_t>(in[offset++]);
}

uint32_t UintCodec::DecodeU24(const uint8_t *in, size_t len, int &offset)
{
    return (static_cast<uint32_t>(in[offset++]) << 16) |
           (static_cast<uint32_t>(in[offset++]) << 8) |
           static_cast<uint32_t>(in[offset++]);
}

uint32_t UintCodec::DecodeU32(const uint8_t *in, size_t len, int &offset)
{
    return (static_cast<uint32_t>(in[offset++]) << 24) |
           (static_cast<uint32_t>(in[offset++]) << 16) |
           (static_cast<uint32_t>(in[offset++]) << 8) |
           static_cast<uint32_t>(in[offset++]);
}

uint64_t UintCodec::DecodeU64(const uint8_t *in, size_t len, int &offset)
{
    return (static_cast<uint64_t>(in[offset++]) << 56) |
           (static_cast<uint64_t>(in[offset++]) << 48) |
           (static_cast<uint64_t>(in[offset++]) << 40) |
           (static_cast<uint64_t>(in[offset++]) << 32) |
           (static_cast<uint64_t>(in[offset++]) << 24) |
           (static_cast<uint64_t>(in[offset++]) << 16) |
           (static_cast<uint64_t>(in[offset++]) << 8) |
           static_cast<uint64_t>(in[offset++]);
}

void UintCodec::EncodeU8(std::vector<uint8_t> &buf, uint8_t val)
{
    buf.insert(buf.end(),
               {static_cast<uint8_t>(val & 0xFF)});
}

void UintCodec::EncodeU16(std::vector<uint8_t> &buf, uint16_t val)
{
    buf.insert(buf.end(),
               {static_cast<uint8_t>((val >> 8) & 0xFF),
                static_cast<uint8_t>(val & 0xFF)});
}

void UintCodec::EncodeU24(std::vector<uint8_t> &buf, uint32_t val)
{
    buf.insert(buf.end(),
               {static_cast<uint8_t>((val >> 16) & 0xFF),
                static_cast<uint8_t>((val >> 8) & 0xFF),
                static_cast<uint8_t>(val & 0xFF)});
}

void UintCodec::EncodeU32(std::vector<uint8_t> &buf, uint32_t val)
{
    buf.insert(buf.end(),
               {static_cast<uint8_t>((val >> 24) & 0xFF),
                static_cast<uint8_t>((val >> 16) & 0xFF),
                static_cast<uint8_t>((val >> 8) & 0xFF),
                static_cast<uint8_t>(val & 0xFF)});
}

void UintCodec::EncodeU64(std::vector<uint8_t> &buf, uint64_t val)
{
    buf.insert(buf.end(),
               {static_cast<uint8_t>((val >> 56) & 0xFF),
                static_cast<uint8_t>((val >> 48) & 0xFF),
                static_cast<uint8_t>((val >> 40) & 0xFF),
                static_cast<uint8_t>((val >> 32) & 0xFF),
                static_cast<uint8_t>((val >> 24) & 0xFF),
                static_cast<uint8_t>((val >> 16) & 0xFF),
                static_cast<uint8_t>((val >> 8) & 0xFF),
                static_cast<uint8_t>(val & 0xFF)});
}

uint16_t UintCodec::DecodeU16LE(const uint8_t *in, size_t len, int &offset)
{
    return static_cast<uint16_t>(in[offset++]) |
           (static_cast<uint16_t>(in[offset++]) << 8);
}

uint32_t UintCodec::DecodeU24LE(const uint8_t *in, size_t len, int &offset)
{
    return static_cast<uint32_t>(in[offset++]) |
           (static_cast<uint32_t>(in[offset++]) << 8) |
           (static_cast<uint32_t>(in[offset++]) << 16);
}

uint32_t UintCodec::DecodeU32LE(const uint8_t *in, size_t len, int &offset)
{
    return static_cast<uint32_t>(in[offset++]) |
           (static_cast<uint32_t>(in[offset++]) << 8) |
           (static_cast<uint32_t>(in[offset++]) << 16) |
           (static_cast<uint32_t>(in[offset++]) << 24);
}

uint64_t UintCodec::DecodeU64LE(const uint8_t *in, size_t len, int &offset)
{
    return static_cast<uint64_t>(in[offset++]) |
           (static_cast<uint64_t>(in[offset++]) << 8) |
           (static_cast<uint64_t>(in[offset++]) << 16) |
           (static_cast<uint64_t>(in[offset++]) << 24) |
           (static_cast<uint64_t>(in[offset++]) << 32) |
           (static_cast<uint64_t>(in[offset++]) << 40) |
           (static_cast<uint64_t>(in[offset++]) << 48) |
           (static_cast<uint64_t>(in[offset++]) << 56);
}

void UintCodec::EncodeU16LE(std::vector<uint8_t> &buf, uint16_t val)
{
    buf.insert(buf.end(),
               {static_cast<uint8_t>(val & 0xFF),
                static_cast<uint8_t>((val >> 8) & 0xFF)});
}

void UintCodec::EncodeU24LE(std::vector<uint8_t> &buf, uint32_t val)
{
    buf.insert(buf.end(),
               {static_cast<uint8_t>(val & 0xFF),
                static_cast<uint8_t>((val >> 8) & 0xFF),
                static_cast<uint8_t>((val >> 16) & 0xFF)});
}

void UintCodec::EncodeU32LE(std::vector<uint8_t> &buf, uint32_t val)
{
    buf.insert(buf.end(),
               {static_cast<uint8_t>(val & 0xFF),
                static_cast<uint8_t>((val >> 8) & 0xFF),
                static_cast<uint8_t>((val >> 16) & 0xFF),
                static_cast<uint8_t>((val >> 24) & 0xFF)});
}

void UintCodec::EncodeU64LE(std::vector<uint8_t> &buf, uint64_t val)
{
    buf.insert(buf.end(),
               {static_cast<uint8_t>(val & 0xFF),
                static_cast<uint8_t>((val >> 8) & 0xFF),
                static_cast<uint8_t>((val >> 16) & 0xFF),
                static_cast<uint8_t>((val >> 24) & 0xFF),
                static_cast<uint8_t>((val >> 32) & 0xFF),
                static_cast<uint8_t>((val >> 40) & 0xFF),
                static_cast<uint8_t>((val >> 48) & 0xFF),
                static_cast<uint8_t>((val >> 56) & 0xFF)});
}

void UintCodec::EncodeU8(std::vector<uint8_t> &buf, size_t idx, uint8_t val)
{
    size_t endPos = idx + 1;
    if (buf.size() < endPos)
    {
        buf.resize(endPos);
    }
    std::vector<uint8_t> tmp = {static_cast<uint8_t>(val & 0xFF)};
    std::copy(tmp.begin(), tmp.end(), buf.begin() + idx);
}

void UintCodec::EncodeU16(std::vector<uint8_t> &buf, size_t idx, uint16_t val)
{
    size_t endPos = idx + 2;
    if (buf.size() < endPos)
    {
        buf.resize(endPos);
    }
    std::vector<uint8_t> tmp = {static_cast<uint8_t>((val >> 8) & 0xFF),
                                static_cast<uint8_t>(val & 0xFF)};
    std::copy(tmp.begin(), tmp.end(), buf.begin() + idx);
}

void UintCodec::EncodeU24(std::vector<uint8_t> &buf, size_t idx, uint32_t val)
{
    size_t endPos = idx + 3;
    if (buf.size() < endPos)
    {
        buf.resize(endPos);
    }
    std::vector<uint8_t> tmp = {static_cast<uint8_t>((val >> 16) & 0xFF),
                                static_cast<uint8_t>((val >> 8) & 0xFF),
                                static_cast<uint8_t>(val & 0xFF)};
    std::copy(tmp.begin(), tmp.end(), buf.begin() + idx);
}

void UintCodec::EncodeU32(std::vector<uint8_t> &buf, size_t idx, uint32_t val)
{
    size_t endPos = idx + 4;
    if (buf.size() < endPos)
    {
        buf.resize(endPos);
    }
    std::vector<uint8_t> tmp = {static_cast<uint8_t>((val >> 24) & 0xFF),
                                static_cast<uint8_t>((val >> 16) & 0xFF),
                                static_cast<uint8_t>((val >> 8) & 0xFF),
                                static_cast<uint8_t>(val & 0xFF)};
    std::copy(tmp.begin(), tmp.end(), buf.begin() + idx);
}

void UintCodec::EncodeU64(std::vector<uint8_t> &buf, size_t idx, uint64_t val)
{
    size_t endPos = idx + 8;
    if (buf.size() < endPos)
    {
        buf.resize(endPos);
    }
    std::vector<uint8_t> tmp = {static_cast<uint8_t>((val >> 56) & 0xFF),
                                static_cast<uint8_t>((val >> 48) & 0xFF),
                                static_cast<uint8_t>((val >> 40) & 0xFF),
                                static_cast<uint8_t>((val >> 32) & 0xFF),
                                static_cast<uint8_t>((val >> 24) & 0xFF),
                                static_cast<uint8_t>((val >> 16) & 0xFF),
                                static_cast<uint8_t>((val >> 8) & 0xFF),
                                static_cast<uint8_t>(val & 0xFF)};
    std::copy(tmp.begin(), tmp.end(), buf.begin() + idx);
}

void UintCodec::EncodeU16LE(std::vector<uint8_t> &buf, size_t idx, uint16_t val)
{
    size_t endPos = idx + 2;
    if (buf.size() < endPos)
    {
        buf.resize(endPos);
    }
    std::vector<uint8_t> tmp = {static_cast<uint8_t>(val & 0xFF),
                                static_cast<uint8_t>((val >> 8) & 0xFF)};
    std::copy(tmp.begin(), tmp.end(), buf.begin() + idx);
}

void UintCodec::EncodeU24LE(std::vector<uint8_t> &buf, size_t idx, uint32_t val)
{
    size_t endPos = idx + 3;
    if (buf.size() < endPos)
    {
        buf.resize(endPos);
    }
    std::vector<uint8_t> tmp = {static_cast<uint8_t>(val & 0xFF),
                                static_cast<uint8_t>((val >> 8) & 0xFF),
                                static_cast<uint8_t>((val >> 16) & 0xFF)};
    std::copy(tmp.begin(), tmp.end(), buf.begin() + idx);
}

void UintCodec::EncodeU32LE(std::vector<uint8_t> &buf, size_t idx, uint32_t val)
{
    size_t endPos = idx + 4;
    if (buf.size() < endPos)
    {
        buf.resize(endPos);
    }
    std::vector<uint8_t> tmp = {static_cast<uint8_t>(val & 0xFF),
                                static_cast<uint8_t>((val >> 8) & 0xFF),
                                static_cast<uint8_t>((val >> 16) & 0xFF),
                                static_cast<uint8_t>((val >> 24) & 0xFF)};
    std::copy(tmp.begin(), tmp.end(), buf.begin() + idx);
}

void UintCodec::EncodeU64LE(std::vector<uint8_t> &buf, size_t idx, uint64_t val)
{
    size_t endPos = idx + 8;
    if (buf.size() < endPos)
    {
        buf.resize(endPos);
    }
    std::vector<uint8_t> tmp = {static_cast<uint8_t>(val & 0xFF),
                                static_cast<uint8_t>((val >> 8) & 0xFF),
                                static_cast<uint8_t>((val >> 16) & 0xFF),
                                static_cast<uint8_t>((val >> 24) & 0xFF),
                                static_cast<uint8_t>((val >> 32) & 0xFF),
                                static_cast<uint8_t>((val >> 40) & 0xFF),
                                static_cast<uint8_t>((val >> 48) & 0xFF),
                                static_cast<uint8_t>((val >> 56) & 0xFF)};
    std::copy(tmp.begin(), tmp.end(), buf.begin() + idx);
}

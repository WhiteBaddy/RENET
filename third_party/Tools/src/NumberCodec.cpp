#include "Tools/NumberCodec.h"
#include <cstring>

uint8_t NumberCodec::DecodeU8(const uint8_t *in, size_t len, int &offset)
{
    return static_cast<uint16_t>(in[offset++]);
}

uint16_t NumberCodec::DecodeU16(const uint8_t *in, size_t len, int &offset)
{
    return (static_cast<uint16_t>(in[offset++]) << 8) |
           static_cast<uint16_t>(in[offset++]);
}

uint32_t NumberCodec::DecodeU24(const uint8_t *in, size_t len, int &offset)
{
    return (static_cast<uint32_t>(in[offset++]) << 16) |
           (static_cast<uint32_t>(in[offset++]) << 8) |
           static_cast<uint32_t>(in[offset++]);
}

uint32_t NumberCodec::DecodeU32(const uint8_t *in, size_t len, int &offset)
{
    return (static_cast<uint32_t>(in[offset++]) << 24) |
           (static_cast<uint32_t>(in[offset++]) << 16) |
           (static_cast<uint32_t>(in[offset++]) << 8) |
           static_cast<uint32_t>(in[offset++]);
}

uint64_t NumberCodec::DecodeU64(const uint8_t *in, size_t len, int &offset)
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

uint16_t NumberCodec::DecodeU16LE(const uint8_t *in, size_t len, int &offset)
{
    return static_cast<uint16_t>(in[offset++]) |
           (static_cast<uint16_t>(in[offset++]) << 8);
}

uint32_t NumberCodec::DecodeU24LE(const uint8_t *in, size_t len, int &offset)
{
    return static_cast<uint32_t>(in[offset++]) |
           (static_cast<uint32_t>(in[offset++]) << 8) |
           (static_cast<uint32_t>(in[offset++]) << 16);
}

uint32_t NumberCodec::DecodeU32LE(const uint8_t *in, size_t len, int &offset)
{
    return static_cast<uint32_t>(in[offset++]) |
           (static_cast<uint32_t>(in[offset++]) << 8) |
           (static_cast<uint32_t>(in[offset++]) << 16) |
           (static_cast<uint32_t>(in[offset++]) << 24);
}

uint64_t NumberCodec::DecodeU64LE(const uint8_t *in, size_t len, int &offset)
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

void NumberCodec::EncodeU8(std::vector<uint8_t> &buf, uint8_t val)
{
    buf.insert(buf.end(),
               {static_cast<uint8_t>(val & 0xFF)});
}

void NumberCodec::EncodeU16(std::vector<uint8_t> &buf, uint16_t val)
{
    buf.insert(buf.end(),
               {static_cast<uint8_t>((val >> 8) & 0xFF),
                static_cast<uint8_t>(val & 0xFF)});
}

void NumberCodec::EncodeU24(std::vector<uint8_t> &buf, uint32_t val)
{
    buf.insert(buf.end(),
               {static_cast<uint8_t>((val >> 16) & 0xFF),
                static_cast<uint8_t>((val >> 8) & 0xFF),
                static_cast<uint8_t>(val & 0xFF)});
}

void NumberCodec::EncodeU32(std::vector<uint8_t> &buf, uint32_t val)
{
    buf.insert(buf.end(),
               {static_cast<uint8_t>((val >> 24) & 0xFF),
                static_cast<uint8_t>((val >> 16) & 0xFF),
                static_cast<uint8_t>((val >> 8) & 0xFF),
                static_cast<uint8_t>(val & 0xFF)});
}

void NumberCodec::EncodeU64(std::vector<uint8_t> &buf, uint64_t val)
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

void NumberCodec::EncodeU16LE(std::vector<uint8_t> &buf, uint16_t val)
{
    buf.insert(buf.end(),
               {static_cast<uint8_t>(val & 0xFF),
                static_cast<uint8_t>((val >> 8) & 0xFF)});
}

void NumberCodec::EncodeU24LE(std::vector<uint8_t> &buf, uint32_t val)
{
    buf.insert(buf.end(),
               {static_cast<uint8_t>(val & 0xFF),
                static_cast<uint8_t>((val >> 8) & 0xFF),
                static_cast<uint8_t>((val >> 16) & 0xFF)});
}

void NumberCodec::EncodeU32LE(std::vector<uint8_t> &buf, uint32_t val)
{
    buf.insert(buf.end(),
               {static_cast<uint8_t>(val & 0xFF),
                static_cast<uint8_t>((val >> 8) & 0xFF),
                static_cast<uint8_t>((val >> 16) & 0xFF),
                static_cast<uint8_t>((val >> 24) & 0xFF)});
}

void NumberCodec::EncodeU64LE(std::vector<uint8_t> &buf, uint64_t val)
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

void NumberCodec::EncodeU8(std::vector<uint8_t> &buf, size_t idx, uint8_t val)
{
    size_t endPos = idx + 1;
    if (buf.size() < endPos)
    {
        buf.resize(endPos);
    }
    std::vector<uint8_t> tmp = {static_cast<uint8_t>(val & 0xFF)};
    std::copy(tmp.begin(), tmp.end(), buf.begin() + idx);
}

void NumberCodec::EncodeU16(std::vector<uint8_t> &buf, size_t idx, uint16_t val)
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

void NumberCodec::EncodeU24(std::vector<uint8_t> &buf, size_t idx, uint32_t val)
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

void NumberCodec::EncodeU32(std::vector<uint8_t> &buf, size_t idx, uint32_t val)
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

void NumberCodec::EncodeU64(std::vector<uint8_t> &buf, size_t idx, uint64_t val)
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

void NumberCodec::EncodeU16LE(std::vector<uint8_t> &buf, size_t idx, uint16_t val)
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

void NumberCodec::EncodeU24LE(std::vector<uint8_t> &buf, size_t idx, uint32_t val)
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

void NumberCodec::EncodeU32LE(std::vector<uint8_t> &buf, size_t idx, uint32_t val)
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

void NumberCodec::EncodeU64LE(std::vector<uint8_t> &buf, size_t idx, uint64_t val)
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

int8_t NumberCodec::DecodeI8(const uint8_t *in, size_t len, int &offset)
{
    int8_t val = 0;
    auto tmp = DecodeU8(in, len, offset);
    memcpy(&val, &tmp, sizeof(int8_t));
    return val;
}

int16_t NumberCodec::DecodeI16(const uint8_t *in, size_t len, int &offset)
{
    int16_t val = 0;
    auto tmp = DecodeU16(in, len, offset);
    memcpy(&val, &tmp, sizeof(int16_t));
    return val;
}

int32_t NumberCodec::DecodeI32(const uint8_t *in, size_t len, int &offset)
{
    int32_t val = 0;
    auto tmp = DecodeU32(in, len, offset);
    memcpy(&val, &tmp, sizeof(int32_t));
    return val;
}

int64_t NumberCodec::DecodeI64(const uint8_t *in, size_t len, int &offset)
{
    int64_t val = 0;
    auto tmp = DecodeU64(in, len, offset);
    memcpy(&val, &tmp, sizeof(int64_t));
    return val;
}

int16_t NumberCodec::DecodeI16LE(const uint8_t *in, size_t len, int &offset)
{
    int16_t val = 0;
    auto tmp = DecodeU16LE(in, len, offset);
    memcpy(&val, &tmp, sizeof(int16_t));
    return val;
}

int32_t NumberCodec::DecodeI32LE(const uint8_t *in, size_t len, int &offset)
{
    int32_t val = 0;
    auto tmp = DecodeU32LE(in, len, offset);
    memcpy(&val, &tmp, sizeof(int32_t));
    return val;
}

int64_t NumberCodec::DecodeI64LE(const uint8_t *in, size_t len, int &offset)
{
    int64_t val = 0;
    auto tmp = DecodeU64LE(in, len, offset);
    memcpy(&val, &tmp, sizeof(int64_t));
    return val;
}

void NumberCodec::EncodeI8(std::vector<uint8_t> &buf, int8_t val)
{
    uint8_t tmp = 0;
    memcpy(&tmp, &val, sizeof(int8_t));
    EncodeU8(buf, tmp);
}

void NumberCodec::EncodeI16(std::vector<uint8_t> &buf, int16_t val)
{
    uint16_t tmp = 0;
    memcpy(&tmp, &val, sizeof(int16_t));
    EncodeU16(buf, tmp);
}

void NumberCodec::EncodeI32(std::vector<uint8_t> &buf, int32_t val)
{
    uint32_t tmp = 0;
    memcpy(&tmp, &val, sizeof(int32_t));
    EncodeU32(buf, tmp);
}

void NumberCodec::EncodeI64(std::vector<uint8_t> &buf, int64_t val)
{
    uint64_t tmp = 0;
    memcpy(&tmp, &val, sizeof(int64_t));
    EncodeU64(buf, tmp);
}

void NumberCodec::EncodeI16LE(std::vector<uint8_t> &buf, int16_t val)
{
    uint16_t tmp = 0;
    memcpy(&tmp, &val, sizeof(int16_t));
    EncodeU16LE(buf, tmp);
}

void NumberCodec::EncodeI32LE(std::vector<uint8_t> &buf, int32_t val)
{
    uint32_t tmp = 0;
    memcpy(&tmp, &val, sizeof(int32_t));
    EncodeU32LE(buf, tmp);
}

void NumberCodec::EncodeI64LE(std::vector<uint8_t> &buf, int64_t val)
{
    uint64_t tmp = 0;
    memcpy(&tmp, &val, sizeof(int64_t));
    EncodeU64LE(buf, tmp);
}

void NumberCodec::EncodeI8(std::vector<uint8_t> &buf, size_t idx, int8_t val)
{
    uint8_t tmp = 0;
    memcpy(&tmp, &val, sizeof(int8_t));
    EncodeU8(buf, idx, tmp);
}

void NumberCodec::EncodeI16(std::vector<uint8_t> &buf, size_t idx, int16_t val)
{
    uint16_t tmp = 0;
    memcpy(&tmp, &val, sizeof(int16_t));
    EncodeU16(buf, idx, tmp);
}

void NumberCodec::EncodeI32(std::vector<uint8_t> &buf, size_t idx, int32_t val)
{
    uint32_t tmp = 0;
    memcpy(&tmp, &val, sizeof(int32_t));
    EncodeU32(buf, idx, tmp);
}

void NumberCodec::EncodeI64(std::vector<uint8_t> &buf, size_t idx, int64_t val)
{
    uint64_t tmp = 0;
    memcpy(&tmp, &val, sizeof(int64_t));
    EncodeU64(buf, idx, tmp);
}

void NumberCodec::EncodeI16LE(std::vector<uint8_t> &buf, size_t idx, int16_t val)
{
    uint16_t tmp = 0;
    memcpy(&tmp, &val, sizeof(int16_t));
    EncodeU16LE(buf, idx, tmp);
}

void NumberCodec::EncodeI32LE(std::vector<uint8_t> &buf, size_t idx, int32_t val)
{
    uint32_t tmp = 0;
    memcpy(&tmp, &val, sizeof(int32_t));
    EncodeU32LE(buf, idx, tmp);
}

void NumberCodec::EncodeI64LE(std::vector<uint8_t> &buf, size_t idx, int64_t val)
{
    uint64_t tmp = 0;
    memcpy(&tmp, &val, sizeof(int64_t));
    EncodeU64LE(buf, idx, tmp);
}

float NumberCodec::DecodeF32(const uint8_t *in, size_t len, int &offset)
{
    float val = 0;
    uint32_t tmp = DecodeU32(in, len, offset);
    memcpy(&val, &tmp, sizeof(float));
    return val;
}

double NumberCodec::DecodeF64(const uint8_t *in, size_t len, int &offset)
{
    double val = 0;
    uint64_t tmp = DecodeU64(in, len, offset);
    memcpy(&val, &tmp, sizeof(double));
    return val;
}

float NumberCodec::DecodeF32LE(const uint8_t *in, size_t len, int &offset)
{
    float val = 0;
    uint32_t tmp = DecodeU32LE(in, len, offset);
    memcpy(&val, &tmp, sizeof(float));
    return val;
}

double NumberCodec::DecodeF64LE(const uint8_t *in, size_t len, int &offset)
{
    double val = 0;
    uint64_t tmp = DecodeU64LE(in, len, offset);
    memcpy(&val, &tmp, sizeof(double));
    return val;
}

void NumberCodec::EncodeF32(std::vector<uint8_t> &buf, float val)
{
    uint32_t tmp = 0;
    memcpy(&tmp, &val, sizeof(float));
    EncodeU32(buf, tmp);
}

void NumberCodec::EncodeF64(std::vector<uint8_t> &buf, double val)
{
    uint64_t tmp = 0;
    memcpy(&tmp, &val, sizeof(double));
    EncodeU64(buf, tmp);
}

void NumberCodec::EncodeF32LE(std::vector<uint8_t> &buf, float val)
{
    uint32_t tmp = 0;
    memcpy(&tmp, &val, sizeof(float));
    EncodeU32LE(buf, tmp);
}

void NumberCodec::EncodeF64LE(std::vector<uint8_t> &buf, double val)
{
    uint64_t tmp = 0;
    memcpy(&tmp, &val, sizeof(double));
    EncodeU64LE(buf, tmp);
}

void NumberCodec::EncodeF32(std::vector<uint8_t> &buf, size_t idx, float val)
{
    uint32_t tmp = 0;
    memcpy(&tmp, &val, sizeof(float));
    EncodeU32(buf, idx, tmp);
}

void NumberCodec::EncodeF64(std::vector<uint8_t> &buf, size_t idx, double val)
{
    uint64_t tmp = 0;
    memcpy(&tmp, &val, sizeof(double));
    EncodeU64(buf, idx, tmp);
}

void NumberCodec::EncodeF32LE(std::vector<uint8_t> &buf, size_t idx, float val)
{
    uint32_t tmp = 0;
    memcpy(&tmp, &val, sizeof(float));
    EncodeU32LE(buf, idx, tmp);
}

void NumberCodec::EncodeF64LE(std::vector<uint8_t> &buf, size_t idx, double val)
{
    uint64_t tmp = 0;
    memcpy(&tmp, &val, sizeof(double));
    EncodeU64LE(buf, idx, tmp);
}

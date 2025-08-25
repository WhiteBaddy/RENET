#pragma once
#include <vector>
#include <cstdint>
#include <cstddef>
class NumberCodec
{
public:
    static uint8_t DecodeU8(const uint8_t *in, size_t len, int &offset);
    static uint16_t DecodeU16(const uint8_t *in, size_t len, int &offset);
    static uint32_t DecodeU24(const uint8_t *in, size_t len, int &offset);
    static uint32_t DecodeU32(const uint8_t *in, size_t len, int &offset);
    static uint64_t DecodeU64(const uint8_t *in, size_t len, int &offset);
    static uint16_t DecodeU16LE(const uint8_t *in, size_t len, int &offset);
    static uint32_t DecodeU24LE(const uint8_t *in, size_t len, int &offset);
    static uint32_t DecodeU32LE(const uint8_t *in, size_t len, int &offset);
    static uint64_t DecodeU64LE(const uint8_t *in, size_t len, int &offset);
    static void EncodeU8(std::vector<uint8_t> &buf, uint8_t val);
    static void EncodeU16(std::vector<uint8_t> &buf, uint16_t val);
    static void EncodeU24(std::vector<uint8_t> &buf, uint32_t val);
    static void EncodeU32(std::vector<uint8_t> &buf, uint32_t val);
    static void EncodeU64(std::vector<uint8_t> &buf, uint64_t val);
    static void EncodeU16LE(std::vector<uint8_t> &buf, uint16_t val);
    static void EncodeU24LE(std::vector<uint8_t> &buf, uint32_t val);
    static void EncodeU32LE(std::vector<uint8_t> &buf, uint32_t val);
    static void EncodeU64LE(std::vector<uint8_t> &buf, uint64_t val);
    static void EncodeU8(std::vector<uint8_t> &buf, size_t idx, uint8_t val);
    static void EncodeU16(std::vector<uint8_t> &buf, size_t idx, uint16_t val);
    static void EncodeU24(std::vector<uint8_t> &buf, size_t idx, uint32_t val);
    static void EncodeU32(std::vector<uint8_t> &buf, size_t idx, uint32_t val);
    static void EncodeU64(std::vector<uint8_t> &buf, size_t idx, uint64_t val);
    static void EncodeU16LE(std::vector<uint8_t> &buf, size_t idx, uint16_t val);
    static void EncodeU24LE(std::vector<uint8_t> &buf, size_t idx, uint32_t val);
    static void EncodeU32LE(std::vector<uint8_t> &buf, size_t idx, uint32_t val);
    static void EncodeU64LE(std::vector<uint8_t> &buf, size_t idx, uint64_t val);

    static int8_t DecodeI8(const uint8_t *in, size_t len, int &offset);
    static int16_t DecodeI16(const uint8_t *in, size_t len, int &offset);
    // static int32_t DecodeI24(const uint8_t *in, size_t len, int &offset);
    static int32_t DecodeI32(const uint8_t *in, size_t len, int &offset);
    static int64_t DecodeI64(const uint8_t *in, size_t len, int &offset);
    static int16_t DecodeI16LE(const uint8_t *in, size_t len, int &offset);
    // static int32_t DecodeI24LE(const uint8_t *in, size_t len, int &offset);
    static int32_t DecodeI32LE(const uint8_t *in, size_t len, int &offset);
    static int64_t DecodeI64LE(const uint8_t *in, size_t len, int &offset);
    static void EncodeI8(std::vector<uint8_t> &buf, int8_t val);
    static void EncodeI16(std::vector<uint8_t> &buf, int16_t val);
    // static void EncodeI24(std::vector<uint8_t> &buf, int32_t val);
    static void EncodeI32(std::vector<uint8_t> &buf, int32_t val);
    static void EncodeI64(std::vector<uint8_t> &buf, int64_t val);
    static void EncodeI16LE(std::vector<uint8_t> &buf, int16_t val);
    // static void EncodeI24LE(std::vector<uint8_t> &buf, int32_t val);
    static void EncodeI32LE(std::vector<uint8_t> &buf, int32_t val);
    static void EncodeI64LE(std::vector<uint8_t> &buf, int64_t val);
    static void EncodeI8(std::vector<uint8_t> &buf, size_t idx, int8_t val);
    static void EncodeI16(std::vector<uint8_t> &buf, size_t idx, int16_t val);
    // static void EncodeI24(std::vector<uint8_t> &buf, size_t idx, int32_t val);
    static void EncodeI32(std::vector<uint8_t> &buf, size_t idx, int32_t val);
    static void EncodeI64(std::vector<uint8_t> &buf, size_t idx, int64_t val);
    static void EncodeI16LE(std::vector<uint8_t> &buf, size_t idx, int16_t val);
    // static void EncodeI24LE(std::vector<uint8_t> &buf, size_t idx, int32_t val);
    static void EncodeI32LE(std::vector<uint8_t> &buf, size_t idx, int32_t val);
    static void EncodeI64LE(std::vector<uint8_t> &buf, size_t idx, int64_t val);

    static float DecodeF32(const uint8_t *in, size_t len, int &offset);
    static double DecodeF64(const uint8_t *in, size_t len, int &offset);
    static float DecodeF32LE(const uint8_t *in, size_t len, int &offset);
    static double DecodeF64LE(const uint8_t *in, size_t len, int &offset);
    static void EncodeF32(std::vector<uint8_t> &buf, float val);
    static void EncodeF64(std::vector<uint8_t> &buf, double val);
    static void EncodeF32LE(std::vector<uint8_t> &buf, float val);
    static void EncodeF64LE(std::vector<uint8_t> &buf, double val);
    static void EncodeF32(std::vector<uint8_t> &buf, size_t idx, float val);
    static void EncodeF64(std::vector<uint8_t> &buf, size_t idx, double val);
    static void EncodeF32LE(std::vector<uint8_t> &buf, size_t idx, float val);
    static void EncodeF64LE(std::vector<uint8_t> &buf, size_t idx, double val);
};

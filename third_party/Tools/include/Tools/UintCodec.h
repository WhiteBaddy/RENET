#pragma once
#include <vector>
#include <cstdint>
#include <cstddef>
class UintCodec
{
public:
    static uint8_t DecodeU8(const uint8_t *in, size_t len, int &offset);
    static uint16_t DecodeU16(const uint8_t *in, size_t len, int &offset);
    static uint32_t DecodeU24(const uint8_t *in, size_t len, int &offset);
    static uint32_t DecodeU32(const uint8_t *in, size_t len, int &offset);
    static uint64_t DecodeU64(const uint8_t *in, size_t len, int &offset);
    static void EncodeU8(std::vector<uint8_t> &buf, uint8_t val);
    static void EncodeU16(std::vector<uint8_t> &buf, uint16_t val);
    static void EncodeU24(std::vector<uint8_t> &buf, uint32_t val);
    static void EncodeU32(std::vector<uint8_t> &buf, uint32_t val);
    static void EncodeU64(std::vector<uint8_t> &buf, uint64_t val);
    static uint16_t DecodeU16LE(const uint8_t *in, size_t len, int &offset);
    static uint32_t DecodeU24LE(const uint8_t *in, size_t len, int &offset);
    static uint32_t DecodeU32LE(const uint8_t *in, size_t len, int &offset);
    static uint64_t DecodeU64LE(const uint8_t *in, size_t len, int &offset);
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
};

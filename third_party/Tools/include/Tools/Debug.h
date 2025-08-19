#pragma once
#include <cstdint>
#include <cstddef>
class Debug
{
public:
    static void ShowHex(uint8_t *in, size_t len, int rowSize = 16);
};
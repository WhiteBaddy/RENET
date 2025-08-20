#include "Tools/Debug.h"
#include <fmt/core.h>

void Debug::ShowHex(const uint8_t *in, size_t len, int rowSize)
{
    for (int offset = 0; offset < len; offset += rowSize)
    {
        int this_len = std::min(rowSize, (int)len - offset);
        for (int i = 0; i < this_len; ++i)
        {
            fmt::print("{:02X} ", in[i + offset] & 0xFF);
        }
        for (int i = this_len; i < rowSize; ++i)
        {
            fmt::print("   ");
        }
        fmt::print("| ");
        for (int i = 0; i < this_len; ++i)
        {
            fmt::print("{} ", (in[i + offset] > 32 && in[i + offset] < 127) ? static_cast<char>(in[i + offset]) : '.');
        }
        fmt::print("\n");
    }
    fmt::print("\n");
}
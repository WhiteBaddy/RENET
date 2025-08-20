#pragma once
#include <cstdint>
class Time
{
public:
    // 获取当前时间戳（毫秒）
    static uint64_t GetTimestamp_ms();
};
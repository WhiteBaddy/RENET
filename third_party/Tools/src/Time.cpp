#include "Tools/Time.h"
#include <random>

// 获取当前时间戳（毫秒）
uint64_t Time::rtmp_timestamp_ms()
{
    struct timespec ts;
    // CLOCK_MONOTONIC 表示从系统启动开始的单调递增时间
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint64_t ms = (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    return ms;
}

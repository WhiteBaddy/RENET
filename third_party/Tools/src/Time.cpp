#include "Tools/Time.h"
#include <random>
#include <chrono>

// 获取当前时间戳（毫秒）
// uint64_t Time::GetTimestamp_ms()
// {
//     static timespec start_time;
//     static bool initialized = false;
//     if (!initialized)
//     {
//         clock_gettime(CLOCK_MONOTONIC, &start_time);
//         initialized = true;
//     }
//     struct timespec ts;
//     // CLOCK_MONOTONIC 表示从系统启动开始的单调递增时间
//     clock_gettime(CLOCK_MONOTONIC, &ts);
//     uint64_t ms = (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
//     return ms - ((uint64_t)start_time.tv_sec * 1000 + start_time.tv_nsec / 1000000);
// }

uint64_t Time::GetTimestamp_ms()
{
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return duration.count(); // 返回当前时间戳（毫秒）
}

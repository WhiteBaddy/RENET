#include <chrono>
#include <functional>
#include <memory>
#include <map>
#include <thread>
#include <unordered_map>

using TimerEvent = std::function<bool()>;
using TimerId = uint64_t;

class Timer
{
    friend class TimerQueue;

public:
    Timer(const TimerEvent &event = []
          { return false; },
          uint64_t msec = 0) : m_interval(msec), m_timerCallback(event) {}
    ~Timer() = default;
    void SetNextTimeout(uint64_t timePoint) { m_nextTimeout = timePoint + m_interval; }
    uint64_t GetNextTimeout() { return m_nextTimeout; }
    void Sleep(uint64_t msec) { std::this_thread::sleep_for(std::chrono::milliseconds(msec)); }

private:
    uint64_t m_interval;        // 运行间隔
    uint64_t m_nextTimeout;     // 下一次触发时间
    TimerEvent m_timerCallback; // 定时任务，返回值代表是否再次执行
};
using TimerPtr = std::shared_ptr<Timer>;

class TimerQueue
{
public:
    TimerQueue() = default;
    ~TimerQueue() { Close(); }
    int Close()
    {
        m_timers.clear();
        m_events.clear();
        return 0;
    }
    TimerId AddTimer(const TimerEvent &event, uint64_t msec)
    {
        auto timePoint = GetNowTime();
        TimerId id = ++m_lastTimerId;
        TimerPtr timer = std::make_shared<Timer>(event, msec);
        timer->SetNextTimeout(timePoint);
        m_timers.emplace(id, timer);
        m_events.emplace(std::pair<uint64_t, TimerId>(timePoint + msec, id), timer);
        return id;
    }
    void RemoveTimer(TimerId id)
    {
        auto it = m_timers.find(id);
        if (it != m_timers.end())
        {
            uint64_t nextTimeout = it->second->GetNextTimeout();
            m_events.erase(std::pair<uint64_t, TimerId>(nextTimeout, it->first));
            m_timers.erase(it);
        }
    }
    void HandleTimerEvent()
    {
        if (m_timers.empty())
            return;
        auto timePoint = GetNowTime();
        while (!m_timers.empty() && m_events.begin()->first.first <= timePoint)
        {
            TimerId id = m_events.begin()->first.second;
            auto it = m_timers.find(id);
            if (it != m_timers.end())
            {
                bool flag = it->second->m_timerCallback();
                m_events.erase(m_events.begin());
                if (flag)
                {
                    it->second->SetNextTimeout(timePoint);
                    m_events.emplace(std::pair<uint64_t, TimerId>(it->second->GetNextTimeout(), id), it->second);
                }
                else
                {
                    m_timers.erase(it);
                }
            }
            else
            {
                m_events.erase(m_events.begin());
            }
        }
    }

private:
    static int64_t GetNowTime()
    {
        auto timePoint = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(timePoint.time_since_epoch()).count();
    }

private:
    TimerId m_lastTimerId;
    std::unordered_map<TimerId, TimerPtr> m_timers;
    // std::map<uint64_t, TimerId> m_events;
    std::map<std::pair<uint64_t, TimerId>, TimerPtr> m_events; // 定时器的触发时间可能相同，所以key值不能只有时间
};

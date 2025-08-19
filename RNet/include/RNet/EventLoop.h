#pragma once

#include <memory>
#include <atomic>
#include <mutex>
#include "Poller.h"
#include "Timer.h"

class EventLoop
{
private:
    EventLoop(int id)
        : m_id(id), m_isShutdown(true) {}

public:
    static EventLoopSPtr Create(int id = 0)
    {
        auto ins = std::shared_ptr<EventLoop>(new EventLoop(id));
        ins->m_poller = Poller::Create();
        ins->m_poller->SetEventLoop(ins);
        return ins;
    }
    ~EventLoop() { Quit(); }
    int Loop()
    {
        if (!m_isShutdown)
            return -1;
        m_isShutdown = false;
        while (!m_isShutdown)
        {
            m_timerQue.HandleTimerEvent();
            HandleEvent(0);
        }
        return 0;
    }
    int Quit()
    {
        std::lock_guard<std::mutex> lck(m_mtx);
        if (!m_isShutdown)
        {
            m_isShutdown = true;
            m_timerQue.Close();
            m_poller->Close();
        }
        return 0;
    }
    TimerId AddTimer(const TimerEvent &event, uint64_t msec)
    {
        return m_timerQue.AddTimer(event, msec);
    }
    void RemoveTimer(TimerId id)
    {
        m_timerQue.RemoveTimer(id);
    }

    int UpdateChannel(const ChannelSPtr &channel)
    {
        std::lock_guard<std::mutex> lck(m_mtx);
        return m_poller->UpdateChannel(channel);
    }
    int RemoveChannel(const ChannelSPtr &channel)
    {
        std::lock_guard<std::mutex> lck(m_mtx);
        return m_poller->RemoveChannel(channel);
    }
    bool HandleEvent(uint64_t timeout)
    {
        std::vector<ChannelSPtr> activeChannels; // 获取活跃的通道
        int n = m_poller->Poll(timeout, activeChannels);
        for (auto &ch : activeChannels)
        {
            ch->HandleEvents(); // 执行
        }
        return n >= 0;
    }

    inline int EventLoopId() const { return m_id; }

private:
    int m_id;
    std::atomic_bool m_isShutdown;
    std::mutex m_mtx;
    PollerUPtr m_poller;
    TimerQueue m_timerQue;
};
using EventLoopWPtr = std::weak_ptr<EventLoop>;
using EventLoopSPtr = std::shared_ptr<EventLoop>;
#pragma once

#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <functional>
#include <condition_variable>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <bits/syscall.h>
#include "EventLoop.h"
#include "Channel.h"

class EventLoopThread
{
public:
    EventLoopThread() : m_eventLoop(nullptr) {}
    ~EventLoopThread() { Close(); }
    int Start()
    {
        m_thread = std::thread([this]
                               { ThreadFunc(); });

        std::unique_lock<std::mutex> lck(m_mtx);
        m_cv.wait(lck, [this]
                  { return m_eventLoop != nullptr; });

        return 0;
    }
    int Close()
    {
        m_eventLoop->Quit();
        if (m_thread.joinable())
            m_thread.join();
        return 0;
    }
    inline EventLoopSPtr GetEventLoop() const { return m_eventLoop; }

private:
    int ThreadFunc()
    {
        {
            std::unique_lock<std::mutex> lck(m_mtx);
            m_eventLoop = EventLoop::Create(syscall(SYS_gettid));
            m_cv.notify_one();
        }
        m_eventLoop->Loop();
        return 0;
    }

private:
    EventLoopSPtr m_eventLoop;
    std::thread m_thread;
    std::condition_variable m_cv;
    std::mutex m_mtx;
};
using EventLoopThreadUPtr = std::unique_ptr<EventLoopThread>;

class EventLoopThreadPool
{
private:
    EventLoopThreadPool(int size) : m_isShutdown(true), m_threads(size), m_size(size), m_next(0)
    {
    }

public:
    static std::shared_ptr<EventLoopThreadPool> Create(int size = 1)
    {
        auto ins = std::shared_ptr<EventLoopThreadPool>(new EventLoopThreadPool(size));
        for (auto &t : ins->m_threads)
        {
            t = std::make_unique<EventLoopThread>();
        }
        ins->Start();
        return ins;
    }
    ~EventLoopThreadPool() { Close(); }
    int Start()
    {
        {
            std::lock_guard<std::mutex> lck(m_mtx);
            if (!m_isShutdown)
                return -1;
            m_isShutdown = false;
        }
        for (auto &t : m_threads)
        {
            t->Start();
        }
        return 0;
    }
    int Close()
    {
        {
            std::lock_guard<std::mutex> lck(m_mtx);
            if (m_isShutdown)
                return -1;
            m_isShutdown = true;
        }
        for (auto &t : m_threads)
        {
            t->Close();
        }
        return 0;
    }
    EventLoopSPtr GetEventLoop()
    {
        std::lock_guard<std::mutex> lck(m_mtx);
        if (m_isShutdown)
            return nullptr;

        auto loop = m_threads[m_next]->GetEventLoop();
        m_next = (m_next + 1) % m_size;
        return loop;
    }
    // void RemoveChannel(const ChannelSPtr& channel) {
    //     auto loop = channel->GetEventLoop();
    //     if (loop) {
    //         loop->RemoveChannel(channel);
    //     } else {
    //         // 如果事件循环已经被销毁，可能需要处理这种情况
    //         // 例如记录日志或抛出异常
    //         // 这里可以选择忽略或记录错误
    //         // std::cerr << "EventLoop has been destroyed, cannot remove channel." << std::endl;
    //     }
    // }
private:
    std::atomic_bool m_isShutdown;
    std::mutex m_mtx;
    std::vector<EventLoopThreadUPtr> m_threads;
    int m_size;
    int m_next;
};
using EventLoopThreadPoolSPtr = std::shared_ptr<EventLoopThreadPool>;

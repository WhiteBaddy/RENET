#pragma once

#include <functional>
#include <memory>
#include "TcpSocket.h"
#include "Types.h"
#include <Epoll/EventType.h>

using EventCallback = std::function<void()>;

class EventLoop;
using EventLoopWPtr = std::weak_ptr<EventLoop>;
using EventLoopSPtr = std::shared_ptr<EventLoop>;
class Channel
{
private:
    Channel(int fd, const EventLoopSPtr &loop)
        : m_fd(fd), m_eventLoop(loop) {}

public:
    static std::shared_ptr<Channel> Create(int fd, const EventLoopSPtr &loop)
    {
        return std::shared_ptr<Channel>(new Channel(fd, loop));
    }
    ~Channel() = default;

    inline int GetFd() const { return m_fd; }
    inline EventLoopSPtr GetEventLoop() const { return m_eventLoop.lock(); }
    inline EventType GetEvents() const { return m_events; }
    inline EventType GetReEvents() const { return m_reEvents; }
    inline void SetEvents(EventType events) { m_events = events; }
    inline void SetReEvents(EventType events) { m_reEvents = events; }

    inline bool IsReadable() const { return m_events.IsIn(); }
    inline bool IsWriteable() const { return m_events.IsOut(); }
    inline bool IsNoneEvent() const { return m_events.IsNone(); }

    inline void EnableRead()
    {
        m_events += EventTypes::In;
    }
    inline void DisableRead()
    {
        m_events -= EventTypes::In;
    }
    inline void EnableWrite()
    {
        m_events += EventTypes::Out;
    }
    inline void DisableWrite()
    {
        m_events -= EventTypes::Out;
    }

    inline void SetReadCallback(const EventCallback &cb) { m_read = cb; }
    inline void SetWriteCallback(const EventCallback &cb) { m_write = cb; }
    inline void SetCloseCallback(const EventCallback &cb) { m_close = cb; }
    inline void SetErrorCallback(const EventCallback &cb) { m_error = cb; }

    void HandleEvents()
    {
        auto events = m_reEvents;
        m_reEvents = EventTypes::None;
        if (events.IsIn() || events.IsPri())
        {
            m_read();
        }
        if (events.IsOut())
        {
            m_write();
        }
        if (events.IsHup())
        {
            m_close();
            return;
        }
        if (events.IsErr())
        {
            m_error();
        }
    }

private:
    int m_fd;                  // 文件描述符，并不持有，只是相关联
    EventType m_events;        // 关心的事件
    EventType m_reEvents;      // 事件收集器获取到的需要处理的事件
    EventLoopWPtr m_eventLoop; // 所属的事件循环

    EventCallback m_read;
    EventCallback m_write;
    EventCallback m_close;
    EventCallback m_error;
};
using ChannelSPtr = std::shared_ptr<Channel>;

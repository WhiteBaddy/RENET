#pragma once

#include <memory>
#include <unordered_map>
#include <map>
#include <functional>
#include <vector>
#include "Channel.h"
#include <Epoll/Epoll.h>
#include <String/String.h>

constexpr auto POLLERTYPE = "EpollPoller";

class EventLoop;
using EventLoopWPtr = std::weak_ptr<EventLoop>;
using EventLoopSPtr = std::shared_ptr<EventLoop>;
class Poller
{
public:
    // Poller注册工厂函数
    using Creator = std::function<std::unique_ptr<Poller>()>;
    static void Register(const String &name, Creator &&creator)
    {
        GetRegistry()[name] = std::move(creator);
    }
    static std::unique_ptr<Poller> Create(const String &name = POLLERTYPE)
    {
        auto it = GetRegistry().find(name);
        if (it != GetRegistry().end())
        {
            return (it->second)();
        }
        return nullptr;
    }
    static std::map<String, Creator> &GetRegistry()
    {
        static std::map<String, Creator> registry;
        return registry;
    }

public:
    Poller() = default;
    virtual ~Poller() { Close(); }

    virtual int Close()
    {
        m_channels.clear();
        return 0;
    }

    virtual int UpdateChannel(const ChannelSPtr &) = 0;
    virtual int RemoveChannel(const ChannelSPtr &) = 0;
    // 返回活跃的channel
    virtual int Poll(uint64_t, std::vector<ChannelSPtr> &) = 0;

private:
    friend class EventLoop;
    inline void SetEventLoop(const EventLoopSPtr &loop) { m_eventLoop = loop; }

protected:
    std::unordered_map<int, ChannelSPtr> m_channels;
    EventLoopWPtr m_eventLoop;
};
using PollerUPtr = std::unique_ptr<Poller>;

class EpollPoller : public Poller
{
    // static bool registered;
    inline static bool registered = []
    {
        Poller::Register("EpollPoller", []
                         { return std::make_unique<EpollPoller>(); });
        return true;
    }();

public:
    EpollPoller() : Poller() { m_epoll.Create(); }
    ~EpollPoller() override { Close(); }
    int Close() override
    {
        Poller::Close();
        m_epoll.Close();
        return 0;
    }

    int UpdateChannel(const ChannelSPtr &channel) override
    {
        int fd = channel->GetFd();
        int ret = 0;
        if (m_channels.find(fd) != m_channels.end())
        {
            if (channel->IsNoneEvent())
            {
                ret = m_epoll.Del(fd);
                m_channels.erase(fd);
            }
            else
            {
                ret = m_epoll.Modify(fd, channel->GetEvents(), channel);
            }
        }
        else
        {
            if (!channel->IsNoneEvent())
            {
                m_channels.emplace(fd, channel);
                ret = m_epoll.Add(fd, channel, channel->GetEvents());
            }
        }
        return ret;
    }
    int RemoveChannel(const ChannelSPtr &channel) override
    {
        int fd = channel->GetFd();
        m_channels.erase(fd);
        return m_epoll.Del(fd);
    }

    int Poll(uint64_t timeoutMs, std::vector<ChannelSPtr> &activeChannels) override
    {
        EpollEvents events(512);
        int numEvents = m_epoll.WaitEvents(events, timeoutMs);
        for (auto &ev : events)
        {
            if (auto channel = ev.SPtrData<Channel>(); channel != nullptr)
            {
                channel->SetReEvents(static_cast<EventType>(ev.Events()));
                activeChannels.push_back(channel);
            }
        }
        return numEvents;
    }

private:
    CEpoll m_epoll;
};

// bool EpollPoller::registered = []
// {
//     Poller::Register("EpollPoller", []
//                      { return std::make_unique<EpollPoller>(); });
//     return true;
// }();
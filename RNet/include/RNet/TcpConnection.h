#pragma once

#include <memory>
#include <vector>
#include "Channel.h"
#include "EventLoop.h"
#include <Buffer/Buffer.h>
#include <iostream>
#include <DataPayload/DataPayload.h>

class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
    friend class TcpServer;

public:
    using SPtr = std::shared_ptr<TcpConnection>;
    using DisConnectCallback = std::function<void(const TcpConnection::SPtr &)>;
    using ClosedCallback = std::function<void(const TcpConnection::SPtr &)>;
    using ReadCallback = std::function<bool(const TcpConnection::SPtr &, std::unique_ptr<ReadBuffer> &)>;

protected:
    TcpConnection() : m_isShutdown(false) {}
    virtual void Init(SOCKET sockfd, const EventLoopSPtr &loop)
    {
        SocketUtil::SetNonBlock(sockfd);
        SocketUtil::KeepAlive(sockfd);
        SocketUtil::SendBufferSize(sockfd, 1024 * 1024); // 设置发送缓冲区大小

        m_readBuffer = std::make_unique<ReadBuffer>();
        m_writeBuffer = std::make_unique<WriteBuffer>();

        m_channel = Channel::Create(sockfd, loop);
        m_channel->SetReadCallback([this]
                                   { HandleReadEvent(); });
        m_channel->SetWriteCallback([this]
                                    { HandleWriteEvent(); });
        m_channel->SetCloseCallback([this]
                                    { HandleCloseEvent(); });
        m_channel->SetErrorCallback([this]
                                    { HandleErrorEvent(); });
        m_channel->EnableRead(); // 启用读事件

        loop->UpdateChannel(m_channel);
    }

public:
    static SPtr Create(SOCKET sockfd, const EventLoopSPtr &loop)
    {
        auto ins = std::shared_ptr<TcpConnection>(new TcpConnection());
        ins->Init(sockfd, loop);
        // 设置一个测试任务，定时发送数据
        // 这里可以根据实际需求修改为其他任务
        // 例如发送心跳包、处理业务逻辑等
        // 这里的测试任务是每隔1秒发送一句话
        // 通过捕获一个弱指针，可以在对象销毁的时候把任务一并清理掉
        static int count = 0;
        auto task = [weak_ins = std::weak_ptr<TcpConnection>(ins), idx = ++count, cnt = 0]()
        {
            auto conn = weak_ins.lock();
            if (conn)
            {
                String buffer = "client " + std::to_string(cnt) + " this is server. " + std::to_string(idx) + " " + std::to_string(conn.use_count()) + "\n";
                conn->Send(buffer, buffer.size());
                std::cout << "Send: " << buffer << std::endl;
                return true;
            }
            return false;
        };
        loop->AddTimer(task, 1000); // 测试用例
        std::cout << "conn.use_count : " << ins.use_count() << std::endl;
        return ins;
    }

    virtual ~TcpConnection() { Close(); }
    int Close()
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        if (m_isShutdown)
            return -1;
        m_isShutdown = true;
        m_channel->GetEventLoop()->RemoveChannel(m_channel);
        if (m_closedCallback)
        {
            m_closedCallback(shared_from_this());
        }
        if (m_disConnectCallback)
        {
            m_disConnectCallback(shared_from_this());
        }
        return 0;
    }
    void Send(const DataPayload::SPtr &data)
    {
        if (!m_isShutdown)
        {
            {
                std::lock_guard<std::mutex> lock(m_mtx);
                m_writeBuffer->Append(data);
            }
        }
        HandleWriteEvent(); // !!!这句很重要! 如果不调用，会导致写事件无法触发
    }
    void Send(char *buf, uint32_t size)
    {
        if (buf == nullptr || size == 0)
            return;
        auto payload = DataPayload::Create(buf, size);
        Send(payload);
    }

    inline EventLoopSPtr GetEventLoop() const { return m_channel->GetEventLoop(); }
    inline SOCKET GetSocket() const { return m_channel->GetFd(); }

    inline bool IsShutdown() const { return m_isShutdown; }

    inline void SetDisConnectCallback(const DisConnectCallback &cb) { m_disConnectCallback = cb; }
    inline void SetCloseCallback(const ClosedCallback &cb) { m_closedCallback = cb; }
    inline void SetReadCallback(const ReadCallback &cb) { m_readCallback = cb; }

private:
    void HandleReadEvent()
    {
        struct clear
        {
            std::function<void()> func;
            ~clear()
            {
                if (func)
                    func();
            }
        } close;
        if (auto lck = std::unique_lock<std::mutex>(m_mtx, std::try_to_lock);
            !m_isShutdown && lck.owns_lock()) // 尝试获取锁
        {
            int bytesRead = m_readBuffer->Read(m_channel->GetFd());
            if (bytesRead <= 0)
            {
                close.func = [this]
                { Close(); };
                return;
            }
        }

        if (m_readCallback)
        {
            bool ret = m_readCallback(shared_from_this(), m_readBuffer);
            if (!ret)
            {
                Close(); // 如果回调返回 false，关闭连接
            }
        }
    }
    void HandleWriteEvent()
    {
        struct clear
        {
            std::function<void()> func;
            ~clear()
            {
                if (func)
                    func();
            }
        } close;
        if (auto lck = std::unique_lock<std::mutex>(m_mtx, std::try_to_lock);
            !m_isShutdown && lck.owns_lock())
        {
            if (m_writeBuffer->Send(m_channel->GetFd()) < 0)
            {
                close.func = [this]
                { Close(); };
                return;
            }
            if (m_writeBuffer->IsEmpty())
            {
                if (m_channel->IsWriteable()) // 如果写缓冲区已空，禁用写事件，否则可能会导致写事件一直触发
                {
                    m_channel->DisableWrite();
                    m_channel->GetEventLoop()->UpdateChannel(m_channel); // 更新通道状态
                }
            }
            else if (!m_channel->IsWriteable()) // 如果当前是禁止写事件状态，需要设置为可写
            {
                m_channel->EnableWrite();                            // 如果写缓冲区不为空，启用写事件
                m_channel->GetEventLoop()->UpdateChannel(m_channel); // 更新通道状态
            }
        }
    }
    void HandleErrorEvent()
    {
        Close();
    }
    void HandleCloseEvent()
    {
        Close();
    }
    void DisConnect()
    {
        Close();
    }

protected:
    std::atomic_bool m_isShutdown;
    std::mutex m_mtx;
    ChannelSPtr m_channel;
    std::unique_ptr<ReadBuffer> m_readBuffer;
    std::unique_ptr<WriteBuffer> m_writeBuffer;

    DisConnectCallback m_disConnectCallback; // 断开连接回调
    ClosedCallback m_closedCallback;         // 连接关闭回调
    ReadCallback m_readCallback;             // 数据读取回调
};

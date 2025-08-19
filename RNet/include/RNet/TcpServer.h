#pragma once

#include <memory>
#include <unordered_map>
#include "Acceptor.h"
#include "TaskScheduler.h"
#include "TcpConnection.h"
#include "Types.h"
#include <iostream>

constexpr auto SchedulerThreadSize = 4;

class TcpServer
{
public:
    static std::shared_ptr<TcpServer> Create()
    {
        auto ins = std::shared_ptr<TcpServer>(new TcpServer());
        ins->Init();
        return ins;
    }
    TcpServer() = default;
    virtual ~TcpServer() { Close(); }
    void Init()
    {
        m_loopPool = EventLoopThreadPool::Create(SchedulerThreadSize);
        m_acceptor = Acceptor::Create(m_loopPool);
        m_acceptor->SetNewConnectionCallback(
            [this](SOCKET sockfd)
            {
                HandleNewConnection(sockfd);
            });
    }

    void HandleNewConnection(SOCKET sockfd)
    {
        auto conn = OnConnect(sockfd);
        if (!conn)
            return;
        AddConnection(conn);
        SetupCallbacks(conn);
    }

    void SetupCallbacks(const TcpConnection::SPtr &conn)
    {
        conn->SetDisConnectCallback([this](const TcpConnection::SPtr &conn)
                                    { RemoveConnection(conn->m_channel->GetFd()); });
        // 应用于rtmp服务器时，会先进行rtmp的赋值，后进行这里的赋值，会覆盖掉正确的函数，所以注释掉这里
        // conn->SetReadCallback(
        //     [](const TcpConnection::SPtr &conn, std::unique_ptr<ReadBuffer> &buf)
        //     {
        //         if (buf->ReadableBytes() > 0)
        //         {
        //             std::cout << "Received data: "
        //                       << std::string(buf->Peek(), buf->ReadableBytes())
        //                       << std::endl;
        //         }
        //         return true;
        //     });
        conn->SetCloseCallback(
            [](const TcpConnection::SPtr &conn)
            {
                std::cout << "Client disconnected: "
                          << conn->m_channel->GetFd()
                          << std::endl;
                if (SOCKET sockfd = conn->m_channel->GetFd(); sockfd > 0)
                {
                    close(sockfd);
                }
            });
    }

    int Start(const String &ip, uint16_t port)
    {
        Close(); // 确保之前的服务器已关闭
        int ret = m_acceptor->Listen(ip, port);
        if (ret < 0)
            return -1; // 监听失败
        m_ip = ip;
        m_port = port;
        m_isShutdown = false;
        return 0;
    }
    void Close()
    {
        if (m_isShutdown)
        {
            return; // 服务器已关闭
        }
        m_isShutdown = true;
        std::lock_guard<std::mutex> lock(m_mtx);
        m_acceptor->Close();   // 关闭接受连接的通道
        m_connections.clear(); // 清空客户端连接
    }
    inline String GetIp() const { return m_ip; }
    inline uint32_t GetPort() const { return m_port; }
    virtual TcpConnection::SPtr OnConnect(SOCKET sockfd)
    {
        if (m_isShutdown)
        {
            return nullptr; // 服务器已关闭
        }
        auto eventLoop = m_loopPool->GetEventLoop();
        auto conn = TcpConnection::Create(sockfd, eventLoop);
        // auto conn = std::make_shared<TcpConnection>(sockfd, eventLoop);
        // client->SetDisConnectCallback([this](const TcpConnection::Ptr &conn)
        //                               { this->RemoveClient(conn->Socket()); });
        return conn;
    }
    void AddConnection(const TcpConnection::SPtr &conn)
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        // m_clients[client->Socket()] = client;
        m_connections.emplace(conn->m_channel->GetFd(), conn); // 使用 emplace 直接构造
    }
    void RemoveConnection(SOCKET sockfd)
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        auto it = m_connections.find(sockfd);
        if (it == m_connections.end())
        {
            std::cerr << "Error: Client not found for socket " << sockfd << std::endl;
            return; // 客户端不存在
        }
        else
        {
            // 看一下当前共享指针引用数量
            std::cout << it->second.use_count() << " references to TcpConnection with socket " << sockfd << std::endl;
        }
        m_connections.erase(sockfd);
    }

protected:
    std::atomic_bool m_isShutdown;
    String m_ip;
    uint32_t m_port;
    EventLoopThreadPoolSPtr m_loopPool;
    AcceptorUPtr m_acceptor;
    std::mutex m_mtx;
    std::unordered_map<SOCKET, TcpConnection::SPtr> m_connections;
};

#pragma once
#include <memory>
#include "TaskScheduler.h"
#include "TcpConnection.h"
#include "EventLoop.h"

using NewConnectionCallback = std::function<void(SOCKET)>;

class Acceptor
{
private:
    Acceptor() = default;

public:
    static std::unique_ptr<Acceptor> Create(const EventLoopThreadPoolSPtr &loopPool)
    {
        auto ins = std::unique_ptr<Acceptor>(new Acceptor());
        ins->m_eventLoopThreadPool = loopPool;
        ins->m_socket = std::make_unique<TcpSocket>();
        return ins;
    }
    ~Acceptor() { Close(); }
    void SetNewConnectionCallback(const NewConnectionCallback &cb) { m_newConnectionCallback = cb; }
    int Listen(const String &ip, uint16_t port)
    {
        if (m_socket->GetSocket() > 0)
            m_socket->Close(); // 如果之前有套接字，先关闭
        SOCKET sockfd = m_socket->Create();
        SocketUtil::SetNonBlock(sockfd);
        SocketUtil::ReuseAddr(sockfd);
        SocketUtil::ReusePort(sockfd);
        auto eventLoop = m_eventLoopThreadPool->GetEventLoop();
        m_acceptChannel = Channel::Create(sockfd, eventLoop);
        if (m_socket->Bind(ip, port) < 0)
        {
            return -1; // 绑定失败
        }
        if (m_socket->Listen(1024) < 0)
        {
            return -2; // 监听失败
        }
        m_acceptChannel->SetReadCallback([this]()
                                         { OnAccept(); });
        m_acceptChannel->EnableRead();             // 启用读事件
        eventLoop->UpdateChannel(m_acceptChannel); // 将通道添加到事件循环中
        return 0;                                  // 成功
    }
    int Close()
    {
        if (m_socket->GetSocket() > 0)
        {
            m_acceptChannel->GetEventLoop()->RemoveChannel(m_acceptChannel); // 从事件循环中移除通道
            m_socket->Close();                                               // 关闭套接字
        }
        return 0;
    }

private:
    void OnAccept()
    {
        SOCKET fd = m_socket->Accept();
        if (fd < 0)
            return; // 接受连接失败
        if (m_newConnectionCallback)
        {
            m_newConnectionCallback(fd); // 调用新连接回调, 处理新连接
        }
    }

private:
    NewConnectionCallback m_newConnectionCallback;
    EventLoopThreadPoolSPtr m_eventLoopThreadPool;
    ChannelSPtr m_acceptChannel;
    TcpSocketUPtr m_socket;
};
using AcceptorUPtr = std::unique_ptr<Acceptor>;

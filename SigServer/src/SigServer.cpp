#include "SigServer/SigServer.h"
#include "SigServer/SigConnection.h"

SigServer::SPtr SigServer::Create()
{
    auto ins = std::shared_ptr<SigServer>(new SigServer());
    ins->Init();
    return ins;
}

SigServer::SigServer() : TcpServer() {}

void SigServer::Init()
{
    TcpServer::Init();
}

TcpConnection::SPtr SigServer::OnConnect(int sockfd)
{
    if (m_isShutdown)
    {
        return nullptr; // 服务器已关闭
    }
    auto eventLoop = m_loopPool->GetEventLoop();
    auto conn = SigConnection::Create(sockfd, eventLoop);
    return std::dynamic_pointer_cast<TcpConnection>(conn);
}
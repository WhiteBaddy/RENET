#pragma once

#include <RNet/TcpServer.h>

class SigServer : public TcpServer
{
public:
    using SPtr = std::shared_ptr<SigServer>;

public:
    static SPtr Create();
    ~SigServer() override = default;

private:
    SigServer();
    void Init() override;
    TcpConnection::SPtr OnConnect(int sockfd) override;
};
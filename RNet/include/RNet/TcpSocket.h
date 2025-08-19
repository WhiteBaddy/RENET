#pragma once

#include <memory>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <String/String.h>
#include "Types.h"

constexpr auto INVALID_SOCK = ~0;

class SocketUtil
{
public:
    static void SetNonBlock(SOCKET sock)
    {
        int flags = fcntl(sock, F_GETFL, 0);
        fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    }
    static void SetBlock(SOCKET sock)
    {
        int flags = fcntl(sock, F_GETFL, 0);
        fcntl(sock, F_SETFL, flags & ~O_NONBLOCK);
    }
    static void ReuseAddr(SOCKET sock)
    {
        int op = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &op, sizeof(op));
    }
    static void ReusePort(SOCKET sock)
    {
        int op = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &op, sizeof(op));
    }
    static void KeepAlive(SOCKET sock)
    {
        int op = 1;
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &op, sizeof(op));
    }
    static void SendBufferSize(SOCKET sock, int size)
    {
        setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));
    }
    static void RecvBufferSize(SOCKET sock, int size)
    {
        setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
    }
};

class TcpSocket
{
public:
    TcpSocket() : m_sockfd(INVALID_SOCK) {}
    ~TcpSocket() { Close(); }
    SOCKET Create()
    {
        m_sockfd = socket(PF_INET, SOCK_STREAM, 0);
        return m_sockfd;
    }
    int Bind(const String &ip, uint16_t port)
    {
        if (!IsValid())
            return -1;
        sockaddr_in addr{0};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(ip);
        if (-1 == bind(m_sockfd, (sockaddr *)&addr, sizeof(addr)))
            return -2;
        return 0;
    }
    int Listen(int size)
    {
        if (!IsValid())
            return -1;
        if (-1 == listen(m_sockfd, size))
            return -2;
        return 0;
    }
    SOCKET Accept()
    {
        if (!IsValid())
            return -1;
        sockaddr_in addr = {0};
        socklen_t len = sizeof(addr);
        return accept(m_sockfd, (sockaddr *)&addr, &len);
    }
    int Close()
    {
        if (!IsValid())
            return -1;
        SOCKET tmp = m_sockfd;
        m_sockfd = INVALID_SOCK;
        close(tmp);
        return 0;
    }
    inline bool IsValid() const { return m_sockfd != INVALID_SOCK; }
    inline operator int() const { return m_sockfd; }
    inline SOCKET GetSocket() const { return m_sockfd; }

private:
    SOCKET m_sockfd;
};
using TcpSocketSPtr = std::shared_ptr<TcpSocket>;
using TcpSocketUPtr = std::unique_ptr<TcpSocket>;

#pragma once
#include <list>
#include <array>
#include <queue>
#include <vector>
#include <memory>
#include <string>
#include <memory.h>
#include <sys/socket.h>

#include <cerrno>
#include <Tools/UintCodec.h>
#include <DataPayload/DataPayload.h>

class WriteBuffer
{
public:
    WriteBuffer(int maxLen = MAXLEN) : m_maxLen(maxLen) {}
    ~WriteBuffer() = default;
    bool Append(const DataPayload::SPtr &buf, uint64_t index = 0)
    {
        if (buf != nullptr && buf->size() <= index || IsFull())
            return false;
        m_buffers.emplace(Packet{.data = buf, .index = index});
        return true;
    }
    int Send(int sockfd)
    {
        int sendLen = 0;
        for (bool ctu = true; ctu;)
        {
            if (m_buffers.empty())
                return 0;
            ctu = false;
            Packet &pkt = m_buffers.front();
            ssize_t ret = send(sockfd, pkt.data->data() + pkt.index, pkt.data->size() - pkt.index, 0);
            if (ret > 0)
            {
                if ((pkt.index += ret) >= pkt.data->size())
                {
                    m_buffers.pop();
                    sendLen += ret;
                    ctu = true;
                }
            }
            else if (ret < 0)
            {
                if (errno == EINTR || errno == EAGAIN)
                {
                    ret = 0;
                }
            }
        }
        return sendLen;
    }

    inline bool IsEmpty() const { return m_buffers.empty(); }
    inline bool IsFull() const { return m_buffers.size() >= m_maxLen; }
    inline int Size() const { return m_buffers.size(); }

private:
    struct Packet
    {
        DataPayload::SPtr data;
        uint64_t index;
    };
    std::queue<Packet> m_buffers;
    int m_maxLen;
    static constexpr int MAXLEN{10 * 1000};
};

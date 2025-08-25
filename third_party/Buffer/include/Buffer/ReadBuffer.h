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
#include <Tools/NumberCodec.h>
#include <Tools/Debug.h>
#include <DataPayload/DataPayload.h>
#include <fmt/core.h>

class ReadBuffer
{
public:
    ReadBuffer(uint32_t initSize = 2048) : m_buffer(initSize) {}
    virtual ~ReadBuffer() = default;
    inline uint32_t ReadableBytes() const { return m_writerIndex - m_readerIndex; }
    inline uint32_t WritableBytes() const { return m_buffer.size() - m_writerIndex; }
    uint8_t *Peek() { return Begin() + m_readerIndex; }
    const uint8_t *Peek() const { return Begin() + m_readerIndex; }

    void RetrieveAll()
    {
        m_writerIndex = 0;
        m_readerIndex = 0;
    }
    void Retrieve(size_t len)
    {
        if (len < ReadableBytes())
        {
            m_readerIndex += len;
            if (m_readerIndex == m_writerIndex)
            {
                RetrieveAll();
            }
        }
        else
        {
            RetrieveAll();
        }
    }
    int Read(int fd)
    {
        // 简单扩容
        uint32_t size = WritableBytes();
        if (size < MAX_BYTES_PER_READ)
        {
            // 如果当前可写空间不足，就先进行一次扩容
            uint32_t bufferReadSize = m_buffer.size();
            if (bufferReadSize > MAX_BUFFER_SIZE)
            {
                return 0;
            }
            m_buffer.resize(bufferReadSize + MAX_BYTES_PER_READ);
        }
        // 读数据 从sock接收缓存 -> buffer
        int bytes_read = recv(fd, BeginWrite(), MAX_BYTES_PER_READ, 0);
        if (bytes_read > 0)
        {
            // Debug::ShowHex(BeginWrite(), bytes_read);
            // fmt::print("ReadBuffer::Read, bytes_read: {}\n", bytes_read);
            m_writerIndex += bytes_read;
        }
        return bytes_read;
    }
    uint32_t ReadAll(std::string &data)
    {
        uint32_t size = ReadableBytes();
        if (size > 0)
        {
            data.assign((char *)Peek(), size);
            m_writerIndex = 0;
            m_readerIndex = 0;
        }
        return size;
    }
    uint32_t Size() const { return m_buffer.size(); }

private:
    uint8_t *Begin() { return &*m_buffer.begin(); }
    const uint8_t *Begin() const { return &*m_buffer.begin(); }
    uint8_t *BeginWrite() { return Begin() + m_writerIndex; }
    const uint8_t *BeginWrite() const { return Begin() + m_writerIndex; }

private:
    std::vector<uint8_t> m_buffer;
    size_t m_readerIndex = 0;
    size_t m_writerIndex = 0;
    static constexpr uint32_t MAX_BYTES_PER_READ = 4096;
    static constexpr uint32_t MAX_BUFFER_SIZE = 1024 * 100000; // 加大缓冲区
};

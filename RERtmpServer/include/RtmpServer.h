#pragma once

#include <RNet/TcpServer.h>
#include <memory>
#include "Session.h"
#include "Rtmp.h"

class Rtmp
{
public:
    virtual ~Rtmp();
    void SetChunkSize(uint32_t size);
    void SetPeerBandwidth(uint32_t size);
    uint32_t GetChunkSize() const;
    uint32_t GetAcknowledgementSize() const;
    uint32_t GetPeerBandwidth() const;
    std::string GetApp() const;
    std::string GetStreamPath() const;
    std::string GetStreamName() const;

    virtual int ParseRtmpUrl(const std::string &url);

private:
    uint16_t port_ = 1935;
    std::string ip_;
    std::string app_;
    std::string stream_name_;
    std::string stream_path_;
    uint32_t peer_bandwidth_ = 5 * 1000 * 1000;
    uint32_t acknowledgement_ = 5 * 1000 * 1000;
    uint32_t max_chunk_size_ = 128;
};

class RtmpServer
    : public TcpServer,
      public Rtmp,
      public std::enable_shared_from_this<RtmpServer>
{
public:
    friend class RtmpConnection;
    using SPtr = std::shared_ptr<RtmpServer>;
    using EventCallback = std::function<void(std::string type, std::string stream_path)>;

public:
    static SPtr Create();
    RtmpServer();
    ~RtmpServer();
    void SetEventCallback(const EventCallback &cb);
    void Init();

    // private:
    void AddSession(std::string stream_path);
    void RemoveSession(std::string stream_path);
    Session::SPtr GetSession(std::string stream_path);
    bool HasPublisher(std::string stream_path);
    bool HasSession(std::string stream_path);
    void NotifyEvent(std::string type, std::string stream_path);
    TcpConnection::SPtr OnConnect(int sockfd) override;

private:
    std::mutex mtx_sessons_;
    std::unordered_map<std::string, Session::SPtr> sessions_;
    std::vector<EventCallback> event_cbs_;

    // 下面变量和函数用于给推流的客户端分配流id
private:
    uint32_t stream_id_ = 0;

public:
    uint32_t ApplyStreamId() { return ++stream_id_; }
};
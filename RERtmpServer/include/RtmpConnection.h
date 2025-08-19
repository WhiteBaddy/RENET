#pragma once

#include <memory>
#include "Rtmp.h"
#include "Handshake.h"
#include "ChunkManager.h"
#include "AMF.h"
#include <RNet/TcpConnection.h>
#include "Sink.h"

class RtmpServer;
using RtmpServerSPtr = std::shared_ptr<RtmpServer>;
using RtmpServerWPtr = std::weak_ptr<RtmpServer>;

class Session;
using SessionSPtr = std::shared_ptr<Session>;
using SessionWPtr = std::weak_ptr<Session>;

class RtmpConnection
    : public RtmpSink,
      public TcpConnection
{
public:
    using SPtr = std::shared_ptr<RtmpConnection>;
    // RTMP连接状态
    enum class State : uint8_t
    {
        Handshake,    // Performing handshake
        Connecting,   // Sent/received 'connect' command
        CreateStream, // Sent/received 'createStream' command
        DeleteStream, // Sent/received 'deleteStream' command
        Playing,      // Playing a stream
        Publishing,   // Publishing a stream
        Error,        // Connection error occurred
        Closed        // Connection closed
    };

public:
    static SPtr Create(const RtmpServerSPtr &server, const EventLoopSPtr &eventloop, SOCKET sockfd);
    RtmpConnection();
    ~RtmpConnection() override;

private:
    void Init(const RtmpServerSPtr &server, const EventLoopSPtr &eventloop, SOCKET sockfd);

public:
    bool IsPlayer() const override;
    bool IsPublisher() const override;
    bool IsPlaying() const override;
    bool IsPublishing() const override;
    uint32_t GetId() const override;

private:
    bool OnRead(std::unique_ptr<ReadBuffer> &readBuf);
    void OnClose();

    bool HandleChunk(std::unique_ptr<ReadBuffer> &readBuf);

    bool HandleMessage(RtmpMessage &rtmpMsg);
    bool HandleInvoke(RtmpMessage &rtmpMsg);
    bool HandleNotify(RtmpMessage &rtmpMsg);
    bool HandleAudio(RtmpMessage &rtmpMsg);
    bool HandleVideo(RtmpMessage &rtmpMsg);

    bool HandleConnect(uint32_t transaction_id);
    // bool HandleReleaseStream(uint32_t transaction_id);
    bool HandleCreateStream(uint32_t transaction_id);
    bool HandlePublish(uint32_t transaction_id);
    bool HandlePlay(uint32_t transaction_id);
    bool HandleDeleteStream(uint32_t transaction_id, uint32_t deleted_stream_id);
    bool HandleSendResult(uint32_t transaction_id);
    bool HandleGetStreamLength(uint32_t transaction_id);

    bool SendCtrlMsg(RtmpMessage::Type tpid, const std::vector<uint8_t> &data);

    void SetPeerBandWidth();
    void SendAcknowledgement();
    void SetOutChunkSize(uint32_t size);

    bool IsKeyFrame(const DataPayload::SPtr &data);

    bool SendMsg(const RtmpMessage::SPtr &rtmpMsg);

    bool SendMetaData(const AmfObject::SPtr &metaData) override;
    bool SendMetaData(const AmfEcmaArray::SPtr &metaData) override;

    bool SendVideoData(RtmpVideoFrameType type, uint64_t timestamp, DataPayload::SPtr &payload) override;
    bool SendAudioData(RtmpAudioFrameType type, uint64_t timestamp, DataPayload::SPtr &payload) override;

private:
    State state_;                                   // 当前状态
    ServerHandshake::SPtr handshake_;               // Rtmp握手
    RtmpMessageManager::SPtr rtmp_message_manager_; // msg包管理

    RtmpServerWPtr rtmp_server_; // 连接的服务器
    SessionWPtr rtmp_session_;   // 所属会话

    uint32_t peer_width_;           // 窗口大小
    uint32_t acknowledgement_size_; // 确认阈值
    uint32_t max_chunk_size_;       // 最大块大小
    uint32_t stream_id_;            // 流id, 每个连接唯一

    AmfObject::SPtr connect_metadata_;   // 连接元数据
    AmfEcmaArray::SPtr stream_metadata_; // 推流元数据

    bool is_playing_;    // 拉流者使用，是否正在拉流
    bool is_publishing_; // 推流者使用，是否正在推流

    std::string app_;
    std::string stream_name_;
    std::string stream_path_;

    bool has_key_frame_; // 视频数据使用，是否获取到关键帧

    DataPayload::SPtr avc_sequence_header_; // 视频序列头
    DataPayload::SPtr aac_sequence_header_; // 音频序列头
};

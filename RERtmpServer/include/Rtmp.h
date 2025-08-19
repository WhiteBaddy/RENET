#pragma once
#include <stdint.h>

// 音频编解码器ID
enum class RtmpAudioCodecId : uint8_t
{
    MP3 = 2,  // MP3 audio
    AAC = 10, // AAC audio
};

// 视频编解码器ID
enum class RtmpVideoCodecId : uint8_t
{
    H263 = 2,      // H.263 video
    SCREEN = 3,    // Screen sharing video
    VP6 = 4,       // On2 VP6 video
    VP6_ALPHA = 5, // On2 VP6 with alpha
    SCREEN2 = 6,   // Screen sharing video (version 2)
    H264 = 7,      // H.264 video
    HEVC = 12,     // H.265 video (non-standard)
    AV1 = 13       // AV1 video (non-standard)
};

// 视频帧类型
enum class RtmpVideoFrameType : uint8_t
{
    KeyFrame = 1,        // Key frame
    InterFrame = 2,      // Inter frame
    DisposableFrame = 3, // Disposable inter frame
    SequenceHeader = 4   // AVC/H.264 sequence header
};

// 音频帧类型
enum class RtmpAudioFrameType : uint8_t
{
    SequenceHeader = 0, // AAC sequence header
    RawData = 1         // AAC raw data
};

// RTMP协议版本
// constexpr uint8_t RTMP_VERSION = 0x03;

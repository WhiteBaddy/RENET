#include "SigServer/Packet.h"

Packet::SPtr Packet::Create(Cmd cmd)
{
    switch (cmd)
    {
    case Cmd::Join:
        return std::make_shared<JoinPacket>();
    case Cmd::JoinReply:
        return std::make_shared<JoinReplyPacket>();
    case Cmd::CreateStream:
        return std::make_shared<CreateStreamPacket>();
    case Cmd::CreateStreamReply:
        return std::make_shared<CreateStreamReplyPacket>();
    case Cmd::ObtainStream:
        return std::make_shared<ObtainStreamPacket>();
    case Cmd::ObtainStreamReply:
        return std::make_shared<ObtainStreamReplyPacket>();
    case Cmd::PlayStream:
        return std::make_shared<PlayStreamPacket>();
    case Cmd::PlayStreamReply:
        return std::make_shared<PlayStreamReplyPacket>();
    case Cmd::DeleteStream:
        return std::make_shared<DeleteStreamPacket>();
    case Cmd::MouseButton:
        return std::make_shared<MouseButtonInfo>();
    case Cmd::MouseMove:
        return std::make_shared<MouseMoveInfo>();
    case Cmd::WheelScroll:
        return std::make_shared<WheelScrollInfo>();
    case Cmd::KeyboardInput:
        return std::make_shared<KeyboardInfo>();
    default:
        return nullptr;
    }
    return nullptr;
}

Packet::SPtr Packet::Create(const uint8_t *in, size_t len)
{
    Head head;
    if (head.Decode(in, len) <= 0 || head.len > len)
        return nullptr;
    auto pkt = Create(head.cmd);
    if (pkt != nullptr)
        pkt->Decode(in, len);
    return pkt;
}
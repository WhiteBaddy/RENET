#pragma once
#include "Packet.h"
#include <memory>
#include <unordered_set>
#include <RNet/TcpConnection.h>

class SigConnection : public TcpConnection
{
public:
    using SPtr = std::shared_ptr<SigConnection>;
    static SPtr Create(SOCKET sockfd, const EventLoopSPtr &loop);

public:
    SigConnection();
    ~SigConnection() override;

private:
    void Init(SOCKET sockfd, const EventLoopSPtr &loop) override;

public:
    bool IsAlive() const { return role_state_ < RoleState::Stopped; }
    bool IsNoJion() const { return role_state_ == RoleState::None; }
    bool IsIdle() const { return role_state_ == RoleState::Idle; }
    bool IsWorking() const { return role_state_ == RoleState::Publishing || role_state_ == RoleState::Playing; }
    bool IsDisconnected() const { return role_state_ == RoleState::Disconnected; }
    bool IsClosed() const { return role_state_ == RoleState::Closed; }

    void DisConnect();
    void AddCustomObject(const std::string &object);
    void RemoveCustomObject(const std::string &object);

    RoleState GetRoleState() const { return role_state_; }
    std::string GetCode() const { return code_; }
    std::string GetStreamAddress() const { return stream_address_; }

protected:
    bool OnRead(std::unique_ptr<ReadBuffer> &buffer);
    void HandleMessage(std::unique_ptr<ReadBuffer> &buffer);
    void Clear();

private:
    void HandleJoin(const Packet::SPtr &packet);
    void HandleObtainStream(const Packet::SPtr &packet);
    void HandleCreateStreamReply(const Packet::SPtr &packet);
    void HandleDeleteStream(const Packet::SPtr &packet);
    void HandleOtherMessage(const Packet::SPtr &packet);

private:
    void DoObtainStream(const Packet::SPtr &packet);
    void DoCreateStreamReply(const Packet::SPtr &packet);

private:
    RoleState role_state_;                    // 角色状态
    std::string code_;                        // 连接码
    std::string stream_address_;              // 流地址
    TcpConnection::SPtr tcp_connection_;      // TCP连接指针
    std::unordered_set<std::string> objects_; // 对象列表
};
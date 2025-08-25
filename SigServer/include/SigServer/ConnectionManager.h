#pragma once

// #include <RNet/TcpConnection.h>
#include <mutex>
#include <memory>
#include <unordered_map>
#include "SigConnection.h"

class ConnectionManager
{
public:
    using SPtr = std::shared_ptr<ConnectionManager>;

public:
    static ConnectionManager &GetInstance();
    ~ConnectionManager(); // 禁止外部删除实例
private:
    ConnectionManager(); // 私有构造函数，确保单例模式
public:
    void AddConnection(const std::string &code, const SigConnection::SPtr &connection);
    void RemoveConnection(const std::string &code);
    size_t GetConnectionCount();                                // 获取连接数量
    SigConnection::SPtr GetConnection(const std::string &code); // 查询连接是否存在
    bool IsConnectionExists(const std::string &code);           // 检查连接是否存在
private:
    void ClearConnections(); // 清除所有连接
    void Close();            // 关闭所有连接
private:
    std::mutex mtx_;                                                   // 互斥锁保护连接列表
    std::unordered_map<std::string, SigConnection::SPtr> connections_; // 连接列表，使用连接码作为键
};
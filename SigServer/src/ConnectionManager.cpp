#include "SigServer/ConnectionManager.h"

ConnectionManager &ConnectionManager::GetInstance()
{
    static ConnectionManager instance; // 使用静态局部变量确保单例模式
    return instance;
}

ConnectionManager::~ConnectionManager()
{
    Close(); // 在析构时清除所有连接
}
ConnectionManager::ConnectionManager() = default; // 私有构造函数，确保单例

void ConnectionManager::AddConnection(const std::string &code, const SigConnection::SPtr &connection)
{
    if (code.empty() || !connection)
        return; // 如果代码为空或连接为空，直接返回
    std::lock_guard<std::mutex> lock(mtx_);
    connections_[code] = connection;
}

void ConnectionManager::RemoveConnection(const std::string &code)
{
    if (code.empty())
        return; // 如果代码为空或连接为空，直接返回
    std::lock_guard<std::mutex> lock(mtx_);
    connections_.erase(code);
}

size_t ConnectionManager::GetConnectionCount()
{
    std::lock_guard<std::mutex> lock(mtx_);
    return connections_.size(); // 返回连接数量
}

SigConnection::SPtr ConnectionManager::GetConnection(const std::string &code)
{
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = connections_.find(code);
    if (it != connections_.end())
        return it->second;
    return nullptr; // 如果未找到，返回空指针
}

bool ConnectionManager::IsConnectionExists(const std::string &code)
{
    std::lock_guard<std::mutex> lock(mtx_);
    return connections_.find(code) != connections_.end(); // 检查连接是否存在
}

void ConnectionManager::ClearConnections()
{
    std::lock_guard<std::mutex> lock(mtx_);
    connections_.clear(); // 清除所有连接
}

void ConnectionManager::Close()
{
    ClearConnections(); // 清除所有连接
}
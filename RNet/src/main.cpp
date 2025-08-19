#include <iostream>
#include "RNet/TcpServer.h"

int main()
{
    uint32_t count = std::thread::hardware_concurrency();
    auto server = TcpServer::Create();
    bool status = server->Start("127.0.0.1", 1999);
    std::cout << "Server started: " << (status == 0 ? "Success" : "Failure") << std::endl;
    getchar(); // 等待用户输入以保持程序运行
    server->Close();
    std::cout << "Server stopped." << std::endl;
    return 0;
}
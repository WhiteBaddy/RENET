
#include <fmt/core.h>
#include <thread>
#include <SigServer/SigServer.h>

int main()
{
    fmt::print("hello sig\n");
    int cnt = std::thread::hardware_concurrency();
    fmt::print("cpu core count : {}\n", cnt);
    auto server = SigServer::Create();
    if (0 != server->Start("127.0.0.1", 6539))
    {
        fmt::print("SigServer start failed\n");
        return -1;
    }
    fmt::print("SigServer started at {}:{}\n", server->GetIp(), server->GetPort());
    getchar();

    return 0;
}
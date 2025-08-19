
#include "ChunkManager.h"
#include <fmt/core.h>
#include "RtmpServer.h"
#include <thread>

int main()
{
    int count = std::thread::hardware_concurrency();
    auto rtmp_server = RtmpServer::Create();
    rtmp_server->SetChunkSize(60 * 1000);
    rtmp_server->SetEventCallback([](std::string type, std::string stream_path)
                                  { fmt::print("[Event]{}, stream_path{}\n", type, stream_path); });
    if (rtmp_server->Start("127.0.0.1", 1935))
    {
        fmt::print("rtmp server failed\n");
    }
    else
    {
        fmt::print("rtmp server success\n");
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    return 0;
}

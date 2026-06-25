// ChatServer 服务端入口
// 创建事件循环 → 绑定地址端口 → 启动服务 → 进入事件循环
#include "server/chatserver.hpp"
#include <iostream>
#include <cstdlib>
using namespace std;

int main(int argc, char *argv[])
{
    int port = 6000;
    if (argc >= 2) port = atoi(argv[1]);

    EventLoop loop;                        // muduo 事件循环（Reactor 核心）
    InetAddress addr("127.0.0.1", port);   // 监听指定端口
    ChatServer server(&loop, addr, "ChatServer");

    server.start();     // 启动 TcpServer（开启监听）
    loop.loop();        // 进入事件循环（阻塞在此，等待客户端连接）

    return 0;
}

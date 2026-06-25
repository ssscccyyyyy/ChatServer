#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <string>
using namespace muduo;
using namespace muduo::net;
using namespace std;

// 聊天服务器的主类（网络层）
// 封装 muduo TcpServer，负责建立 TCP 连接、收发消息、将请求分发给业务层
class ChatServer
{
public:
    // 初始化聊天服务器对象（绑定事件循环、监听地址、服务名称）
    ChatServer(EventLoop *loop,
               const InetAddress &listenAddr,
               const string &nameArg);
    // 启动服务（开启 muduo 事件循环）
    void start();

private:
    // 新连接建立/断开时的回调函数
    void onConnection(const TcpConnectionPtr &conn);
    // 收到客户端消息时的回调函数：反序列化 JSON → 路由到业务处理器
    void onMessage(const TcpConnectionPtr &conn,
                   Buffer *buffer,
                   Timestamp time);

    TcpServer _server;  // muduo 网络服务对象（组合方式）
    EventLoop *_loop;   // 指向事件循环对象的指针
};
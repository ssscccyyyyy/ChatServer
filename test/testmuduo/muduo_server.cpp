// moduo网络库给用户提供了两个主要的类
// tcpSever:用于编写服务器程序的
// tcpClient:用于编写客户端程序的

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>
#include <functional>
#include<string>
using namespace muduo;
using namespace muduo::net;

/*基于muduo网络库开发服务器程序
1.组合TcpServer对象
2.创建EventLoop事件循环对象的指针
3.明确TcpServer构造函数需要什么参数，输出ChatServer的构造参数
4.在当前服务器类的构造函数当中，注册处理连接的回调函数和处理读写事件的回调函数
epoll+线程池
5.设置合适的服务端线程数量,muduo库会自己划分I/O线程和worker线程

好处:能够把网络I/O的代码和业务代码区分开
            用户的连接和断开 用户的可读写事件
*/
class ChatServer
{
public:
    ChatServer(EventLoop *loop,               // 时间循环
               const InetAddress &listenAddr, // ip+port
               const string &nameArg)         // 服务器名字
        : _server(loop, listenAddr, nameArg),
          _loop(loop)
    {
        // 给服务器注册用户连接的创建和断开回调
        _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, std::placeholders::_1));

        // 给服务器注册用户读写时间的回调
        _server.setMessageCallback(std::bind(&ChatServer::onMessage,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));

        //设置服务器端的线程数量 1个I/O线程 3个worker线程
        _server.setThreadNum(4);

    }

    //开启事件循环
    void start(){
        _server.start();
    }

private:
    // 专门处理用户的连接创建和断开 epoll listenfd accept
    void onConnection(const TcpConnectionPtr &conn)
    {
        if(conn->connected()){
            std::cout<<conn->peerAddress().toIpPort()<<"->"<<
            conn->localAddress().toIpPort()<<"state:online"<<std::endl;
        }
        else{
            std::cout<<conn->peerAddress().toIpPort()<<"->"<<conn->localAddress().toIpPort()
            <<"state:offline"<<std::endl;
            conn->shutdown();//close
            //_loop->quit();
        }
    }

    // 专门处理用户的读写事件
    void onMessage(const TcpConnectionPtr &conn,//连接
                   Buffer *buffer,//缓冲区
                   Timestamp time)//接收到数据的时间信息
    {
        string buf=buffer->retrieveAllAsString();
        std::cout<<"recv data:"<<buf<<"time"<<time.toString()<<std::endl;
        conn->send(buf);
    }

    muduo::net::TcpServer _server;
    muduo::net::EventLoop *_loop;
};

int main(){

    EventLoop loop;//epoll
    InetAddress addr("127.0.0.1",6000);
    ChatServer server(&loop,addr,"ChatServer");
    server.start();
    loop.loop();//epoll_wait以阻塞方式等待新用户连接,已连接用户的读写事件


    return 0;
}

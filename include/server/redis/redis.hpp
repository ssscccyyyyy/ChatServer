#pragma once
#include <string>
#include <functional>
#include <hiredis/hiredis.h>
using namespace std;

// Redis 工具类：封装 hiredis，实现发布/订阅功能
// 设计要点：
// 1. 发布和订阅使用两套独立 redisContext，避免订阅阻塞影响发布
// 2. 订阅消息在独立线程中阻塞读取，收到后通过回调通知上层业务
// 3. 通道统一用 int 数字标识
class Redis
{
public:
    Redis();
    ~Redis();

    // 连接 Redis 服务（初始化发布和订阅两套上下文）
    bool connect();

    // 向指定通道发布消息
    bool publish(int channel, string message);

    // 订阅指定通道
    bool subscribe(int channel);

    // 取消订阅指定通道
    bool unsubscribe(int channel);

    // 开启独立线程循环阻塞读取订阅消息
    void observer_channel_message();

    // 注册消息回调函数（业务层处理收到的消息）
    void init_notify_handler(function<void(int, string)> fn);

private:
    // hiredis 原生同步上下文：专门用于 publish 操作
    redisContext *_publish_context;

    // hiredis 原生同步上下文：专门用于 subscribe/unsubscribe 和阻塞接收
    redisContext *_subscribe_context;

    // 收到订阅消息后的回调函数（通道号, 消息内容）
    function<void(int, string)> _notify_message_handler;
};

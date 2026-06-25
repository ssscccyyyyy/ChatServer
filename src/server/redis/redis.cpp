#include "server/redis/redis.hpp"
#include <iostream>
#include <thread>

// 构造函数：初始化两个上下文为空指针
Redis::Redis()
    : _publish_context(nullptr), _subscribe_context(nullptr)
{
}

// 析构函数：释放两个 redis 连接资源
Redis::~Redis()
{
    if (_publish_context != nullptr)
    {
        redisFree(_publish_context);
    }
    if (_subscribe_context != nullptr)
    {
        redisFree(_subscribe_context);
    }
}

// 连接 Redis 服务（默认 127.0.0.1:6379）
bool Redis::connect()
{
    // 创建发布用上下文
    _publish_context = redisConnect("127.0.0.1", 6379);
    if (_publish_context == nullptr || _publish_context->err)
    {
        cerr << "Redis publish connect failed: "
             << (_publish_context ? _publish_context->errstr : "nullptr") << endl;
        return false;
    }

    // 创建订阅用上下文
    _subscribe_context = redisConnect("127.0.0.1", 6379);
    if (_subscribe_context == nullptr || _subscribe_context->err)
    {
        cerr << "Redis subscribe connect failed: "
             << (_subscribe_context ? _subscribe_context->errstr : "nullptr") << endl;
        redisFree(_publish_context);
        _publish_context = nullptr;
        return false;
    }

    return true;
}

// 向指定通道发布消息
bool Redis::publish(int channel, string message)
{
    redisReply *reply = (redisReply *)redisCommand(
        _publish_context, "PUBLISH %d %s", channel, message.c_str());
    if (reply == nullptr)
    {
        cerr << "Redis publish command failed" << endl;
        return false;
    }
    freeReplyObject(reply);
    return true;
}

// 订阅指定通道
bool Redis::subscribe(int channel)
{
    // SUBSCRIBE 命令本身不阻塞，阻塞发生在 redisGetReply 时
    redisReply *reply = (redisReply *)redisCommand(
        _subscribe_context, "SUBSCRIBE %d", channel);
    if (reply == nullptr)
    {
        cerr << "Redis subscribe command failed" << endl;
        return false;
    }
    freeReplyObject(reply);
    return true;
}

// 取消订阅指定通道
bool Redis::unsubscribe(int channel)
{
    redisReply *reply = (redisReply *)redisCommand(
        _subscribe_context, "UNSUBSCRIBE %d", channel);
    if (reply == nullptr)
    {
        cerr << "Redis unsubscribe command failed" << endl;
        return false;
    }
    freeReplyObject(reply);
    return true;
}

// 开启独立线程，循环阻塞读取订阅通道的消息
void Redis::observer_channel_message()
{
    thread([this]() {
        while (_subscribe_context != nullptr)
        {
            redisReply *reply = nullptr;
            // 阻塞等待订阅消息
            if (redisGetReply(_subscribe_context, (void **)&reply) == REDIS_OK)
            {
                // 订阅消息的 reply 是一个数组：
                // reply[0] = 消息类型（"message"/"subscribe"/"unsubscribe"）
                // reply[1] = 通道名（字符串）
                // reply[2] = 消息内容
                if (reply != nullptr && reply->type == REDIS_REPLY_ARRAY && reply->elements >= 3)
                {
                    string type = reply->element[0]->str;
                    int channel = atoi(reply->element[1]->str);
                    string message = reply->element[2]->str;

                    if (type == "message" && _notify_message_handler)
                    {
                        // 收到消息，回调业务层处理
                        _notify_message_handler(channel, message);
                    }
                }
                freeReplyObject(reply);
            }
        }
        cerr << "Redis observer thread exited" << endl;
    }).detach();
}

// 注册消息回调函数
void Redis::init_notify_handler(function<void(int, string)> fn)
{
    _notify_message_handler = fn;
}

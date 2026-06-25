
#include <unordered_map>
#include <functional>
#include <muduo/net/TcpConnection.h>
#include <mutex>
#include "json.hpp"
#include "server/model/usermodel.hpp"
#include "server/model/offlinemessagemodel.hpp"
#include "server/model/friendmodel.hpp"
#include "server/model/groupmodel.hpp"
using namespace std;
using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;

// MsgHandler：消息处理器的类型定义
// 参数：(Tcp连接, JSON数据, 时间戳)
using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp)>;

// ChatService 聊天服务器业务类（业务层）
// 单例模式：全局只有一个实例
// 职责：管理消息路由表、在线用户连接、调用 Model 层完成数据操作
class ChatService
{
public:
    // 获取单例对象
    static ChatService *instance();

    // 处理登录业务
    void login(const TcpConnectionPtr &conn, json &js, Timestamp);
    // 处理注册业务
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp);
    // 一对一聊天业务
    void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp);
    // 添加好友业务
    void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp);
    // 创建群组业务
    void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp);
    // 加入群组业务
    void joinGroup(const TcpConnectionPtr &conn, json &js, Timestamp);
    // 群聊业务
    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp);

    // 根据 msgid 获取对应的消息处理器
    MsgHandler getHandler(int msgid);
    // 处理客户端异常退出（断线、崩溃等）
    void clientCloseException(const TcpConnectionPtr &conn);

private:
    ChatService();  // 私有构造函数（单例）

    // 消息 id → 处理函数 的映射表（消息路由核心）
    unordered_map<int, MsgHandler> _msgHandlerMap;

    // 在线用户 id → Tcp连接 的映射表
    unordered_map<int, TcpConnectionPtr> _userConnMap;

    // 保护 _userConnMap 的互斥锁（多线程安全）
    mutex _connMutex;

    // 数据操作类对象
    UserModel _userModel;
    OfflineMsgModel _offlineMsgModel;
    FriendModel _friendModel;
    GroupModel _groupModel;
};
#include "server/chatservice.hpp"
#include "public.hpp"
#include <muduo/base/Logging.h>
#include <string>
#include <server/user.hpp>
#include<vector>
using namespace muduo;
using namespace std::placeholders;

// 获取单例对象的接口函数
ChatService *ChatService::instance()
{
    static ChatService service;
    return &service;
}

// 注册消息以及对应的Handler回调操作
ChatService::ChatService()
{
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});
    
    //群组业务回调
    _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({JOIN_GROUP_MSG, std::bind(&ChatService::joinGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});
}

MsgHandler ChatService::getHandler(int msgid)
{
    // 记录错误日志,msgid没有对应的事件处理回调
    auto it = _msgHandlerMap.find(msgid);
    if (it == _msgHandlerMap.end())
    {
        // 返回一个默认的处理器，空操作
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp)
        {
            LOG_ERROR << "msgid:" << msgid << " can not find handler!";
        };
    }
    else
    {
        return _msgHandlerMap[msgid];
    }
}

// 处理登录业务
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp)
{
    int id = js["id"];
    string pwd = js["password"];

    User user = _userModel.query(id);
    // 登录成功
    if (user.getId() == id && user.getPassword() == pwd)
    {

        if (user.getState() == "online")
        {
            // 该用户已经登录，不允许重复登录
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "该账号已经登录，请重新输入";
            conn->send(response.dump());
        }
        else
        {
            // 登录成功，记录用户连接信息
            lock_guard<mutex> lock(_connMutex);
            _userConnMap.insert({id, conn});

            // 登录成功,更新用户状态信息 state
            user.setState("online");
            _userModel.updateState(user);

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();

            // 查询该用户是否有离线消息
            vector<string> vec = _offlineMsgModel.query(id);
            if (!vec.empty())
            {
                response["offlinemsg"] = vec;
                // 读取该用户的离线消息后，把该用户的所有离线消息删除
                _offlineMsgModel.remove(id);
            }

            // 查询该用户的好友信息并返回
            vector<User> friends = _friendModel.query(id);
            if (!friends.empty())
            {
                json friendsJson = json::array();
                for (auto &f : friends)
                {
                    json fJson;
                    fJson["id"] = f.getId();
                    fJson["name"] = f.getName();
                    fJson["state"] = f.getState();
                    friendsJson.push_back(fJson);
                }
                response["friends"] = friendsJson;
            }

            conn->send(response.dump());
        }
    }
    // 登录失败
    else
    {
        // 该用户不存在，用户存在但是密码错误,登录失败
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "用户名或者密码错误";
        conn->send(response.dump());
    }
}

// 处理注册业务
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp)
{
    string name = js["name"];
    string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPassword(pwd);

    bool state = _userModel.insert(user);
    if (state)
    {
        // 注册成功
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();
        conn->send(response.dump());
    }
    else
    {
        // 注册失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        conn->send(response.dump());
    }
}

// 处理客户端一异常退出
void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user;

    {
        lock_guard<mutex> lock(_connMutex);

        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it)
        {
            // 从map表删除用户的连接信息
            if (it->second == conn)
            {
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }

    if (user.getId() != -1)
    {
        // 更新用户信息
        user.setState("offline");
        _userModel.updateState(user);
    }
}

// 一对一聊天业务
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp)
{
    int toid = js["to"].get<int>();

    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(toid);
        if (it != _userConnMap.end())
        {
            // toid在线，转发消息 服务器主动推送消息给toid 用户
            it->second->send(js.dump());
            return;
        }
    }

    _offlineMsgModel.insert(toid,js.dump());
}


// 添加好友业务
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    // 在 Friend 表中插入双向好友关系
    _friendModel.insert(userid, friendid);
    _friendModel.insert(friendid, userid);

    json response;
    response["msgid"] = ADD_FRIEND_MSG;
    response["errno"] = 0;
    response["errmsg"] = "添加好友成功";
    conn->send(response.dump());
}

// 创建群组业务
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp)
{
    int userid = js["id"].get<int>();
    string groupname = js["groupname"];
    string groupdesc = js["groupdesc"];

    Group group(-1, groupname, groupdesc);
    if (_groupModel.creatGroup(group))
    {
        // 创建成功，把创建者设为群主
        _groupModel.joinGroup(userid, group.getId(), "creator");

        json response;
        response["msgid"] = CREATE_GROUP_MSG;
        response["errno"] = 0;
        response["id"] = group.getId();
        conn->send(response.dump());
    }
    else
    {
        json response;
        response["msgid"] = CREATE_GROUP_MSG;
        response["errno"] = 1;
        conn->send(response.dump());
    }
}

// 加入群组业务
void ChatService::joinGroup(const TcpConnectionPtr &conn, json &js, Timestamp)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();

    _groupModel.joinGroup(userid, groupid, "normal");

    json response;
    response["msgid"] = JOIN_GROUP_MSG;
    response["errno"] = 0;
    response["errmsg"] = "加入群组成功";
    conn->send(response.dump());
}

// 群聊业务
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();

    // 查群成员（排除自己）
    vector<int> members = _groupModel.queryGroupUsers(userid, groupid);

    lock_guard<mutex> lock(_connMutex);
    for (int memberid : members)
    {
        auto it = _userConnMap.find(memberid);
        if (it != _userConnMap.end())
        {
            // 在线，直接转发消息
            it->second->send(js.dump());
        }
        else
        {
            // 离线，存离线消息
            _offlineMsgModel.insert(memberid, js.dump());
        }
    }
}
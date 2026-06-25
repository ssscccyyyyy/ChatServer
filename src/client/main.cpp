// ChatServer 客户端程序
// 主线程：显示菜单、获取用户输入、发送 JSON 请求
// 子线程：持续接收服务器消息并展示（实时显示私聊、群聊等推送）
#include <iostream>
#include <string>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <json.hpp>
#include <muduo/base/Logging.h>

using json = nlohmann::json;
using namespace std;

// 命令处理器类型：无参函数，执行具体业务逻辑
using CommandHandler = function<void()>;

// 全局状态变量（接收线程和主线程共享）
int client_fd = -1;          // 与服务器的 TCP 连接
string current_name;          // 当前登录用户名
int current_id = -1;          // 当前登录用户 id
bool is_login = false;        // 是否已登录成功

// 消息类型常量（与 server/public.hpp 保持一致）
const int LOGIN_MSG = 1;
const int REG_MSG = 2;
const int REG_MSG_ACK = 3;
const int LOGIN_MSG_ACK = 4;
const int ONE_CHAT_MSG = 5;
const int ADD_FRIEND_MSG = 6;
const int CREATE_GROUP_MSG = 7;
const int JOIN_GROUP_MSG = 8;
const int GROUP_CHAT_MSG = 9;

// 创建 TCP 套接字并连接到 127.0.0.1:6000
bool connectServer()
{
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0)
    {
        cerr << "创建 socket 失败" << endl;
        return false;
    }

    sockaddr_in addr;      // 定义一个 IPv4 地址结构体
    memset(&addr, 0, sizeof(addr)); // 清零初始化，避免残留数据
    addr.sin_family = AF_INET;// 设置为 IPv4 协议族
    addr.sin_port = htons(6000);// 设置端口 6000（主机字节序 → 网络字节序）
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (connect(client_fd, (sockaddr *)&addr, sizeof(addr)) < 0)
    {
        cerr << "连接服务器失败" << endl;
        return false;
    }
    return true;
}

// 将 JSON 对象序列化为字符串并通过 TCP 发送给服务器
void sendJson(const json &js)
{
    string msg = js.dump();
    send(client_fd, msg.c_str(), msg.size(), 0);
}

// 接收子线程：持续从服务器接收消息，按 msgid 分类处理
void recvThread()
{
    char buffer[4096];
    while (true)
    {
        // 阻塞接收服务器数据
        memset(buffer, 0, sizeof(buffer));
        //recv 是 Linux socket 的接收数据函数，阻塞等待服务器发数据过来
        int len = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        //len< 0连接出错（网络异常）
        //len= 0连接关闭（对方正常断开）
        if (len <= 0)
        {
            // 连接断开（可能是登出或服务器关闭）
            // 不清除 client_fd，让主线程处理重连或退出
            return;
        }

        // 解析 JSON
        json js = json::parse(buffer);
        int msgid = js["msgid"].get<int>();

        if (msgid == ONE_CHAT_MSG)
        {
            // 收到私聊消息：显示发送者名称和内容
            cout << "\n[私聊] " << js["name"].get<string>()
                 << ": " << js["msg"].get<string>() << endl;
        }
        else if (msgid == GROUP_CHAT_MSG)
        {
            // 收到群聊消息：显示发送者名称和内容
            cout << "\n[群聊] " << js["name"].get<string>()
                 << ": " << js["msg"].get<string>() << endl;
        }
        else if (msgid == LOGIN_MSG_ACK)
        {
            // 登录响应处理
            int errno_val = js["errno"].get<int>();
            if (errno_val == 0)
            {
                // 登录成功：记录用户信息，显示好友列表和离线消息
                current_id = js["id"].get<int>();
                current_name = js["name"].get<string>();
                is_login = true;
                cout << "\n[登录成功] 欢迎 " << current_name << endl;

                // 显示好友列表（包含 id、昵称、在线状态）
                if (js.contains("friends"))
                {
                    cout << "好友列表:" << endl;
                    for (auto &f : js["friends"])
                    {
                        cout << "  ID:" << f["id"] << " " << f["name"]
                             << " [" << f["state"] << "]" << endl;
                    }
                }
                // 显示离线期间收到的消息
                if (js.contains("offlinemsg"))
                {
                    cout << "离线消息:" << endl;
                    for (auto &msg : js["offlinemsg"])
                    {
                        json msgJson = json::parse(msg.get<string>());
                        int mtype = msgJson["msgid"].get<int>();
                        if (mtype == ONE_CHAT_MSG)
                        {
                            cout << "  [私聊] " << msgJson["name"] << ": "
                                 << msgJson["msg"] << endl;
                        }
                        else if (mtype == GROUP_CHAT_MSG)
                        {
                            cout << "  [群聊-" << msgJson["groupid"] << "] "
                                 << msgJson["name"] << ": "
                                 << msgJson["msg"] << endl;
                        }
                    }
                }
                // 登录成功，提示可用命令
                cout << "\n可用命令: addfriend chat creategroup groupchat joingroup logout quit" << endl;
            }
            else if (errno_val == 1)
            {
                cout << "\n[登录失败] 用户名或密码错误" << endl;
            }
            else if (errno_val == 2)
            {
                cout << "\n[登录失败] 该账号已登录" << endl;
            }
            cout << "\n> " << flush;
        }
        else if (msgid == REG_MSG_ACK)
        {
            // 注册响应：显示注册结果和分配的 ID
            int errno_val = js["errno"].get<int>();
            if (errno_val == 0)
            {
                cout << "\n[注册成功] 您的 ID: " << js["id"] << endl;
            }
            else
            {
                cout << "\n[注册失败]" << endl;
            }
            cout << "\n> " << flush;
        }
        else if (msgid == ADD_FRIEND_MSG)
        {
            // 添加好友响应
            if (js.contains("errmsg"))
            {
                cout << "\n[好友] " << js["errmsg"].get<string>() << endl;
            }
            cout << "\n> " << flush;
        }
        else if (msgid == CREATE_GROUP_MSG)
        {
            // 创建群组响应
            int errno_val = js["errno"].get<int>();
            if (errno_val == 0)
            {
                cout << "\n[建群成功] 群ID: " << js["id"] << endl;
            }
            else
            {
                cout << "\n[建群失败]" << endl;
            }
            cout << "\n> " << flush;
        }
        else if (msgid == JOIN_GROUP_MSG)
        {
            // 加入群组响应
            if (js.contains("errmsg"))
            {
                cout << "\n[加群] " << js["errmsg"].get<string>() << endl;
            }
            cout << "\n> " << flush;
        }
    }
}

// 命令映射表声明（定义在文件末尾的 initCommands 中）
extern map<string, CommandHandler> _loginCommands;
extern map<string, CommandHandler> _userCommands;

// 显示当前可用的命令列表
void showMenu()
{
    auto &cmds = is_login ? _userCommands : _loginCommands;
    cout << "\n========== ChatServer 客户端 ==========" << endl;
    for (auto &entry : cmds)
    {
        cout << "  " << entry.first << endl;
    }
    cout << "=======================================" << endl;
    cout << "输入命令: ";
}

// 登录：输入 id + 密码，发送 LOGIN_MSG
void doLogin()
{
    int id;
    string pwd;
    cout << "输入 ID: ";
    if (!(cin >> id))
    {
        cin.clear();
        cin.ignore(1024, '\n');
        cout << "ID 必须是数字" << endl;
        return;
    }
    cout << "输入密码: ";
    cin >> pwd;

    json js;
    js["msgid"] = LOGIN_MSG;
    js["id"] = id;
    js["password"] = pwd;
    sendJson(js);
}

// 注册：输入用户名 + 密码，发送 REG_MSG
void doReg()
{
    string name, pwd;
    cout << "输入用户名: ";
    cin >> name;
    cout << "输入密码: ";
    cin >> pwd;

    json js;
    js["msgid"] = REG_MSG;
    js["name"] = name;
    js["password"] = pwd;
    sendJson(js);
}

// 私聊：输入对方 id 和消息内容，发送 ONE_CHAT_MSG
void doOneChat()
{
    int toid;
    string msg;
    cout << "对方 ID: ";
    if (!(cin >> toid))
    {
        cin.clear(); cin.ignore(1024, '\n');
        cout << "ID 必须是数字" << endl; return;
    }
    cout << "消息内容: ";
    cin.ignore();
    getline(cin, msg);

    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = current_id;
    js["name"] = current_name;
    js["to"] = toid;
    js["msg"] = msg;
    sendJson(js);
}

// 添加好友：输入对方 id，发送 ADD_FRIEND_MSG
void doAddFriend()
{
    int friendid;
    cout << "对方 ID: ";
    if (!(cin >> friendid))
    {
        cin.clear(); cin.ignore(1024, '\n');
        cout << "ID 必须是数字" << endl; return;
    }

    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = current_id;
    js["friendid"] = friendid;
    sendJson(js);
}

// 创建群组：输入群名和描述，发送 CREATE_GROUP_MSG
void doCreateGroup()
{
    string name, desc;
    cout << "群组名称: ";
    cin >> name;
    cout << "群组描述: ";
    cin.ignore();
    getline(cin, desc);

    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = current_id;
    js["groupname"] = name;
    js["groupdesc"] = desc;
    sendJson(js);
}

// 加入群组：输入群 id，发送 JOIN_GROUP_MSG
void doJoinGroup()
{
    int groupid;
    cout << "群组 ID: ";
    if (!(cin >> groupid))
    {
        cin.clear(); cin.ignore(1024, '\n');
        cout << "ID 必须是数字" << endl; return;
    }

    json js;
    js["msgid"] = JOIN_GROUP_MSG;
    js["id"] = current_id;
    js["groupid"] = groupid;
    sendJson(js);
}

// 群聊：输入群 id 和消息内容，发送 GROUP_CHAT_MSG
void doGroupChat()
{
    int groupid;
    string msg;
    cout << "群组 ID: ";
    if (!(cin >> groupid))
    {
        cin.clear(); cin.ignore(1024, '\n');
        cout << "ID 必须是数字" << endl; return;
    }
    cout << "消息内容: ";
    cin.ignore();
    getline(cin, msg);

    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = current_id;
    js["name"] = current_name;
    js["groupid"] = groupid;
    js["msg"] = msg;
    sendJson(js);
}

// 退出登录：关闭连接→服务器自动设离线→重新连接
void doLogout()
{
    is_login = false;
    current_id = -1;
    current_name = "";
    close(client_fd);                         // 关闭旧连接，服务端检测断开后自动设 offline
    cout << "已退出登录" << endl;
    if (!connectServer())                     // 重新连接，准备下一次登录
    {
        cout << "重新连接失败，请重启客户端" << endl;
        exit(1);
    }
    thread recv(recvThread);
    recv.detach();
}

// 退出程序：关闭连接并结束进程
void doQuit()
{
    close(client_fd);
    exit(0);
}

// 命令映射表（key=命令名，value=处理函数）
map<string, CommandHandler> _loginCommands;  // 未登录可用命令
map<string, CommandHandler> _userCommands;   // 登录后可用命令

// 初始化命令映射表（符合开闭原则：新增命令只需在这里加一行）
void initCommands()
{
    _loginCommands["login"] = doLogin;
    _loginCommands["reg"] = doReg;
    _loginCommands["quit"] = doQuit;

    _userCommands["chat"] = doOneChat;
    _userCommands["addfriend"] = doAddFriend;
    _userCommands["creategroup"] = doCreateGroup;
    _userCommands["joingroup"] = doJoinGroup;
    _userCommands["groupchat"] = doGroupChat;
    _userCommands["logout"] = doLogout;
    _userCommands["quit"] = doQuit;
}

// 程序入口：连接服务器 → 启动接收线程 → 循环显示菜单处理用户输入
int main()
{
    if (!connectServer())
    {
        return -1;
    }

    // 初始化命令映射表
    initCommands();

    // 启动接收子线程（detach 分离，主线程退出时自动结束）
    thread recv(recvThread);
    recv.detach();

    // 主线程：循环显示菜单，处理用户输入
    while (true)
    {
        if (cin.fail())  // 修复：cin 进入错误状态时恢复
        {
            cin.clear();
            cin.ignore(1024, '\n');
        }

        showMenu();
        string cmd;
        cin >> cmd;

        if (cin.fail())
        {
            cout << "输入异常，请重试" << endl;
            continue;
        }

        // 根据当前登录状态选择对应的命令映射表
        auto &cmds = is_login ? _userCommands : _loginCommands;
        auto it = cmds.find(cmd);
        if (it != cmds.end())
        {
            // 找到命令，执行对应的处理函数
            it->second();
        }
        else
        {
            cout << "未知命令 '" << cmd << "'，请重新输入" << endl;
        }
    }

    return 0;
}

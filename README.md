# ChatServer - 分布式集群即时通讯系统

一款基于 **C++ + muduo 网络库 + MySQL + Redis + Nginx** 实现的分布式集群即时聊天系统，支持服务端与客户端全功能交互。

---

## 技术栈

| 技术 | 用途 |
|:----|:------|
| **C++11** | 项目核心开发语言 |
| **muduo** | Reactor 高并发网络库，处理 TCP 事件驱动 |
| **CMake** | 项目构建管理 |
| **MySQL** | 数据持久化存储 |
| **Redis** | 发布订阅（Pub/Sub），跨服务器消息转发 |
| **Nginx** | TCP 四层负载均衡（stream 模块） |
| **hiredis** | Redis C 语言客户端库 |
| **nlohmann/json** | JSON 序列化/反序列化 |

---

## 项目结构

```
ChatServer/
├── CMakeLists.txt              # 根级构建配置
├── include/                    # 头文件
│   ├── public.hpp              # 消息类型枚举定义
│   └── server/
│       ├── chatserver.hpp      # 网络层（TcpServer 封装）
│       ├── chatservice.hpp     # 业务层（消息路由 + 单例）
│       ├── user.hpp            # User 实体类（ORM）
│       ├── group.hpp           # Group 实体类
│       ├── groupuser.hpp       # 群成员实体类（继承 User）
│       ├── db/db.h             # MySQL 数据库封装
│       ├── model/              # 数据操作层
│       │   ├── usermodel.hpp
│       │   ├── friendmodel.hpp
│       │   ├── groupmodel.hpp
│       │   └── offlinemessagemodel.hpp
│       └── redis/redis.hpp     # Redis 发布订阅封装
├── src/
│   ├── CMakeLists.txt
│   ├── client/
│   │   ├── CMakeLists.txt
│   │   └── main.cpp            # 客户端（主线程发+子线程收）
│   └── server/
│       ├── CMakeLists.txt
│       ├── main.cpp            # 服务端入口
│       ├── chatserver.cpp      # 网络层实现
│       ├── chatservice.cpp     # 业务层实现
│       ├── db/db.cpp
│       ├── model/
│       │   ├── usermodel.cpp
│       │   ├── friendmodel.cpp
│       │   ├── groupmodel.cpp
│       │   └── offlinemessagemodel.cpp
│       └── redis/redis.cpp
├── test/                       # 测试程序
├── thirdparty/json.hpp         # JSON 库
└── CMakeLists.txt              # 根 CMake 配置
```

---

## 已实现功能

### 服务端

| 功能 | msgid | 说明 |
|:----|:-----:|:------|
| 用户注册 | 2 | 用户名+密码注册，返回自动分配的 ID |
| 用户登录 | 1 | ID+密码登录，返回用户信息、好友列表、离线消息 |
| 一对一聊天 | 5 | 在线用户实时转发，离线用户消息存储 |
| 添加好友 | 6 | 双向好友关系建立 |
| 创建群组 | 7 | 创建者自动设为群主 |
| 加入群组 | 8 | 普通成员加入 |
| 群组聊天 | 9 | 在线成员实时转发，离线成员消息存储 |
| 客户端异常退出 | — | TCP 断开自动检测，更新用户离线状态 |

### 客户端

| 命令 | 功能 |
|:-----|:------|
| `reg` | 注册新用户 |
| `login` | 用户登录 |
| `chat` | 一对一私聊 |
| `addfriend` | 添加好友 |
| `creategroup` | 创建群组 |
| `joingroup` | 加入群组 |
| `groupchat` | 群组聊天 |
| `logout` | 退出登录（服务端自动离线） |
| `quit` | 退出程序 |

---

## 架构设计

### 分层架构

```
┌─────────────────────────────────┐
│       客户端 (main.cpp)          │
│  主线程：输入 → 发送 JSON        │
│  子线程：接收 JSON → 展示        │
└──────────────┬──────────────────┘
               │ TCP (JSON)
┌──────────────▼──────────────────┐
│   Nginx TCP 负载均衡 (stream)     │
│   127.0.0.1:6000 → 6001/6002    │
└──────────────┬──────────────────┘
               │
┌──────────────▼──────────────────┐
│  ChatServer (网络层)             │
│  onMessage → json::parse        │
│  → getHandler(msgid) → 路由     │
└──────────────┬──────────────────┘
               │
┌──────────────▼──────────────────┐
│  ChatService (业务层)            │
│  单例 + msgid→Handler 映射表     │
│  _userConnMap 在线用户管理        │
└──────┬───────────────┬──────────┘
       │               │
┌──────▼──────┐  ┌─────▼────────┐
│  Model 层   │  │  Redis Pub/Sub│
│  MySQL CRUD │  │  跨服消息转发 │
└─────────────┘  └──────────────┘
```

### 消息路由

客户端发送 JSON 包含 `msgid` 字段，服务端根据 `msgid` 查找路由表，调用对应业务函数。

```
                   msgid 映射表
msgid=1  →  login()      登录
msgid=2  →  reg()        注册
msgid=5  →  oneChat()    私聊
msgid=6  →  addFriend()  添加好友
msgid=7  →  createGroup() 建群
msgid=8  →  joinGroup()  加群
msgid=9  →  groupChat()  群聊
```

---

## 数据库设计

| 表名 | 说明 |
|:-----|:------|
| `user` | 用户表（id, name, password, state） |
| `Friend` | 好友关系表（userid, friendid） |
| `OfflineMessage` | 离线消息表（userid, message, create_time） |
| `AllGroup` | 群组信息表（id, groupname, groupdesc） |
| `GroupUser` | 群组成员表（groupid, userid, grouprole） |

---

## 环境依赖

- **CMake** ≥ 3.10
- **muduo** 网络库（[github.com/chenshuo/muduo](https://github.com/chenshuo/muduo)）
- **MySQL** 8.0+
- **Redis** 7.0+
- **Nginx**（需 stream 模块，可用 `nginx-full`）
- **hiredis**（[github.com/redis/hiredis](https://github.com/redis/hiredis)）

---

## 编译与运行

### 编译

```bash
git clone https://github.com/ssscccyyyyy/ChatServer.git
cd ChatServer
mkdir build && cd build
cmake .. && make -j$(nproc)
```

### 运行（单机模式）

```bash
# 1. 导入数据库表结构
mysql -uroot -p < sql/init.sql    # 自行创建 chat 数据库

# 2. 启动服务端
./bin/ChatServer

# 3. 启动客户端
./bin/ChatClient
```

### 运行（集群模式）

```bash
# 1. 启动多个 ChatServer 实例
./bin/ChatServer 6001
./bin/ChatServer 6002

# 2. 配置 Nginx TCP 负载均衡
#    在 /etc/nginx/nginx.conf 中添加：
#
#    stream {
#        upstream chatserver {
#            server 127.0.0.1:6001;
#            server 127.0.0.1:6002;
#        }
#        server {
#            listen 6000;
#            proxy_pass chatserver;
#        }
#    }

# 3. 重启 nginx
sudo systemctl restart nginx

# 4. 客户端连接 nginx 端口 6000
./bin/ChatClient
```

---

## 通信协议

基于 JSON 格式，通过 `msgid` 区分消息类型。

### 请求示例

```json
{"msgid":1, "id":1, "password":"123"}          // 登录
{"msgid":2, "name":"user", "password":"123"}   // 注册
{"msgid":5, "id":1, "to":2, "msg":"你好"}      // 私聊
{"msgid":6, "id":1, "friendid":2}              // 加好友
{"msgid":7, "id":1, "groupname":"群名"}         // 建群
{"msgid":9, "id":1, "groupid":1, "msg":"大家好"} // 群聊
```

### 响应示例

```json
// 登录成功（含好友列表）
{"errno":0, "id":1, "name":"user", "msgid":4,
 "friends":[{"id":2, "name":"friend", "state":"online"}]}

// 注册成功
{"errno":0, "id":3, "msgid":3}
```

---

## 项目进度

- [x] 项目骨架搭建（CMake + 目录结构）
- [x] JSON 序列化集成
- [x] muduo 网络库封装
- [x] MySQL 数据库封装
- [x] 用户注册 / 登录
- [x] 一对一聊天（在线 + 离线）
- [x] 好友管理
- [x] 群组功能（建群/加群/群聊）
- [x] 客户端程序（命令映射模式）
- [x] Redis 封装（hiredis）
- [x] Nginx TCP 负载均衡配置
- [ ] Redis 集成到 ChatService（跨服务器消息转发）
- [ ] 客户端 UI 优化

---

## License

MIT

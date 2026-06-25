#pragma once
#include "server/user.hpp"
#include <vector>

// Friend 表的数据操作类（Model 层）
// 维护用户之间的好友关系（双向关系表）
class FriendModel
{
public:
    // 添加好友关系（单条记录，调用方需插入双向）
    void insert(int userid, int friendid);

    // 查询用户的好友列表（通过连表查询返回 User 对象）
    vector<User> query(int userid);
};
#pragma once
#include "server/user.hpp"

// User 表的数据操作类（Model 层）
// 封装对 user 表的 insert / query / updateState 操作
class UserModel
{
public:
    // 插入用户记录，成功后会把自增 id 回填给 user 对象
    bool insert(User &user);

    // 根据用户 id 查询用户信息（返回 User 对象）
    User query(int id);

    // 更新用户的在线状态（online / offline）
    bool updateState(User user);
};
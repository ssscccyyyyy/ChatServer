#pragma once
#include "server/user.hpp"

// 群成员实体类，继承 User，额外加上群内角色
class GroupUser : public User
{
public:
    void setRole(string role) { _role = role; }
    string getRole() const { return _role; }

private:
    string _role;  // 群内角色：creator（群主）/ normal（普通成员）
};
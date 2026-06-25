#pragma once
#include <string>
#include <vector>
#include "server/groupuser.hpp"
using namespace std;

// 群组实体类，对应 AllGroup 表
class Group
{
public:
    Group(int id = -1, string name = "", string desc = "")
    {
        this->groupid = id;
        this->_groupname = name;
        this->_groupdesc = desc;
    }

    int getId() const { return groupid; }
    void setId(int id) { groupid = id; }

    string getName() const { return _groupname; }
    void setName(const string &name) { _groupname = name; }

    string getDesc() const { return _groupdesc; }
    void setDesc(const string &desc) { _groupdesc = desc; }

    vector<GroupUser> &getUsers() { return _users; }
    void setUsers(const vector<GroupUser> &users) { _users = users; }

private:
    int groupid;                     // 群组id
    string _groupname;           // 群组名称
    string _groupdesc;           // 群组描述
    vector<GroupUser> _users;    // 群成员列表
};
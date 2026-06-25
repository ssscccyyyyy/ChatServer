#pragma once
#include <string>
using namespace std;

// User实体类，对应数据库中的User表 ORM类
class User
{
public:
    // 构造函数：默认id=-1（未入库时），state="offline"
    User(int id = -1, string name = "", string pwd = "", string state = "offline")
    {
        this->id = id;
        this->name = name;
        this->password = pwd;
        this->state = state;
    }

    int getId() const { return id; }
    void setId(int id) { this->id = id; }

    string getName() const { return name; }
    void setName(const string &name) { this->name = name; }

    string getPassword() const { return password; }
    void setPassword(const string &pwd) { this->password = pwd; }

    string getState() const { return state; }
    void setState(const string &state) { this->state = state; }

private:
    int id;          // 用户id
    string name;     // 用户名
    string password; // 用户密码
    string state;    // 当前状态：online / offline
};

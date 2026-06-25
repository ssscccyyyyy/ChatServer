// User 表的数据操作类（Model 层）
#include "server/model/usermodel.hpp"
#include "server/db/db.h"
#include <iostream>

// 向 user 表插入一条用户记录（name, password, state）
// 插入成功后通过 mysql_insert_id() 获取自增 id 并回填给 user 对象
bool UserModel::insert(User &user)
{
    char sql[1024];
    sprintf(sql, "insert into user(name,password,state) values('%s','%s','%s')",
            user.getName().c_str(), user.getPassword().c_str(), user.getState().c_str());
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            // 把插入后 MySQL 自增的 id 回填给 User 对象
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

// 更新用户的在线状态（online / offline）
bool UserModel::updateState(User user)
{
    char sql[1024];
    sprintf(sql, "update user set state='%s' where id=%d",
            user.getState().c_str(), user.getId());
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}

// 根据用户 id 查询用户信息
// 返回 User 对象（若未查到，返回默认 User，其 id = -1）
User UserModel::query(int id)
{
    char sql[1024];
    sprintf(sql, "select * from user where id=%d", id);

    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));       // id
                user.setName(row[1]);            // name
                user.setPassword(row[2]);        // password
                user.setState(row[3]);           // state
                mysql_free_result(res);
                return user;
            }
        }
    }
    return User();  // 未查到，返回默认对象
}
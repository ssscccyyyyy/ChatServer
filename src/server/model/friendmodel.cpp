#include "server/model/friendmodel.hpp"
#include "server/db/db.h"

// 添加好友关系（双向：userid -> friendid）
void FriendModel::insert(int userid, int friendid)
{
    char sql[1024];
    // 向 Friend 表插入一条好友关系记录
    sprintf(sql, "insert into Friend(userid, friendid) values(%d, %d)", userid, friendid);
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

// 查询用户的昵好友列表（通过连表查询拿到好友的 id、name、state）
vector<User> FriendModel::query(int userid)
{
    char sql[1024];
    // Friend 表与 user 表联合查询：根据 userid 查出所有好友的信息
    sprintf(sql, "select user.id, user.name, user.state from Friend join user on Friend.friendid = user.id where Friend.userid = %d", userid);

    vector<User> vec;
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            // 遍历结果集，将每一行封装成 User 对象
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));       // 好友 id
                user.setName(row[1]);            // 好友名称
                user.setState(row[2]);           // 好友在线状态
                vec.push_back(user);
            }
            mysql_free_result(res);              // 释放结果集
        }
    }
    return vec;
}

#include "server/model/groupmodel.hpp"
#include "server/db/db.h"

// 创建群组
bool GroupModel::creatGroup(Group &group)
{
    char sql[1024];
    sprintf(sql, "insert into AllGroup(groupname, groupdesc) values('%s','%s')",
            group.getName().c_str(), group.getDesc().c_str());
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

// 加入群组
void GroupModel::joinGroup(int userid, int groupid, string role)
{
    char sql[1024];
    sprintf(sql, "insert into GroupUser(groupid, userid, grouprole) values(%d, %d, '%s')",
            groupid, userid, role.c_str());
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

// 查询用户所在的群组 id 列表
vector<int> GroupModel::queryGroup(int userid)
{
    char sql[1024];
    sprintf(sql, "select groupid from GroupUser where userid=%d", userid);

    vector<int> vec;
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                vec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
    }
    return vec;
}

// 根据 groupid 查询群成员用户 id 列表，排除当前用户自己
vector<int> GroupModel::queryGroupUsers(int userid, int groupid)
{
    char sql[1024];
    sprintf(sql, "select userid from GroupUser where groupid=%d and userid!=%d",
            groupid, userid);

    vector<int> vec;
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                vec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
    }
    return vec;
}

#include "server/model/offlinemessagemodel.hpp"
#include "server/db/db.h"

// 存储用户的离线消息
void OfflineMsgModel::insert(int userid, string msg)
{
    char sql[1024];
    // 将消息 JSON 字符串插入 OfflineMessage 表，关联该用户
    sprintf(sql, "insert into OfflineMessage(userid, message) values(%d,'%s')", userid, msg.c_str());
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

// 删除用户的离线消息（上线推送成功后清理）
void OfflineMsgModel::remove(int userid)
{
    char sql[1024];
    // 删除该用户的所有离线消息
    sprintf(sql, "delete from OfflineMessage where userid=%d", userid);
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

// 查询用户的离线消息（返回 JSON 字符串数组）
vector<string> OfflineMsgModel::query(int userid)
{
    char sql[1024];
    // 查出该用户所有离线消息的 message 字段
    sprintf(sql, "select message from OfflineMessage where userid=%d", userid);
    vector<string> vec;
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                vec.push_back(row[0]);  // row[0] → message 字段
            }
            mysql_free_result(res);
        }
    }
    return vec;
}
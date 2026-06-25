#pragma once
#include <string>
#include <vector>
using namespace std;

// OfflineMessage 表的数据操作类（Model 层）
// 用户不在线时，消息存到 OfflineMessage 表中；
// 用户上线后，拉取离线消息并删除
class OfflineMsgModel
{
public:
    // 存储用户的离线消息
    void insert(int userid, string msg);

    // 删除用户的离线消息（上线推送后清理）
    void remove(int userid);

    // 查询用户的离线消息列表，返回 JSON 字符串数组
    vector<string> query(int userid);
};
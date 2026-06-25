
#include"server/group.hpp"
#include<vector>
using namespace std;
class GroupModel{

    public:
    //创建群组
        bool creatGroup(Group & group);
    //加入群组
        void joinGroup(int userid,int groupid,string role);
    //查询用户所在的群组信息
    vector<int> queryGroup(int userid);
    //根据指定的groupid查询用户id列表,除userid，主要用户群聊业务给群组其它成员群发消息
    vector<int> queryGroupUsers(int userid,int groupid);
};
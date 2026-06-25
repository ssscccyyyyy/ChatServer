
//server和client的公共文件

enum EnMsgType{
    LOGIN_MSG=1,//登录消息
    REG_MSG=2,    //注册消息
    REG_MSG_ACK=3, //注册响应消息
    LOGIN_MSG_ACK=4, //登录响应消息
    ONE_CHAT_MSG=5, //聊天消息
    ADD_FRIEND_MSG=6, //添加好友消息
    CREATE_GROUP_MSG=7,
    JOIN_GROUP_MSG=8,
    GROUP_CHAT_MSG=9,
};
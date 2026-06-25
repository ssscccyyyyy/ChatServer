#include "server/db/db.h"

// 构造函数：初始化 MYSQL 连接句柄
MySQL::MySQL()
{
    _conn = mysql_init(nullptr);
}

// 析构函数：关闭数据库连接，释放资源
MySQL::~MySQL()
{
    if (_conn != nullptr)
        mysql_close(_conn);
}

// 连接数据库（使用全局配置连接 MySQL 服务器）
bool MySQL::connect()
{
    MYSQL *p = mysql_real_connect(_conn, server.c_str(), user.c_str(),
                                  password.c_str(), dbname.c_str(), 3306, nullptr, 0);
    if (p != nullptr)
    {
        // 设置字符集为 gbk（支持中文存储）
        mysql_query(_conn, "set names gbk");
        LOG_INFO << "connect mysql success";
    }
    else
    {
        LOG_INFO << "connect mysql fail!";
    }
    return p;
}

// 执行 SQL 更新语句（insert / update / delete）
bool MySQL::update(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":"
                 << sql << " 更新失败! " << mysql_error(_conn);
        return false;
    }
    return true;
}

// 执行 SQL 查询语句（select），返回结果集
MYSQL_RES *MySQL::query(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":"
                 << sql << " 查询失败!";
        return nullptr;
    }
    return mysql_use_result(_conn);
}

// 获取原始 MYSQL 连接句柄
// 用于 mysql_insert_id() 等需要直接操作句柄的场景
MYSQL *MySQL::getConnection()
{
    return _conn;
}
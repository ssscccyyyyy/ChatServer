#include <mysql/mysql.h>
#include <string>
#include <muduo/base/Logging.h>
using namespace std;

// 数据库配置信息
static string server = "127.0.0.1";
static string user = "root";
static string password = "123456";
static string dbname = "chat";

// MySQL 数据库操作封装类
// 封装 MySQL C API，提供连接管理、更新、查询等基础操作
class MySQL
{
public:
    // 构造函数：初始化 MYSQL 连接句柄
    MySQL();
    // 析构函数：关闭数据库连接，释放资源
    ~MySQL();
    // 连接数据库（使用全局配置：地址、用户名、密码、库名）
    bool connect();
    // 更新操作（insert / update / delete），成功返回 true
    bool update(string sql);
    // 查询操作（select），返回结果集，失败返回 nullptr
    MYSQL_RES *query(string sql);
    // 获取原始 MYSQL 连接句柄（用于 mysql_insert_id 等操作）
    MYSQL *getConnection();

private:
    MYSQL *_conn;  // MySQL 连接句柄
};

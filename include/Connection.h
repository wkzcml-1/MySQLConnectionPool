#ifndef __CONNECTION__
#define __CONNECTION__

#include "Common.h"
#include <mysql/mysql.h>

class Connection {
public:
    // 构造与析构函数
    Connection();
    ~Connection();       

    // 数据库连接
    bool connect(
        std::string ip,     // ip
        unsigned int port,  // 端口
        std::string user,   // 用户
        std::string passwd, // 密码
        std::string dbname // 数据库
    );

    // 数据库更新操作
    bool update(std::string sql);

    // 数据库查询操作
    MYSQL_RES* query(std::string sql);

    void updateTimePoint();
    bool isIdleTimeOut(int maxIdleTime);
private:
    // 与mysql服务器的一条连接
    MYSQL* _conn;

    // 记录进入线程池时间
    std::chrono::steady_clock::time_point _time;
};


#endif
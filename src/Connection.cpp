#include "Connection.h"

Connection::Connection() {
    this->_conn = mysql_init(nullptr);
    if (this->_conn == nullptr) {
        LOG("Init mysql error");
        exit(1);
    }
}

Connection::~Connection() {
    if (this->_conn == nullptr) {
        mysql_close(this->_conn);
    }
}

bool Connection::connect(
        std::string ip,     // ip
        unsigned int port,  // 端口
        std::string user,   // 用户
        std::string passwd, // 密码
        std::string dbname  // 数据库
) {
    if (!mysql_real_connect(
        this->_conn, 
        ip.c_str(),
        user.c_str(),
        passwd.c_str(),
        dbname.c_str(),
        port,
        nullptr,
        0
    )) {
        // 连接失败
        return false;
    }
    return true;
}   

bool Connection::update(std::string sql) {
    if (mysql_query(this->_conn, sql.c_str())) {
        LOG("Update error " + sql);
        return false;
    }
    return true;
}


MYSQL_RES* Connection::query(std::string sql) {
    if (mysql_query(this->_conn, sql.c_str())) {
        LOG("Query error " + sql);
        return nullptr;
    }
    return mysql_use_result(this->_conn);
}

void Connection::updateTimePoint() {
    _time = std::chrono::steady_clock::now();
}

bool Connection::isIdleTimeOut(int maxIdleTime) {
    // 现在时间
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>
        (now - _time).count() > maxIdleTime;
}
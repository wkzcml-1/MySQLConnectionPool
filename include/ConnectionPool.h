#ifndef __CONNECTION_POOL__
#define __CONNECTION_POOL__

#include <atomic>
#include <mutex>
#include <queue>
#include <condition_variable>

#include <fstream>
#include <jsoncpp/json/json.h>

#include <thread>
#include <memory>
#include <functional>

#include "Common.h"
#include "Connection.h"

class ConnectionPool {
public:
    // 单例模式
    ~ConnectionPool();
    ConnectionPool(const ConnectionPool& other)=delete;
    Connection operator=(const ConnectionPool& other)=delete;

    // 获取唯一实例
    static ConnectionPool* getPoolInstance();

    // 从连接池中获取空闲连接
    std::shared_ptr<Connection> getConnection();

private:
    // 单例模式
    ConnectionPool();

    // 载入配置文件
    bool loadConfigFile(std::string path);

    // 创建新的数据库连接
    bool createNewConnection();

    // 生成新连接的线程函数, 以防连接不够使用
    void produceConnection();

    // 扫描连接池，删除超时连接
    void scanConnection();

    /* 线程池参数 */
    // 网络相关
    std::string _ip;
    unsigned int _port;
    // 用户相关
    std::string _username;
    std::string _password;
    std::string _dbname;
    // 连接池参数
    int _initSize;      // 初始连接量
    int _maxSize;       // 最大连接量
    int _maxIdleTime;   // 最大空闲时间
    int _connectTimeOut;// 连接超时时间
    
    /* 线程池队列 (线程安全) */
    std::mutex _queMtx;
    // 连接计数器
    std::atomic_int _connectCnt;
    std::queue<Connection*> _connectQue;
    // 条件变量，负责生产连接线程与工作通信
    std::condition_variable _queCv;
};

#endif


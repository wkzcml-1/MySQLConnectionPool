#include "ConnectionPool.h"

const std::string CONFIG_FILE_PATH = "../config/mysql.json";

ConnectionPool::ConnectionPool() {

    // 加载配置文件
    if (!loadConfigFile(CONFIG_FILE_PATH)) {
        LOG("Fail to load config file " + CONFIG_FILE_PATH);
        return;
    }

    _connectCnt = 0;
    // 创建初始连接池
    for (int i = 0; i < _initSize; ++i) {
        createNewConnection();
    }

    std::thread producer(std::bind(&ConnectionPool::produceConnection, this));
    producer.detach();

    std::thread scanner(std::bind(&ConnectionPool::scanConnection, this));
    scanner.detach();
}


ConnectionPool* ConnectionPool::getPoolInstance() {
    static ConnectionPool pool_;
    return &pool_;
}


// 载入配置文件: 使用
bool ConnectionPool::loadConfigFile(std::string path) {
    /* 打开文件 */
    std::ifstream input(path);
    if (!input.is_open()) {
        LOG("Fail to open " + path);
        return false;
    }

    /* 解析json 文件 */
    Json::Value root;
    Json::Reader reader;

    if (!reader.parse(input, root, false)) {
        LOG("Parse failed");
        return false;
    }

    // 写入数据
    _ip = root["ip"].asString();
    _port = root["port"].asUInt();
    _username = root["username"].asString();
    _password = root["password"].asString();
    _dbname = root["dbname"].asString();

    _initSize = root["initSize"].asInt();
    _maxSize = root["maxSize"].asInt();
    _maxIdleTime = root["maxIdleTime"].asInt();
    _connectTimeOut = root["connectTimeOut"].asInt();

    // for DEBUG
    LOG("ip: " + _ip);
    LOG("port: " + std::to_string(_port));
    LOG("usename: " + _username);
    LOG("password: " + _password);
    LOG("dbname: " + _dbname);

    LOG("initSize: " + std::to_string(_initSize));
    LOG("maxSize: " + std::to_string(_maxSize));
    LOG("maxIdleTime: " + std::to_string(_maxIdleTime));
    LOG("connectTimeOut: " + std::to_string(_connectTimeOut));

    return true;
}


void ConnectionPool::produceConnection() {
    while (true) {
        // 上锁
        std::unique_lock<std::mutex> lock(_queMtx);

        // 线程池不为空，无需生产
        while (!_connectQue.empty()) {
            _queCv.wait(lock);
        }

        // 超过最大连接限制，无法生产
        if (_connectCnt >= _maxSize)
            continue;
        
        // 生产新连接
        if (createNewConnection()) {
            // 成功通知消费者线程
            _queCv.notify_all();
        }
    }
}

bool ConnectionPool::createNewConnection() {
    auto conn = new Connection();
    if (!conn->connect(_ip, _port, 
        _username, _password, _dbname)) {
        LOG("Fail to create new database connection");
    }

    conn->updateTimePoint();
    _connectQue.push(conn);
    ++ _connectCnt;
    return true;
}


std::shared_ptr<Connection> ConnectionPool::getConnection() {
    // 加锁
    std::unique_lock<std::mutex> lock(_queMtx);
    // 尝试获取连接
    while (_connectQue.empty()) {
        _queCv.wait_for(lock, std::chrono::milliseconds(_connectTimeOut));
        if (_connectQue.empty()) {
            LOG("Get connection timeout");
            return nullptr;
        }
    }

    // 指定删除函数，重新回收资源
    std::shared_ptr<Connection> ret(_connectQue.front(), 
    [&] (Connection* conn) { 
        std::lock_guard<std::mutex> guard(_queMtx);
        _connectQue.push(conn);
    });

    _connectQue.pop();

    // 消费完成
    _queCv.notify_all();

    return ret;
}


void ConnectionPool::scanConnection() {
    while (true) {
        // 休眠一段时间
        auto idleTime = std::chrono::seconds(_maxIdleTime);
        std::this_thread::sleep_for(idleTime);

        // 获得锁
        std::unique_lock<std::mutex> lock(_queMtx);

        // 根据队列进出规则，队首连接更容易超时
        while (_connectCnt > _initSize 
            && !_connectQue.empty()) {
            auto pconn = _connectQue.front();
            if (pconn->isIdleTimeOut(_maxIdleTime)) {
                _connectQue.pop();
                delete pconn;
                -- _connectCnt;
            }
        }
    }
}

ConnectionPool::~ConnectionPool() {
    std::lock_guard<std::mutex> guard(_queMtx);
    while (!_connectQue.empty()) {
        auto pconn = _connectQue.front();
        _connectQue.pop();
        delete pconn;
    }
}
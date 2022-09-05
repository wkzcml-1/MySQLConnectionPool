#include "ConnectionPool.h"

const int THREAD_NUM = 4;
const int MAX_TEST_TIME = 10000;

using namespace std;

int main() {
    string sql  = "insert into users(name, age, sex) "
        "values('刘智', 38, 'male')";
    auto start = chrono::steady_clock::now();

    Connection conn;
    conn.connect("127.0.0.1", 3306, "kai", "123456", "db");
    for (int i = 0; i < MAX_TEST_TIME; ++i) {
        conn.update(sql);
    }

    auto end = chrono::steady_clock::now();
    cout << "单线程: ";
    cout << chrono::duration_cast<chrono::milliseconds>(end - start).count() << "ms" << endl;
    
    conn.update("delete from users");

    start = chrono::steady_clock::now();

    auto pool = ConnectionPool::getPoolInstance();
    for (int i = 0; i < MAX_TEST_TIME; ++i) {
        auto pconn = pool->getConnection();
        pconn->update(sql);
    }

    end = chrono::steady_clock::now();
    cout << "单线程 线程池：";
    cout << chrono::duration_cast<chrono::milliseconds>(end - start).count() << "ms" << endl;

    start = chrono::steady_clock::now();

    thread t1([sql] {
        auto myPool = ConnectionPool::getPoolInstance();
        for (int i = 0; i < MAX_TEST_TIME / 4; ++i) {
            auto pconn = myPool->getConnection();
            pconn->update(sql);
        }
    });

    thread t2([sql] {
        auto myPool = ConnectionPool::getPoolInstance();
        for (int i = 0; i < MAX_TEST_TIME / 4; ++i) {
            auto pconn = myPool->getConnection();
            pconn->update(sql);
        }
    });

    thread t3([sql] {
        auto myPool = ConnectionPool::getPoolInstance();
        for (int i = 0; i < MAX_TEST_TIME / 4; ++i) {
            auto pconn = myPool->getConnection();
            pconn->update(sql);
        }
    });

    thread t4([sql] {
        auto myPool = ConnectionPool::getPoolInstance();
        for (int i = 0; i < MAX_TEST_TIME / 4; ++i) {
            auto pconn = myPool->getConnection();
            pconn->update(sql);
        }
    });

    t1.join();
    t2.join();
    t3.join();
    t4.join();

    end = chrono::steady_clock::now();
    cout << "四线程 线程池：";
    cout << chrono::duration_cast<chrono::milliseconds>(end - start).count() << "ms" << endl;
    
    return 0;
}

#include<thread>
#include<functional>
#include<deque>
#include<vector>
#include<mutex>
#include<condition_variable>
#include <atomic>

typedef std::function<void()> task_type;

class TaskPool final {
private:
    std::deque<task_type> queue_;
    std::vector<std::thread> threads_;
    std::mutex queue_mutex_;
    std::condition_variable cond_queue_not_empty;
    std::atomic<bool> stop_{false};
public:
    TaskPool();
    TaskPool( const TaskPool& ) = delete;
    TaskPool( TaskPool&& ) = delete;
    TaskPool& operator=( const TaskPool& ) = delete;
    TaskPool& operator=( TaskPool&& ) = delete;
    ~TaskPool();
    
    void AddTask( const task_type& task );
};



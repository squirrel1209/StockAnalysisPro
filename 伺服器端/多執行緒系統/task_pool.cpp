#include "task_pool.h"

/// 建構子：初始化執行緒池
TaskPool::TaskPool() :
    // 根據硬體支援的執行緒數，建立對應數量的 std::thread
    threads_( std::thread::hardware_concurrency() ) 
{
    // 對每一個執行緒物件進行初始化
    for ( auto& t : threads_ ) {
    	
        // 對每個執行緒指派 lambda 函式（工作主迴圈）
        t = std::thread( [this]() {                                
        	
            while ( !stop_ ) {  // 如果沒有收到結束訊號，就持續執行

                std::unique_lock<std::mutex> lock( queue_mutex_ ); // 鎖住任務佇列以保護共享資源

                // 等待佇列非空或 stop_ 為 true
                cond_queue_not_empty.wait( lock, [this]() {
                    return !queue_.empty() || stop_; 
                });

                // 如果 stop_ 被設為 true，就結束執行緒
                if ( stop_ ) {
                    return;
                }

                // 從任務佇列中取出第一個任務
                auto task = std::move( queue_.front() );
                queue_.pop_front();  // 移除已取出的任務
                lock.unlock();       // 釋放鎖，讓其他執行緒能加入任務

                // 執行任務
                if ( task ) {
                    task();
                }
            }
        });
    }
}

/// 解構子：安全關閉所有執行緒
TaskPool::~TaskPool() {
    stop_ = true; // 設定停止旗標，讓所有執行緒結束主迴圈

    cond_queue_not_empty.notify_all(); // 喚醒所有等待中的執行緒

    // 等待所有執行緒完成（join）
    for ( auto& t : threads_ ) {
        if ( t.joinable() ) {
            t.join();
        }
    }
}

/// 將任務加入任務佇列
void TaskPool::AddTask( const task_type& task ) {
    {
        std::lock_guard<std::mutex> lock( queue_mutex_ ); // 鎖住佇列，防止競爭條件
        queue_.push_back( task ); // 加入任務到尾端
    }

    cond_queue_not_empty.notify_one(); // 喚醒一個等待中的執行緒處理任務
}

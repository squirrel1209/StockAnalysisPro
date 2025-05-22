#include <thread>                 // 用於 std::thread 執行緒管理
#include <functional>            // 用於 std::function 任務包裝
#include <deque>                 // 用於 std::deque 任務佇列（雙端佇列）
#include <vector>                // 用於儲存多個執行緒
#include <mutex>                 // 用於 std::mutex 排他鎖保護共享資源
#include <condition_variable>    // 用於執行緒同步的條件變數
#include <atomic>                // 用於 std::atomic 原子操作（執行緒安全）

// 任務類型定義：每個任務是一個 void() 無參數、無回傳值的函式包裝
typedef std::function<void()> task_type;

/// @brief 執行緒池類別：允許多執行緒同時處理佇列中的任務
class TaskPool final {
private:
    std::deque<task_type> queue_;              // 任務佇列，FIFO 儲存待執行的任務
    std::vector<std::thread> threads_;         // 執行緒池中所有執行緒的容器
    std::mutex queue_mutex_;                   // 互斥鎖，保護 queue_ 的存取
    std::condition_variable cond_queue_not_empty; // 條件變數，用於通知任務佇列有新任務可執行
    std::atomic<bool> stop_{false};            // 停止旗標，執行緒會根據此旗標決定是否終止

public:
    /// 預設建構子：根據硬體支援的核心數自動建立對應數量的工作執行緒
    TaskPool();

    // 禁止複製與移動操作，避免資源誤用或重複釋放
    TaskPool( const TaskPool& ) = delete;
    TaskPool( TaskPool&& ) = delete;
    TaskPool& operator=( const TaskPool& ) = delete;
    TaskPool& operator=( TaskPool&& ) = delete;

    /// 解構子：安全關閉執行緒池並釋放資源
    ~TaskPool();
    
    /// 新增一個任務至佇列，供空閒的執行緒擷取並執行
    /// @param task 使用 std::function 封裝的任務（可為 lambda、function pointer、bind 結果）
    void AddTask( const task_type& task );
};

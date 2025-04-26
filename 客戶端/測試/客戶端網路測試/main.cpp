#include "ClientApplication.h"
#include <iostream>

// 主程式，啟動客戶端
int main() {
    // 初始化客戶端，連線到本地伺服器
    ClientApplication client("127.0.0.1", 8080, "output.json");

    // 執行客戶端邏輯
    if (!client.run()) {
        std::cerr << "客戶端執行失敗" << std::endl;
        return -1;
    }

    std::cout << "客戶端執行成功" << std::endl;
    return 0;
}
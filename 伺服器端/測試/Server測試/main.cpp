#include "FileReader.h"
#include "NetworkServer.h"
#include "JsonPacket.h"
#include <iostream>

#define PORT 8080

// 主程式，展示伺服器端傳送 JSON 封包
int main() {
    // 創建檔案讀取器
    FileReader fileReader;
    // 讀取 JSON 檔案
    std::string json_data = fileReader.readJsonFile("stock_data_AAPL.json");

    if (json_data.empty()) {
        std::cerr << "讀取 JSON 檔案失敗" << std::endl;
        return -1;
    }

    std::cout << "讀取的 JSON 資料: " << json_data << std::endl;

    // 創建網路伺服器
    NetworkServer server(PORT);

    if (!server.initialize()) {
        std::cerr << "伺服器初始化失敗" << std::endl;
        return -1;
    }

    if (!server.startListening()) {
        std::cerr << "伺服器監聽失敗" << std::endl;
        return -1;
    }

    if (!server.acceptConnection()) {
        std::cerr << "接受連線失敗" << std::endl;
        return -1;
    }

    JsonPacket packet(json_data);
    if (!server.sendPacket(packet)) {
        std::cerr << "傳送資料失敗" << std::endl;
        return -1;
    }

    std::cout << "資料傳送成功" << std::endl;

    // ✅ 傳完資料後就結束，不要再等 client 回應！

    return 0;
}

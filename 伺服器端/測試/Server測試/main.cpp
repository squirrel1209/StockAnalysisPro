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

    // 檢查檔案讀取
    if (json_data.empty()) {
        std::cerr << "讀取 JSON 檔案失敗" << std::endl;
        return -1;
    }

    std::cout << "讀取的 JSON 資料: " << json_data << std::endl;

    // 創建網路伺服器
    NetworkServer server(PORT);

    // 初始化伺服器
    if (!server.initialize()) {
        std::cerr << "伺服器初始化失敗" << std::endl;
        return -1;
    }

    // 開始監聽
    if (!server.startListening()) {
        std::cerr << "伺服器監聽失敗" << std::endl;
        return -1;
    }

    // 接受客戶端連線
    if (!server.acceptConnection()) {
        std::cerr << "接受連線失敗" << std::endl;
        return -1;
    }

    // 創建並傳送 JSON 封包
    JsonPacket packet(json_data);
    if (!server.sendPacket(packet)) {
        std::cerr << "傳送資料失敗" << std::endl;
        return -1;
    }

    std::cout << "資料傳送成功" << std::endl;

    // 等待客戶端回應
    std::string response = server.receive();
    if (response.empty()) {
        std::cerr << "未收到客戶端回應" << std::endl;
    } else {
        std::cout << "收到客戶端回應: " << response << std::endl;
    }

    return 0;
}
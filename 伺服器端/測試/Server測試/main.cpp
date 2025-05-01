#include "FileReader.h"
#include "NetworkServer.h"
#include "JsonPacket.h"
#include "task_pool.h"
#include "json.hpp"
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

#define PORT 8080

using json = nlohmann::json;

int main() {
    std::cout << "[INFO] 啟動伺服器..." << std::endl;

    FileReader fileReader;

    // 🛠 讀取五個 JSON 檔案
    std::vector<std::string> filenames = {
        "stock_data_AAPL.json",
        "stock_data_AMZN.json",
        "stock_data_GOOGL.json",
        "stock_data_MSFT.json",
        "stock_data_NVDA.json"
    };

    std::vector<std::string> json_data_list;

    for (const auto& filename : filenames) {
        std::cout << "[INFO] 讀取檔案: " << filename << std::endl;
        std::string json = fileReader.readJsonFile(filename);
        if (json.empty()) {
            std::cerr << "[ERROR] 讀取檔案失敗: " << filename << std::endl;
            return -1;
        }
        json_data_list.push_back(json);
    }

    // 組合成 JSON 陣列
    std::cout << "[INFO] 建立 JSON 陣列..." << std::endl;
    json json_array = json::array();
    for (const auto& json_data : json_data_list) {
        try {
            json json_obj = json::parse(json_data);
            json_array.push_back(json_obj);
            std::cout << "[INFO] 已解析 JSON: " << json_obj["Meta Data"]["2. Symbol"].get<std::string>() << std::endl;
        } catch (const json::exception& e) {
            std::cerr << "[ERROR] 解析 JSON 失敗: " << e.what() << std::endl;
            return -1;
        }
    }

    // 轉換為字串
    std::string json_array_str = json_array.dump();
    std::cout << "[INFO] JSON 陣列大小: " << json_array_str.size() << " bytes" << std::endl;

    NetworkServer server(PORT);

    std::cout << "[INFO] 初始化伺服器..." << std::endl;
    if (!server.initialize()) {
        std::cerr << "[ERROR] 伺服器初始化失敗" << std::endl;
        return -1;
    }

    std::cout << "[INFO] 開始監聽埠口 " << PORT << "..." << std::endl;
    if (!server.startListening()) {
        std::cerr << "[ERROR] 伺服器監聽失敗" << std::endl;
        return -1;
    }

    TaskPool tp;

    while (true) {
        std::cout << "[INFO] 等待客戶端連線..." << std::endl;
        if (!server.acceptConnection()) {
            std::cerr << "[ERROR] 接受連線失敗" << std::endl;
            continue;
        }

        SOCKET client = server.getClientSocket();
        std::cout << "[INFO] 客戶端已連線: SOCKET " << client << std::endl;

        tp.AddTask([client, json_array_str, &server]() {
            // ✅ 傳送單一 JSON 陣列
            JsonPacket packet(json_array_str);

            if (server.sendPacket(client, packet)) {
                std::cout << "[INFO] [SOCKET " << client << "] 傳送 JSON 陣列成功" << std::endl;
            } else {
                std::cerr << "[ERROR] [SOCKET " << client << "] 傳送 JSON 陣列失敗" << std::endl;
            }

            // 延遲關閉連線，確保客戶端能完整接收資料
            std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // 延遲 2 秒
            closesocket(client);
            std::cout << "[INFO] [SOCKET " << client << "] 客戶端連線已關閉" << std::endl;
        });
    }

    return 0;
}

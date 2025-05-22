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

    // 🔄 讀取多個 JSON 檔案
    std::vector<std::string> filenames = {
        "../TechnicalIndicators/output_json/stock_data_AAPL_processed.json",
        "../TechnicalIndicators/output_json/stock_data_AMZN_processed.json",
        "../TechnicalIndicators/output_json/stock_data_GOOGL_processed.json",
        "../TechnicalIndicators/output_json/stock_data_MSFT_processed.json",
        "../TechnicalIndicators/output_json/stock_data_NVDA_processed.json",
        "../TechnicalIndicators/output_json/stock_data_TSLA_processed.json",
        "../TechnicalIndicators/output_json/stock_data_META_processed.json",
        "../TechnicalIndicators/output_json/stock_data_INTC_processed.json",
        "../TechnicalIndicators/output_json/stock_data_ORCL_processed.json",
        "../TechnicalIndicators/output_json/stock_data_IBM_processed.json",
        "../TechnicalIndicators/output_json/stock_data_NFLX_processed.json",
        "../TechnicalIndicators/output_json/stock_data_AMD_processed.json",
        "../TechnicalIndicators/output_json/stock_data_BABA_processed.json",
        "../TechnicalIndicators/output_json/stock_data_JPM_processed.json",
        "../TechnicalIndicators/output_json/stock_data_V_processed.json",
        "../TechnicalIndicators/output_json/stock_data_UNH_processed.json"
    };

    std::vector<std::string> json_data_list;

    for (const auto& filename : filenames) {
        std::cout << "[INFO] 讀取檔案: " << filename << std::endl;
        std::string json_data = fileReader.readJsonFile(filename);
        if (json_data.empty()) {
            std::cerr << "[ERROR] 讀取檔案失敗: " << filename << std::endl;
            return -1;
        }
        json_data_list.push_back(json_data);
    }

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

    // ✅ 使用 thread pool
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
            JsonPacket packet(json_array_str);

            if (server.sendPacket(client, packet)) {
                std::cout << "[INFO] [SOCKET " << client << "] 傳送 JSON 陣列成功" << std::endl;
            } else {
                std::cerr << "[ERROR] [SOCKET " << client << "] 傳送 JSON 陣列失敗" << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // 確保 client 收完整
            closesocket(client);
            std::cout << "[INFO] [SOCKET " << client << "] 客戶端連線已關閉" << std::endl;
        });
    }

    return 0;
}

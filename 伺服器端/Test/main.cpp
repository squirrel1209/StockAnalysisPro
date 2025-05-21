// main.cpp
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "FileReader.h"
#include "JsonPacket.h"
#include "NetworkServer.h"
#include "json.hpp"
#include "task_pool.h"

#define PORT 8080

using json = nlohmann::json;

int main() {
    std::cout << "[INFO] 啟動伺服器..." << std::endl;

    FileReader fileReader;

    // 讀取五個 JSON 檔案
    std::vector<std::string> filenames = {
        "../TechnicalIndicators/output_json/stock_data_AAPL_processed.json",
        "../TechnicalIndicators/output_json/stock_data_AMZN_processed.json",
        "../TechnicalIndicators/output_json/stock_data_GOOGL_processed.json",   // 有些 API 用 GOOG，有些用 GOOGL，請依實際命名調整
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

    // 設置 JSON 數據
    server.setJsonData(json_array_str);

    // 啟動伺服器主迴圈
    server.run();

    return 0;
}
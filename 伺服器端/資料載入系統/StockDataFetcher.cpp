// StockDataFetcher.cpp
#include "StockDataFetcher.h"
#include <iostream>
#include <fstream>
#include <curl/curl.h>
#include <thread>
#include <chrono>

// 建構子：使用 API 金鑰初始化
StockDataFetcher::StockDataFetcher(const std::string& apiKey)
    : apiKey_(apiKey) {}

// 靜態回呼函數：libcurl 將接收到的資料寫入字串中
size_t StockDataFetcher::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

// 根據股票代號從 Alpha Vantage API 抓取股票資料
std::string StockDataFetcher::getStockData(const std::string& symbol) {
    std::cout << "[LOG] 準備連線至 Alpha Vantage API，代碼: " << symbol << std::endl;
    std::string url = "https://www.alphavantage.co/query?function=TIME_SERIES_DAILY&symbol=" + symbol + "&apikey=" + apiKey_;
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "[ERROR] curl_easy_perform() 執行失敗，代碼: " << symbol << ", 錯誤訊息: " << curl_easy_strerror(res) << std::endl;
        } else {
            std::cout << "[LOG] 成功獲取資料: " << symbol << std::endl;
        }
        curl_easy_cleanup(curl);
    } else {
        std::cerr << "[ERROR] 初始化 CURL 失敗！" << std::endl;
    }
    return readBuffer;
}

// 將原始 JSON 資料儲存到本地檔案
void StockDataFetcher::saveJson(const std::string& symbol, const std::string& jsonData) {
    std::string filename = "stock_data_" + symbol + ".json";
    std::ofstream outFile(filename);
    if (outFile.is_open()) {
        outFile << jsonData;
        outFile.close();
        std::cout << "[LOG] JSON 資料成功儲存至: " << filename << std::endl;
    } else {
        std::cerr << "[ERROR] 無法儲存 JSON 至 " << filename << std::endl;
    }
}

// 批次抓取多個股票代碼的資料並儲存成 JSON
void StockDataFetcher::fetchAndSaveAll(const std::vector<std::string>& symbols) {
    for (size_t i = 0; i < symbols.size(); ++i) {
        std::cout << "[LOG] 開始處理股票代碼: " << symbols[i] << std::endl;
        std::string jsonData = getStockData(symbols[i]);

        // 儲存原始 JSON 檔案
        saveJson(symbols[i], jsonData);

        // 每次 API 請求間隔 12 秒，避免超出速率限制
        if (i < symbols.size() - 1) {
            std::cout << "[LOG] 等待 12 秒以符合 API 速率限制..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(12));
        }
    }
    std::cout << "[LOG] 所有股票資料處理完成！" << std::endl;
}
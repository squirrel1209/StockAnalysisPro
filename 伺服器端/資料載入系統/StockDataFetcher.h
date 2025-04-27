// StockDataFetcher.h
#pragma once

#include <string>
#include <vector>

// StockDataFetcher 類別：負責從 Alpha Vantage API 抓取股票資料並儲存成 JSON
class StockDataFetcher {
public:
    // 建構子：初始化 API 金鑰
    StockDataFetcher(const std::string& apiKey);

    // 抓取並儲存多個股票代碼的資料
    void fetchAndSaveAll(const std::vector<std::string>& symbols);

private:
    std::string apiKey_; // API 金鑰

    // 靜態回呼函式：供 libcurl 使用，將下載內容寫入字串
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output);

    // 從 API 抓取單一股票代碼的 JSON 資料
    std::string getStockData(const std::string& symbol);

    // 儲存 JSON 資料到本地檔案
    void saveJson(const std::string& symbol, const std::string& jsonData);
};

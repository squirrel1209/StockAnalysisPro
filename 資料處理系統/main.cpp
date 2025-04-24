#include <iostream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include <fstream>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// 回調函數：處理 libcurl 回傳的資料
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

// 從 Alpha Vantage API 獲取股票資料
std::string getStockData(const std::string& symbol, const std::string& apiKey) {
    std::string url = "https://www.alphavantage.co/query?function=TIME_SERIES_DAILY&symbol=" + symbol + "&apikey=" + apiKey;
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
            std::cerr << "curl_easy_perform() failed for " << symbol << ": " << curl_easy_strerror(res) << std::endl;
        }
        curl_easy_cleanup(curl);
    }
    return readBuffer;
}

// 儲存原始 JSON
void saveJson(const std::string& symbol, const std::string& jsonData) {
    std::string filename = "stock_data_" + symbol + ".json";
    std::ofstream outFile(filename);
    if (outFile.is_open()) {
        outFile << jsonData;
        outFile.close();
        std::cout << "JSON 儲存至 " << filename << std::endl;
    } else {
        std::cerr << "無法儲存 " << filename << std::endl;
    }
}

// 儲存元資料
void saveMetaData(const std::string& symbol, const json& metaData) {
    std::string filename = "meta_data_" + symbol + ".json";
    std::ofstream outFile(filename);
    if (outFile.is_open()) {
        outFile << metaData.dump(4); // 美化輸出，縮排 4 空格
        outFile.close();
        std::cout << "元資料儲存至 " << filename << std::endl;
    } else {
        std::cerr << "無法儲存 " << filename << std::endl;
    }
}

// 轉為 CSV
void jsonToCsv(const std::string& symbol, const json& j) {
    std::string csvFilename = "stock_data_" + symbol + ".csv";
    std::ofstream csvFile(csvFilename);
    if (!csvFile.is_open()) {
        std::cerr << "無法儲存 " << csvFilename << std::endl;
        return;
    }

    csvFile << "Date,Open,High,Low,Close,Volume\n";
    auto timeSeries = j["Time Series (Daily)"];
    for (auto& [date, data] : timeSeries.items()) {
        csvFile << date << "," << data["1. open"] << "," << data["2. high"] << ","
                << data["3. low"] << "," << data["4. close"] << "," << data["5. volume"] << "\n";
    }
    csvFile.close();
    std::cout << "CSV 儲存至 " << csvFilename << std::endl;
}

// 主程式
int main() {
    // 從環境變數讀取 API 金鑰
    std::string apiKey = std::getenv("ALPHA_VANTAGE_API_KEY") ? std::getenv("ALPHA_VANTAGE_API_KEY") : "QZDLARSUDF976X0R";

    // 硬編碼 6 個公司的股票代碼
    std::vector<std::string> symbols = {"TSLA", "AAPL", "MSFT", "GOOGL", "AMZN", "NVDA"};

    // 處理每個股票
    for (size_t i = 0; i < symbols.size(); ++i) {
        std::cout << "正在獲取 " << symbols[i] << " 的資料..." << std::endl;
        std::string jsonData = getStockData(symbols[i], apiKey);

        // 儲存原始 JSON
        saveJson(symbols[i], jsonData);

        // 解析 JSON 並儲存
        try {
            json j = json::parse(jsonData);
            if (j.contains("Error Message")) {
                std::cerr << "API 錯誤: " << j["Error Message"] << std::endl;
                continue;
            }

            // 儲存元資料
            saveMetaData(symbols[i], j["Meta Data"]);

            // 轉為 CSV
            jsonToCsv(symbols[i], j);
        } catch (const std::exception& e) {
            std::cerr << "JSON 解析錯誤: " << e.what() << std::endl;
        }

        // API 限制延遲
        if (i < symbols.size() - 1) {
            std::cout << "等待 12 秒以遵守 API 限制..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(12));
        }
    }

    std::cout << "\n所有資料儲存完成！" << std::endl;
    system("pause");
    return 0;
}
#include <iostream>
#include <string>
#include <curl/curl.h>
#include "json.hpp"  // 使用 nlohmann/json 單檔版

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

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

        // ✅ 指定 SSL 憑證路徑（請確保 cacert.pem 放在執行檔旁邊）
        curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }

    return readBuffer;
}

int main() {
    std::string symbol = "TSLA";               // 股票代碼
    std::string apiKey = "QZDLARSUDF976X0R";   // 替換成你的 API 金鑰

    std::string jsonData = getStockData(symbol, apiKey);
    std::cout << "API 回傳資料前 500 字：\n";
    std::cout << jsonData.substr(0, 500) << std::endl;

    system("pause");
    return 0;
}

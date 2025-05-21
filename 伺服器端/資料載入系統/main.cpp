// main.cpp
#include "StockDataFetcher.h"
#include <cstdlib>  // 為了使用 std::getenv

int main() {
    // 從環境變數讀取 API 金鑰，若未設定則使用預設值
    std::string apiKey = "QZDLARSUDF976X0R";

    // 建立 StockDataFetcher 物件
    StockDataFetcher fetcher(apiKey);

    // 設定要抓取的股票代碼
    std::vector<std::string> symbols = {
        "TSLA",   // Tesla
        "AAPL",   // Apple
        "MSFT",   // Microsoft
        "GOOGL",  // Alphabet (Google)
        "AMZN",   // Amazon
        "NVDA",   // NVIDIA
        "META",   // Meta (Facebook)
        "INTC",   // Intel
        "ORCL",   // Oracle
        "IBM",    // IBM
        "NFLX",   // Netflix
        "AMD",    // AMD
        "BABA",   // Alibaba
        "JPM",    // JPMorgan Chase
        "V",      // Visa
        "UNH"     // UnitedHealth Group
    };

    // 執行抓取與儲存
    fetcher.fetchAndSaveAll(symbols);

    return 0;
}

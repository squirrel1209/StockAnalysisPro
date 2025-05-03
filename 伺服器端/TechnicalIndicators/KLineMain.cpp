#include <iostream>
#include <vector>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <string>
#include <cmath>
#include <limits>
#include <fstream>
#include <filesystem>
#include "KLine.h"
#include "Tech_Analysis.h"
#include "TradingSystem.h"
#include "TradeSignal.h"
#include "DataProcessor.h"
#include "KLineRecord.h"
#include "json.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

// 清理字符串，去除前後空格
std::string trim(const std::string &str)
{
    std::string s = str;
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char c)
                                    { return !std::isspace(c); }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char c)
                         { return !std::isspace(c); })
                .base(),
            s.end());
    return s;
}

// 輸出所有記錄
void printRecords(const std::vector<KLineRecord> &records)
{
    for (size_t i = 0; i < records.size(); ++i)
    {
        const auto &r = records[i];
        std::cout << std::fixed << std::setprecision(4);
        std::cout << "Day" << (i + 1) << ".open=" << r.open << "\n";
        std::cout << "Day" << (i + 1) << ".high=" << r.high << "\n";
        std::cout << "Day" << (i + 1) << ".low=" << r.low << "\n";
        std::cout << "Day" << (i + 1) << ".close=" << r.close << "\n";
        std::cout << "Day" << (i + 1) << ".volume=" << r.volume << "\n";
        std::cout << "Day" << (i + 1) << ".ma5=" << (std::isfinite(r.ma5) ? r.ma5 : 0.0) << "\n";
        std::cout << "Day" << (i + 1) << ".ma10=" << (std::isfinite(r.ma10) ? r.ma10 : 0.0) << "\n";
        std::cout << "Day" << (i + 1) << ".ma20=" << (std::isfinite(r.ma20) ? r.ma20 : 0.0) << "\n";
        std::cout << "Day" << (i + 1) << ".k=" << (std::isfinite(r.k) ? r.k : 0.0) << "\n";
        std::cout << "Day" << (i + 1) << ".d=" << (std::isfinite(r.d) ? r.d : 0.0) << "\n";
        std::cout << "Day" << (i + 1) << ".rsi=" << (std::isfinite(r.rsi) ? r.rsi : 0.0) << "\n";
        std::cout << "Day" << (i + 1) << ".macd_line=" << (std::isfinite(r.macd_line) ? r.macd_line : 0.0) << "\n";
        std::cout << "Day" << (i + 1) << ".signal_line=" << (std::isfinite(r.signal_line) ? r.signal_line : 0.0) << "\n";
        std::cout << "Day" << (i + 1) << ".histogram=" << (std::isfinite(r.histogram) ? r.histogram : 0.0) << "\n";
        std::cout << "Day" << (i + 1) << ".price_change_percent=" << (std::isfinite(r.price_change_percent) ? r.price_change_percent : 0.0) << "\n";
        std::cout << "Day" << (i + 1) << ".signal=" << r.signal << "\n";
        std::cout << "Day" << (i + 1) << ".strength=" << r.strength << "\n";
        std::cout << "Day" << (i + 1) << ".body_size=" << std::abs(r.close - r.open) << "\n";
        std::cout << "Day" << (i + 1) << ".body_type=" << (r.close > r.open ? "Bullish" : (r.close < r.open ? "Bearish" : "Doji")) << "\n";
        std::cout << "Day" << (i + 1) << ".upper_shadow=" << (r.high - std::max(r.open, r.close)) << "\n";
        std::cout << "Day" << (i + 1) << ".lower_shadow=" << (std::min(r.open, r.close) - r.low) << "\n";
    }
}

int main(int argc, char *argv[])
{
    // 設置工作目錄為執行檔所在目錄
    //fs::current_path(fs::path(argv[0]).parent_path());
    std::cout << "Working directory: " << fs::current_path().string() << std::endl;

    // 指定輸入和輸出資料夾路徑（相對路徑）
    std::string input_folder_path = "./json.file";
    std::string output_folder_path = "./output_json";

    // 檢查輸入資料夾是否存在
    if (!fs::exists(input_folder_path))
    {
        std::cerr << "Input folder does not exist: " << fs::absolute(input_folder_path).string()
                  << " (Current working directory: " << fs::current_path().string() << ")" << std::endl;
        return 1;
    }

    // 創建輸出資料夾（如果不存在）
    if (!fs::exists(output_folder_path))
    {
        fs::create_directory(output_folder_path);
        std::cout << "Created output folder: " << output_folder_path << std::endl;
    }

    // 遍歷輸入資料夾中的所有 JSON 檔案
    for (const auto &entry : fs::directory_iterator(input_folder_path))
    {
        if (entry.path().extension() == ".json")
        {
            std::string filename = entry.path().string();
            std::cout << "正在處理檔案: " << filename << std::endl;

            TradingSystem system;

            // 讀取 JSON 檔案
            std::ifstream file(filename);
            if (!file.is_open())
            {
                std::cerr << "無法開啟檔案: " << filename << std::endl;
                continue;
            }

            // 解析 JSON
            json j;
            try
            {
                file >> j;
            }
            catch (const json::parse_error &e)
            {
                std::cerr << "JSON 解析錯誤 (" << filename << "): " << e.what() << std::endl;
                file.close();
                continue;
            }
            file.close();

            // 提取 Meta Data
            json meta_data = j.contains("Meta Data") ? j["Meta Data"] : json::object();

            // 檢查 Time Series (Daily) 是否存在
            if (!j.contains("Time Series (Daily)"))
            {
                std::cerr << "JSON 中缺少 'Time Series (Daily)' 鍵 (" << filename << ")" << std::endl;
                continue;
            }

            // 從 JSON 提取 K 線數據
            std::vector<Candle> raw_candles;
            std::vector<std::string> dates;
            for (const auto &[date, data] : j["Time Series (Daily)"].items())
            {
                try
                {
                    double open = std::stod(data["1. open"].get<std::string>());
                    double high = std::stod(data["2. high"].get<std::string>());
                    double low = std::stod(data["3. low"].get<std::string>());
                    double close = std::stod(data["4. close"].get<std::string>());
                    int volume = static_cast<int>(std::stod(data["5. volume"].get<std::string>()));
                    raw_candles.emplace_back(open, high, low, close, volume);
                    dates.push_back(date);
                }
                catch (const std::exception &e)
                {
                    std::cerr << "無法轉換數據 for " << date << " (" << filename << "): " << e.what() << std::endl;
                    continue;
                }
            }

            // 按日期升序排序
            std::vector<std::pair<Candle, std::string>> candle_date_pairs;
            for (size_t i = 0; i < raw_candles.size(); ++i)
            {
                candle_date_pairs.emplace_back(raw_candles[i], dates[i]);
            }
            std::sort(candle_date_pairs.begin(), candle_date_pairs.end(),
                      [](const auto &a, const auto &b)
                      { return a.second < b.second; });

            // 分離排序後的 K 線和日期
            raw_candles.clear();
            dates.clear();
            for (const auto &pair : candle_date_pairs)
            {
                raw_candles.push_back(pair.first);
                dates.push_back(pair.second);
            }

            // 將 K 線數據加入 TradingSystem
            for (const auto &candle : raw_candles)
            {
                system.placeOrder(true, candle, 100);
                system.completeCandle();
            }

            // 使用 system.getCandles() 獲取處理後的 K 線數據
            std::vector<Candle> candles = system.getCandles();

            // 驗證 K 線數據
            for (size_t i = 0; i < raw_candles.size(); ++i)
            {
                if (!raw_candles[i].isValid())
                {
                    std::cerr << "無效 K 線數據於 Day" << (i + 1) << " (" << filename << "): Open=" << raw_candles[i].getOpen()
                              << ", High=" << raw_candles[i].getHigh()
                              << ", Low=" << raw_candles[i].getLow()
                              << ", Close=" << raw_candles[i].getClose() << "\n";
                    continue;
                }
            }

            std::vector<double> closes;
            for (const auto &c : candles)
            {
                closes.push_back(c.getClose());
            }

            DataProcessor::calculatePriceChanges(candles);
            SignalConfig config;
            config.price_change_threshold = 0.005;
            config.ema_fast = 12;
            config.ema_slow = 26;
            config.signal_period = 9;
            config.rsi_overbought = 70.0;
            config.rsi_oversold = 30.0;

            std::vector<MACDResult> macd_cache(candles.size(), MACDResult(0, 0, 0));
            for (size_t i = config.ema_slow - 1; i < candles.size(); ++i)
            {
                macd_cache[i] = MACDResult::macd(closes, i, config);
            }
            auto signals = generateTradeSignals(candles, closes, config, config.ema_slow + config.signal_period - 1, macd_cache);

            std::vector<KLineRecord> records;
            std::vector<double> k_cache(candles.size(), 0.0);
            for (size_t i = 0; i < candles.size(); ++i)
            {
                KLine kline(candles[i]);
                double ma5 = (i >= 4) ? Tech_Analysis::movingAverage(closes, 5, i) : 0.0;
                double ma10 = (i >= 9) ? Tech_Analysis::movingAverage(closes, 10, i) : 0.0;
                double ma20 = (i >= 19) ? Tech_Analysis::movingAverage(closes, 20, i) : 0.0;
                auto kd = (i >= 8) ? KDResult::stochasticKD(candles, 9, 3, i) : KDResult(0.0, 0.0);
                k_cache[i] = kd.getK();
                double d_val = kd.getD();
                if (i >= 10)
                {
                    d_val = (k_cache[i] + k_cache[i - 1] + k_cache[i - 2]) / 3.0;
                    if (!std::isfinite(d_val) || d_val < 0 || d_val > 100)
                    {
                        std::cerr << "Invalid %D at Day" << (i + 1) << " (" << filename << "): " << d_val << "\n";
                        d_val = k_cache[i];
                    }
                }
                else if (i >= 8)
                {
                    d_val = k_cache[i];
                }
                double rsi = (i >= 14) ? RSIResult::rsi(closes, 14, i, config) : 0.0;
                auto macd = (i >= config.ema_fast - 1) ? macd_cache[i] : MACDResult(0, 0, 0);
                std::string date = (i < dates.size()) ? dates[i] : "Unknown";
                std::string signal, strength;

                for (const auto &sig : signals)
                {
                    if (sig.getDate() == static_cast<int>(i + 1))
                    {
                        signal = sig.getSignal();
                        strength = sig.getStrength();
                        break;
                    }
                }

                KLineRecord record(
                    date, kline,
                    ma5, ma10, ma20,
                    k_cache[i], d_val,
                    rsi,
                    macd.macdLine, macd.signalLine, macd.histogram,
                    signal, strength);
                records.push_back(record);
            }

            // 輸出記錄到終端
            std::cout << "\n=== 檔案 " << filename << " 的處理結果 ===\n";
            printRecords(records);
            std::cout << "=====================================\n\n";

            // 將記錄轉換為 JSON 格式，包含 Meta Data 和所有技術指標欄位
            json output_json;
            output_json["Meta Data"] = meta_data; // 添加 Meta Data
            json time_series = json::object();
            for (size_t i = 0; i < records.size(); ++i)
            {
                const auto &record = records[i];
                json daily_data = json::object();

                // 基本 K 線數據
                std::stringstream ss;
                ss << std::fixed << std::setprecision(4);
                ss.str("");
                ss << record.open;
                daily_data["1. open"] = ss.str();
                ss.str("");
                ss << record.high;
                daily_data["2. high"] = ss.str();
                ss.str("");
                ss << record.low;
                daily_data["3. low"] = ss.str();
                ss.str("");
                ss << record.close;
                daily_data["4. close"] = ss.str();
                daily_data["5. volume"] = std::to_string(record.volume);
                ss.str("");
                ss << (std::isfinite(record.ma5) ? record.ma5 : 0.0);
                daily_data["6. ma5"] = ss.str();
                ss.str("");
                ss << (std::isfinite(record.ma10) ? record.ma10 : 0.0);
                daily_data["7. ma10"] = ss.str();
                ss.str("");
                ss << (std::isfinite(record.ma20) ? record.ma20 : 0.0);
                daily_data["8. ma20"] = ss.str();
                ss.str("");
                ss << (std::isfinite(record.k) ? record.k : 0.0);
                daily_data["9. k"] = ss.str();
                ss.str("");
                ss << (std::isfinite(record.d) ? record.d : 0.0);
                daily_data["10. d"] = ss.str();
                ss.str("");
                ss << (std::isfinite(record.rsi) ? record.rsi : 0.0);
                daily_data["11. rsi"] = ss.str();
                ss.str("");
                ss << (std::isfinite(record.macd_line) ? record.macd_line : 0.0);
                daily_data["12. macd_line"] = ss.str();
                ss.str("");
                ss << (std::isfinite(record.signal_line) ? record.signal_line : 0.0);
                daily_data["13. signal_line"] = ss.str();
                ss.str("");
                ss << (std::isfinite(record.histogram) ? record.histogram : 0.0);
                daily_data["14. histogram"] = ss.str();
                ss.str("");
                ss << (std::isfinite(record.price_change_percent) ? record.price_change_percent : 0.0);
                daily_data["15. price_change_percent"] = ss.str();
                daily_data["16. signal"] = record.signal;
                daily_data["17. strength"] = record.strength;
                ss.str("");
                ss << std::abs(record.close - record.open);
                daily_data["18. body_size"] = ss.str();
                daily_data["19. body_type"] = (record.close > record.open) ? "Bullish" : (record.close < record.open ? "Bearish" : "Doji");
                ss.str("");
                ss << (record.high - std::max(record.open, record.close));
                daily_data["20. upper_shadow"] = ss.str();
                ss.str("");
                ss << (std::min(record.open, record.close) - record.low);
                daily_data["21. lower_shadow"] = ss.str();
                time_series[record.date] = daily_data;
            }
            output_json["Time Series (Daily)"] = time_series;

            // 生成輸出檔案路徑（在輸出資料夾中，檔案名後添加 _processed）
            std::string output_filename = fs::path(filename).stem().string() + "_processed.json";
            std::string output_filepath = (fs::path(output_folder_path) / output_filename).string();

            // 寫入新的 JSON 檔案
            std::ofstream output_file(output_filepath);
            if (!output_file.is_open())
            {
                std::cerr << "無法創建輸出檔案: " << output_filepath << std::endl;
                continue;
            }
            output_file << output_json.dump(4); // 使用 4 空格縮進
            output_file.close();
            std::cout << "已生成輸出檔案: " << output_filepath << std::endl;
        }
    }

    std::cout << "所有 JSON 檔案處理完畢！" << std::endl;
    return 0;
}
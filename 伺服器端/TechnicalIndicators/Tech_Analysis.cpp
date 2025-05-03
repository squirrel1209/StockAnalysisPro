#include "Tech_Analysis.h"
#include "KLine.h"
#include <vector>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <limits>
#include <iostream>

double Tech_Analysis::movingAverage(const std::vector<double> &data, int period, int index)
{
    if (index < period - 1 || index >= static_cast<int>(data.size()))
    {
        return 0.0;
    }
    double sum = 0.0;
    int count = 0;
    for (int i = index - period + 1; i <= index; ++i)
    {
        if (std::isfinite(data[i]) && data[i] > 0)
        {
            sum += data[i];
            count++;
        }
    }
    return count == period ? sum / period : 0.0;
}

MACDResult MACDResult::macd(const std::vector<double> &data, int index, const SignalConfig &config)
{

    int min_index = config.ema_slow - 1;
    if (index < min_index || index >= static_cast<int>(data.size()) || !std::isfinite(data[index]))
    {
        return MACDResult(0, 0, 0);
    }

    std::vector<double> ema_fast(data.size(), 0.0), ema_slow(data.size(), 0.0);
    double k_fast = 2.0 / (config.ema_fast + 1.0);
    double k_slow = 2.0 / (config.ema_slow + 1.0);

    // Compute fast EMA
    int fast_sma_index = config.ema_fast - 1;
    if (index >= fast_sma_index)
    {
        double sum = 0.0;
        for (int i = fast_sma_index - config.ema_fast + 1; i <= fast_sma_index; ++i)
        {
            if (i < 0 || i >= static_cast<int>(data.size()) || !std::isfinite(data[i]) || data[i] <= 0)
            {
                std::cerr << "Invalid fast EMA data at i=" << i << "\n";
                return MACDResult(0, 0, 0);
            }
            sum += data[i];
        }
        ema_fast[fast_sma_index] = sum / config.ema_fast;
        for (int i = fast_sma_index + 1; i <= index; ++i)
        {
            if (i >= static_cast<int>(data.size()) || !std::isfinite(data[i]) || data[i] <= 0)
            {
                std::cerr << "Invalid fast EMA data at i=" << i << "\n";
                return MACDResult(0, 0, 0);
            }
            ema_fast[i] = data[i] * k_fast + ema_fast[i - 1] * (1.0 - k_fast);
        }
    }

    // Compute slow EMA
    int slow_sma_index = config.ema_slow - 1;
    if (index >= slow_sma_index)
    {
        double sum = 0.0;
        for (int i = slow_sma_index - config.ema_slow + 1; i <= slow_sma_index; ++i)
        {
            if (i < 0 || i >= static_cast<int>(data.size()) || !std::isfinite(data[i]) || data[i] <= 0)
            {
                std::cerr << "Invalid slow EMA data at i=" << i << "\n";
                return MACDResult(0, 0, 0);
            }
            sum += data[i];
        }
        ema_slow[slow_sma_index] = sum / config.ema_slow;

        for (int i = slow_sma_index + 1; i <= index; ++i)
        {
            if (i >= static_cast<int>(data.size()) || !std::isfinite(data[i]) || data[i] <= 0)
            {
                std::cerr << "Invalid slow EMA data at i=" << i << "\n";
                return MACDResult(0, 0, 0);
            }
            ema_slow[i] = data[i] * k_slow + ema_slow[i - 1] * (1.0 - k_slow);
        }
    }

    double macd_line = ema_fast[index] - ema_slow[index];
    if (!std::isfinite(macd_line))
    {
        std::cerr << "Invalid macd_line at index " << index << ": ema_fast=" << ema_fast[index]
                  << ", ema_slow=" << ema_slow[index] << "\n";
        return MACDResult(0, 0, 0);
    }

    // Compute signal line
    double signal_line = 0.0;
    if (index >= config.ema_slow + config.signal_period - 2)
    {
        std::vector<double> macd_values;
        for (int i = config.ema_slow - 1; i <= index; ++i)
        {
            double macd_val = ema_fast[i] - ema_slow[i];
            if (std::isfinite(macd_val))
            {
                macd_values.push_back(macd_val);
            }
        }
        if (macd_values.size() >= static_cast<size_t>(config.signal_period))
        {
            int signal_sma_index = config.ema_slow + config.signal_period - 2;
            if (index >= signal_sma_index)
            {
                double sum_signal = 0.0;
                for (int i = index - config.signal_period + 1; i <= index; ++i)
                {
                    double macd_val = ema_fast[i] - ema_slow[i];
                    if (std::isfinite(macd_val))
                    {
                        sum_signal += macd_val;
                    }
                }
                signal_line = sum_signal / config.signal_period;
                double k_signal = 2.0 / (config.signal_period + 1.0);
                for (int i = signal_sma_index + 1; i <= index; ++i)
                {
                    double macd_val = ema_fast[i] - ema_slow[i];
                    if (std::isfinite(macd_val))
                    {
                        signal_line = macd_val * k_signal + signal_line * (1.0 - k_signal);
                    }
                    if (!std::isfinite(signal_line))
                    {
                        std::cerr << "Invalid signal_line at index " << i << "\n";
                        return MACDResult(0, 0, 0);
                    }
                }
            }
        }
    }

    double histogram = macd_line - signal_line;
    if (!std::isfinite(histogram))
    {
        std::cerr << "Invalid histogram at index " << index << "\n";
        return MACDResult(0, 0, 0);
    }

    return MACDResult(macd_line, signal_line, histogram);
}

KDResult KDResult::stochasticKD(const std::vector<Candle> &candles, int period, int sma_period, int index)
{
    if (index < period - 1 || index >= static_cast<int>(candles.size()))
    {
        std::cerr << "無效索引或周期於 Day" << (index + 1) << ": index=" << index << ", period=" << period << ", candles.size=" << candles.size() << "\n";
        return KDResult(0.0, 0.0);
    }

    // %K 計算：使用前 9 天數據（索引 i-period+1 到 i）
    double lowest_low = std::numeric_limits<double>::max();
    double highest_high = std::numeric_limits<double>::lowest();
    int start = std::max(0, index - period + 1);
    int end = index; // 包含當前索引

    for (int i = start; i <= end; ++i)
    {
        if (i >= static_cast<int>(candles.size()))
        {
            std::cerr << "無效索引於 KD 計算: " << i << "\n";
            return KDResult(0.0, 0.0);
        }
        double low = candles[i].getLow();
        double high = candles[i].getHigh();
        if (!std::isfinite(low) || !std::isfinite(high) || low <= 0 || high <= 0)
        {
            std::cerr << "無效 K線數據於索引 " << i << ": low=" << low << ", high=" << high << "\n";
            return KDResult(0.0, 0.0);
        }
        // std::cerr << "  索引 " << i << " (Day" << (i + 1) << "): low=" << low << ", high=" << high << ", close=" << candles[i].getClose() << "\n";
        lowest_low = std::min(lowest_low, low);
        highest_high = std::max(highest_high, high);
    }

    double current_close = candles[index].getClose();
    if (!std::isfinite(current_close) || current_close <= 0)
    {
        std::cerr << "無效收盤價於索引 " << index << ": close=" << current_close << "\n";
        return KDResult(0.0, 0.0);
    }

    double k = (highest_high == lowest_low) ? 0.0 : ((current_close - lowest_low) / (highest_high - lowest_low)) * 100.0;
    if (!std::isfinite(k) || k < 0 || k > 100)
    {
        std::cerr << "無效 K 值於索引 " << index << ": k=" << k << ", close=" << current_close
                  << ", lowest_low=" << lowest_low << ", highest_high=" << highest_high << "\n";
        return KDResult(0.0, 0.0);
    }

    // %D 計算：對前 sma_period 天的 %K 取平均
    double d = k; // 默認為 %K，若無法計算 %D
    if (index >= period + sma_period - 2)
    {
        std::vector<double> k_values;
        // std::cerr << "計算 %D 於 Day" << (index + 1) << "，使用 %K 從索引 " << (index - sma_period + 1) << " 到 " << index << "\n";
        for (int i = index - sma_period + 1; i <= index; ++i)
        {
            if (i < period - 1)
            {
                k_values.push_back(0.0);
                std::cerr << "  索引 " << i << ": %K=0.0（數據不足）\n";
                continue;
            }
            double low = std::numeric_limits<double>::max();
            double high = std::numeric_limits<double>::lowest();
            int k_start = std::max(0, i - period + 1);
            int k_end = i;
            for (int j = k_start; j <= k_end; ++j)
            {
                if (j >= static_cast<int>(candles.size()))
                {
                    std::cerr << "無效索引於 KD 平滑計算: " << j << "\n";
                    continue;
                }
                double j_low = candles[j].getLow();
                double j_high = candles[j].getHigh();
                if (!std::isfinite(j_low) || !std::isfinite(j_high) || j_low <= 0 || j_high <= 0)
                {
                    std::cerr << "無效 K線數據於索引 " << j << ": low=" << j_low << ", high=" << j_high << "\n";
                    continue;
                }
                low = std::min(low, j_low);
                high = std::max(high, j_high);
            }
            double close = candles[i].getClose();
            if (!std::isfinite(close) || close <= 0)
            {
                std::cerr << "無效收盤價於索引 " << i << ": close=" << close << "\n";
                continue;
            }
            double k_val = (high == low) ? 0.0 : ((close - low) / (high - low)) * 100.0;
            if (!std::isfinite(k_val) || k_val < 0 || k_val > 100)
            {
                std::cerr << "無效 k_val 於索引 " << i << ": k_val=" << k_val << "\n";
                continue;
            }
            k_values.push_back(k_val);
        }

        if (!k_values.empty())
        {
            d = std::accumulate(k_values.begin(), k_values.end(), 0.0) / k_values.size();
            if (!std::isfinite(d) || d < 0 || d > 100)
            {
                std::cerr << "無效 D 值於索引 " << index << ": d=" << d << "\n";
                d = k; // 回退到 %K
            }
        }
    }

    return KDResult(k, d);
}

double RSIResult::rsi(const std::vector<double> &data, int period, int index, const SignalConfig &config)
{
    if (index < period || index >= static_cast<int>(data.size()))
    {
        return 0.0;
    }

    double gain = 0.0, loss = 0.0;
    for (int i = index - period + 1; i <= index; ++i)
    {
        double diff = data[i] - data[i - 1];
        if (!std::isfinite(diff))
        {
            std::cerr << "無效價格差異於索引 " << i << "\n";
            return 0.0;
        }
        if (diff > 0)
        {
            gain += diff;
        }
        else if (diff < 0)
        {
            loss -= diff;
        }
    }

    gain /= period;
    loss /= period;

    if (loss == 0)
    {
        return gain == 0 ? 0.0 : 100.0;
    }
    double rs = gain / loss;
    double rsi = 100.0 - (100.0 / (1.0 + rs));
    if (!std::isfinite(rsi) || rsi < 0 || rsi > 100)
    {
        std::cerr << "無效 RSI 於索引 " << index << ": rsi=" << rsi << "\n";
        return 0.0;
    }
    return rsi;
}

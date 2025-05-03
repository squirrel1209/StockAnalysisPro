#include "TradeSignal.h"
#include "Tech_Analysis.h"
#include "KLine.h"
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <cmath>
TradeSignal::TradeSignal(int date, const std::string &signal, const std::string &strength)
    : date(date), signal(signal), strength(strength) {}

int TradeSignal::getDate() const { return date; }
std::string TradeSignal::getSignal() const { return signal; }
std::string TradeSignal::getStrength() const { return strength; }

std::vector<TradeSignal> generateTradeSignals(const std::vector<Candle> &candles,
                                              const std::vector<double> &closes,
                                              const SignalConfig &config,
                                              int lookback,
                                              const std::vector<MACDResult> &macd_cache)
{
    std::vector<TradeSignal> signals;

    for (size_t i = lookback; i < candles.size(); ++i)
    {
        KLine kline(candles[i]);
        double macd_score = 0.0, kd_score = 0.0, rsi_score = 0.0;
        int indicator_count = 0;

        // MACD Crossover (Weight: 0.4)
        if (i >= config.ema_slow - 1)
        {
            MACDResult macd = macd_cache[i];
            MACDResult macd_prev = (i > config.ema_slow - 1) ? macd_cache[i - 1] : MACDResult(0, 0, 0);
            if (std::isfinite(macd.macdLine) && std::isfinite(macd.signalLine) &&
                std::isfinite(macd_prev.macdLine) && std::isfinite(macd_prev.signalLine))
            {
                if (macd.macdLine > macd.signalLine && macd_prev.macdLine <= macd_prev.signalLine)
                {
                    macd_score = 0.4; // Bullish
                    indicator_count++;
                }
                else if (macd.macdLine < macd.signalLine && macd_prev.macdLine >= macd_prev.signalLine)
                {
                    macd_score = -0.4; // Bearish
                    indicator_count++;
                }
            }
        }

        // KD Crossover (Weight: 0.3)
        if (i >= 8)
        {
            auto kd = KDResult::stochasticKD(candles, 9, 3, i);
            auto kd_prev = KDResult::stochasticKD(candles, 9, 3, i - 1);
            double k = kd.getK(), d = kd.getD();
            double k_prev = kd_prev.getK(), d_prev = kd_prev.getD();
            if (std::isfinite(k) && std::isfinite(d) && std::isfinite(k_prev) && std::isfinite(d_prev) &&
                k >= 0 && k <= 100 && d >= 0 && d <= 100 && k_prev >= 0 && k_prev <= 100 && d_prev >= 0 && d_prev <= 100)
            {
                if (k > d && k_prev <= d_prev && k < 80)
                {
                    kd_score = 0.3; // Bullish
                    indicator_count++;
                }
                else if (k < d && k_prev >= d_prev && k > 20)
                {
                    kd_score = -0.3; // Bearish
                    indicator_count++;
                }
            }
        }

        // RSI Overbought/Oversold (Weight: 0.3)
        if (i >= 14)
        {
            double rsi = RSIResult::rsi(closes, 14, i, config);
            if (std::isfinite(rsi) && rsi >= 0 && rsi <= 100)
            {
                if (rsi > config.rsi_overbought)
                {                     // 70.0
                    rsi_score = -0.3; // Bearish
                    indicator_count++;
                }
                else if (rsi < config.rsi_oversold)
                {                    // 30.0
                    rsi_score = 0.3; // Bullish
                    indicator_count++;
                }
            }
        }

        // Determine signal and strength
        double total_score = macd_score + kd_score + rsi_score;
        std::string signal, strength;
        if (indicator_count >= 1)
        {
            signal = (total_score > 0) ? "買進" : "賣出";
            strength = (indicator_count >= 2 || std::abs(total_score) >= 0.7) ? "強" : "中";
            signals.emplace_back(i + 1, signal, strength);
        }
    }

    return signals;
}
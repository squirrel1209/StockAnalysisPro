#ifndef TRADESIGNAL_H
#define TRADESIGNAL_H

#include <vector>
#include <string>

struct SignalConfig
{
    double price_change_threshold = 0.005;
    int ema_fast = 12;
    int ema_slow = 26;
    int signal_period = 9;
    double rsi_overbought = 70.0;
    double rsi_oversold = 30.0;
};

class TradeSignal
{
public:
    TradeSignal(int date, const std::string &signal, const std::string &strength);
    int getDate() const;
    std::string getSignal() const;
    std::string getStrength() const;

private:
    int date;
    std::string signal;
    std::string strength;
};

class Candle;
class KLine;
struct MACDResult;

std::vector<TradeSignal> generateTradeSignals(
    const std::vector<Candle> &candles,
    const std::vector<double> &closes,
    const SignalConfig &config,
    int lookback,
    const std::vector<MACDResult> &macd_cache);

#endif
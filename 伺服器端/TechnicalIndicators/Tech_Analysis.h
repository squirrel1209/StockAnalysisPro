#ifndef TECH_ANALYSIS_H
#define TECH_ANALYSIS_H

#include <vector>
#include "TradeSignal.h" // 包含 SignalConfig 定義

class Candle;

struct MACDResult
{
    double macdLine, signalLine, histogram;
    MACDResult(double m, double s, double h) : macdLine(m), signalLine(s), histogram(h) {}
    static MACDResult macd(const std::vector<double> &data, int index, const SignalConfig &config);
};

struct KDResult
{
    double k, d;
    KDResult(double k, double d) : k(k), d(d) {}
    double getK() const { return k; }
    double getD() const { return d; }
    static KDResult stochasticKD(const std::vector<Candle> &candles, int period, int sma_period, int index);
};

struct RSIResult
{
    static double rsi(const std::vector<double> &data, int period, int index, const SignalConfig &config);
};

class Tech_Analysis
{
public:
    static double movingAverage(const std::vector<double> &data, int period, int index);
};

#endif
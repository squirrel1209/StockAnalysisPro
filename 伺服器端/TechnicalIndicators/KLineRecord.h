#ifndef KLINE_RECORD_H
#define KLINE_RECORD_H
#include <string>
#include "KLine.h"

class KLineRecord
{
public:
    std::string date;
    double open, high, low, close;
    double volume;
    double ma5, ma10, ma20;
    double k, d;
    double rsi;
    double macd_line, signal_line, histogram;
    std::string signal, strength;
    double price_change_percent;

    KLineRecord(
        const std::string &d,
        const KLine &k,
        double m5, double m10, double m20,
        double k_val, double d_val,
        double r,
        double macd_l, double sig_l, double hist,
        const std::string &sig,
        const std::string &str);
};

#endif
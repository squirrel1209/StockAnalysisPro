#ifndef KLINE_H
#define KLINE_H
#include <string>

class TradingSystem;
class KLineRecord;
class DataProcessor;

class Candle
{
private:
    double open, high, low, close;
    int volume;
    double price_change_percent;

    friend class TradingSystem;
    friend class KLineRecord;
    friend class DataProcessor;

public:
    Candle();
    Candle(double o, double h, double l, double c, int v, double pcp = 0.0);
    Candle(const Candle &other) = default;
    bool isValid() const;

    double getOpen() const { return open; }
    double getHigh() const { return high; }
    double getLow() const { return low; }
    double getClose() const { return close; }
    int getVolume() const { return volume; }
    double getPriceChangePercent() const { return price_change_percent; }
    void setPriceChangePercent(double pcp);
};

class KLine : public Candle
{
public:
    KLine();
    explicit KLine(const Candle &candle);
    bool isBullish() const;
    double body() const;
    double upperShadow() const;
    double lowerShadow() const;
};

#endif
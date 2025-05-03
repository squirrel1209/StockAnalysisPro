#ifndef TRADING_SYSTEM_H
#define TRADING_SYSTEM_H
#include <vector>
#include "KLine.h"

class TradingSystem
{
private:
    std::vector<Candle> candles;

public:
    void placeOrder(bool isBuy, const Candle &candle, double quantity);
    void completeCandle();
    std::vector<Candle> getCandles() const; // 修正：返回 std::vector<Candle>
};

#endif
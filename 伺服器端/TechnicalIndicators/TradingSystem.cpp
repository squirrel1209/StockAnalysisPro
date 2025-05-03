#include "TradingSystem.h"

void TradingSystem::placeOrder(bool isBuy, const Candle &candle, double quantity)
{
    candles.push_back(candle);
}

void TradingSystem::completeCandle()
{
    // 假設無需額外邏輯
}

std::vector<Candle> TradingSystem::getCandles() const
{
    return candles; // 修正：返回 candles 向量
}
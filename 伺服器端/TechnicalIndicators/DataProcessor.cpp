#include "DataProcessor.h"
#include <algorithm>

std::vector<Candle> DataProcessor::removeDuplicateCandles(const std::vector<Candle> &candles)
{
    std::vector<Candle> unique_candles = candles;
    auto last = std::unique(unique_candles.begin(), unique_candles.end(),
                            [](const Candle &a, const Candle &b)
                            {
                                return a.getOpen() == b.getOpen() &&
                                       a.getHigh() == b.getHigh() &&
                                       a.getLow() == b.getLow() &&
                                       a.getClose() == b.getClose() &&
                                       a.getVolume() == b.getVolume();
                            });
    unique_candles.erase(last, unique_candles.end());
    return unique_candles;
}

void DataProcessor::calculatePriceChanges(std::vector<Candle> &candles)
{
    for (size_t i = 1; i < candles.size(); ++i)
    {
        double prev_close = candles[i - 1].getClose();
        if (prev_close > 0)
        { // 防止除零
            double change = (candles[i].getClose() - prev_close) / prev_close * 100.0;
            candles[i].setPriceChangePercent(change);
        }
        else
        {
            candles[i].setPriceChangePercent(0.0);
        }
    }
    if (!candles.empty())
    {
        candles[0].setPriceChangePercent(0.0); // 第一根K線無前一天數據
    }
}
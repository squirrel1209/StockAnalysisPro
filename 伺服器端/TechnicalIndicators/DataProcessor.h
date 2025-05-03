#ifndef DATA_PROCESSOR_H
#define DATA_PROCESSOR_H
#include <vector>
#include "KLine.h"

class DataProcessor
{
public:
    static std::vector<Candle> removeDuplicateCandles(const std::vector<Candle> &candles); // 修正：const 參數
    static void calculatePriceChanges(std::vector<Candle> &candles);
};

#endif
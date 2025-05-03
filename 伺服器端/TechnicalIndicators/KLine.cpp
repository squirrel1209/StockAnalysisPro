#include "KLine.h"
#include <algorithm>
#include <iostream>

Candle::Candle() : open(0), high(0), low(0), close(0), volume(0), price_change_percent(0) {}

Candle::Candle(double o, double h, double l, double c, int v, double pcp)
    : open(o), high(h), low(l), close(c), volume(v), price_change_percent(pcp)
{
    if (!isValid())
    {
        std::cerr << "Invalid candle data: resetting to default values\n";
        open = high = low = close = 1.0; // 非零默認值
        volume = 0;
        price_change_percent = 0.0;
    }
}

bool Candle::isValid() const
{
    return open > 0 && high > 0 && low > 0 && close > 0 && high >= low &&
           high >= open && high >= close && low <= open && low <= close &&
           volume >= 0;
}

void Candle::setPriceChangePercent(double pcp)
{
    price_change_percent = pcp;
}

KLine::KLine() : Candle() {}

KLine::KLine(const Candle &candle) : Candle(candle) {}

bool KLine::isBullish() const
{
    return getClose() > getOpen();
}

double KLine::body() const
{
    return std::abs(getClose() - getOpen());
}

double KLine::upperShadow() const
{
    return getHigh() - std::max(getOpen(), getClose());
}

double KLine::lowerShadow() const
{
    return std::min(getOpen(), getClose()) - getLow();
}
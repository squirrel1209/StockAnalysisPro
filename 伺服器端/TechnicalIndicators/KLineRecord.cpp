#include "KLineRecord.h"

KLineRecord::KLineRecord(
    const std::string &d,
    const KLine &k,
    double m5, double m10, double m20,
    double k_val, double d_val,
    double r,
    double macd_l, double sig_l, double hist,
    const std::string &sig,
    const std::string &str)
    : date(d),
      open(k.getOpen()),
      high(k.getHigh()),
      low(k.getLow()),
      close(k.getClose()),
      volume(k.getVolume()),
      ma5(m5),
      ma10(m10),
      ma20(m20),
      k(k_val),
      d(d_val),
      rsi(r),
      macd_line(macd_l),
      signal_line(sig_l),
      histogram(hist),
      signal(sig),
      strength(str),
      price_change_percent(k.getPriceChangePercent())
{
}
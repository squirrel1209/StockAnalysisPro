#ifndef STOCKDATAMANAGER_H
#define STOCKDATAMANAGER_H

#include <QString>
#include <QDate>
#include <QVector>
#include <QMap>

// 單日股票數據結構
struct DailyStockData {
    QDate date;      // 日期
    double open;     // 開盤價
    double high;     // 最高價
    double low;      // 最低價
    double close;    // 收盤價
    int volume;      // 成交量
};

// 股票元數據結構
struct StockMetaData {
    QString symbol;  // 股票代碼
    QString lastRefreshed; // 最後更新日期
    QString timeZone;      // 時區
};

// 單支股票的管理類
class SingleStockDataManager {
public:
    SingleStockDataManager();

    // 添加元數據
    void setMetaData(const StockMetaData &meta);

    // 添加單日數據
    void addDailyData(const QDate &date, const DailyStockData &data);

    // 獲取元數據
    StockMetaData getMetaData() const;

    // 獲取所有日期（按日期排序）
    QVector<QDate> getDates() const;

    // 根據日期獲取單日數據
    DailyStockData getDailyData(const QDate &date) const;

    // 獲取所有每日數據（按日期排序）
    QVector<DailyStockData> getAllDailyData() const;

private:
    StockMetaData metaData; // 元數據
    QMap<QDate, DailyStockData> dailyData; // 每日數據（使用 QMap 按日期存儲）
};

// 存放和管理股票數據的類
class StockDataManager
{
public:
    StockDataManager();

    // 添加一支股票的數據
    void addStockData(const QString &symbol, const SingleStockDataManager &stockData);

    // 獲取所有股票代碼
    QVector<QString> getStockSymbols() const;

    // 根據股票代碼獲取數據
    SingleStockDataManager getStockData(const QString &symbol) const;

    // 獲取所有股票的最新數據（每支股票的最新一天數據）
    QVector<QPair<QString, DailyStockData>> getLatestDataForAllStocks() const;

private:
    QMap<QString, SingleStockDataManager> stockDataMap; // 按股票代碼存儲數據
};

#endif // STOCKDATAMANAGER_H

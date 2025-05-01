#include "stockdatamanager.h"

#include <algorithm>

// SingleStockDataManager 的實現
SingleStockDataManager::SingleStockDataManager() {}

void SingleStockDataManager::setMetaData(const StockMetaData &meta) {
    metaData = meta;
}

void SingleStockDataManager::addDailyData(const QDate &date, const DailyStockData &data) {
    dailyData[date] = data;
}

StockMetaData SingleStockDataManager::getMetaData() const {
    return metaData;
}

QVector<QDate> SingleStockDataManager::getDates() const {
    QVector<QDate> dates = dailyData.keys().toVector();
    std::sort(dates.begin(), dates.end(), std::greater<QDate>()); // 按日期降序排序
    return dates;
}

DailyStockData SingleStockDataManager::getDailyData(const QDate &date) const {
    return dailyData.value(date);
}

QVector<DailyStockData> SingleStockDataManager::getAllDailyData() const {
    QVector<QDate> dates = getDates();
    QVector<DailyStockData> data;
    for (const QDate &date : dates) {
        data.append(dailyData.value(date));
    }
    return data;
}

// StockDataManager 的實現
StockDataManager::StockDataManager() {}

void StockDataManager::addStockData(const QString &symbol, const SingleStockDataManager &stockData) {
    stockDataMap[symbol] = stockData;
}

QVector<QString> StockDataManager::getStockSymbols() const {
    return stockDataMap.keys().toVector();
}

SingleStockDataManager StockDataManager::getStockData(const QString &symbol) const {
    return stockDataMap.value(symbol);
}

QVector<QPair<QString, DailyStockData>> StockDataManager::getLatestDataForAllStocks() const {
    QVector<QPair<QString, DailyStockData>> latestData;
    for (const QString &symbol : stockDataMap.keys()) {
        SingleStockDataManager stockData = stockDataMap.value(symbol);
        QVector<QDate> dates = stockData.getDates();
        if (!dates.isEmpty()) {
            DailyStockData latest = stockData.getDailyData(dates.first()); // 獲取最新日期的數據
            latestData.append(qMakePair(symbol, latest));
        }
    }
    return latestData;
}

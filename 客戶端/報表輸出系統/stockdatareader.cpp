#include "stockdatareader.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDate>

bool StockDataReader::parseJsonString(const QString &jsonString, SingleStockDataManager &dataManager) {
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());
    if (doc.isNull() || !doc.isObject()) {
        return false; // 無效的 JSON
    }

    QJsonObject jsonObj = doc.object();

    // 解析元數據
    StockMetaData metaData;
    QJsonObject metaJson = jsonObj["Meta Data"].toObject();
    metaData.symbol = metaJson["2. Symbol"].toString();
    metaData.lastRefreshed = metaJson["3. Last Refreshed"].toString();
    metaData.timeZone = metaJson["4. Time Zone"].toString();
    dataManager.setMetaData(metaData);

    // 解析每日數據
    QJsonObject dailyJson = jsonObj["Time Series (Daily)"].toObject();
    for (const QString &dateStr : dailyJson.keys()) {
        QDate date = QDate::fromString(dateStr, "yyyy-MM-dd");
        if (!date.isValid()) continue;

        QJsonObject dailyDataJson = dailyJson[dateStr].toObject();
        DailyStockData dailyData;
        dailyData.date = date;
        dailyData.open = dailyDataJson["1. open"].toString().toDouble();
        dailyData.high = dailyDataJson["2. high"].toString().toDouble();
        dailyData.low = dailyDataJson["3. low"].toString().toDouble();
        dailyData.close = dailyDataJson["4. close"].toString().toDouble();
        dailyData.volume = dailyDataJson["5. volume"].toString().toInt();

        dataManager.addDailyData(date, dailyData);
    }

    return true;
}

bool StockDataReader::parseMultipleJsonString(const QString &jsonString, StockDataManager &dataManager) {
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());
    if (doc.isNull() || !doc.isArray()) {
        return false; // 假設多支股票的 JSON 是陣列
    }

    QJsonArray jsonArray = doc.array();
    for (const QJsonValue &value : jsonArray) {
        SingleStockDataManager singleStockData;
        QString singleJsonString = QJsonDocument(value.toObject()).toJson();
        if (parseJsonString(singleJsonString, singleStockData)) {
            dataManager.addStockData(singleStockData.getMetaData().symbol, singleStockData);
        }
    }

    return true;
}
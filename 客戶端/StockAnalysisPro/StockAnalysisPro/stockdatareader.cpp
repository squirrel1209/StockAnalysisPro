#include "stockdatareader.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDebug>

StockDataReader::StockDataReader() {}

bool StockDataReader::readJsonFile(const QString &filePath, SingleStockDataManager &dataManager) {
    // 打開檔案
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file:" << filePath;
        return false;
    }

    // 讀取檔案內容並解析 JSON
    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull() || !doc.isObject()) {
        qDebug() << "Invalid JSON format in file:" << filePath;
        return false;
    }

    QJsonObject jsonObj = doc.object();

    // 解析元數據
    QJsonObject metaDataObj = jsonObj["Meta Data"].toObject();
    StockMetaData metaData;
    metaData.symbol = metaDataObj["2. Symbol"].toString();
    metaData.lastRefreshed = metaDataObj["3. Last Refreshed"].toString();
    metaData.timeZone = metaDataObj["5. Time Zone"].toString();
    dataManager.setMetaData(metaData);

    // 解析每日數據
    QJsonObject timeSeriesObj = jsonObj["Time Series (Daily)"].toObject();
    for (const QString &dateStr : timeSeriesObj.keys()) {
        QDate date = QDate::fromString(dateStr, "yyyy-MM-dd");
        if (!date.isValid()) {
            qDebug() << "Invalid date format:" << dateStr;
            continue;
        }

        QJsonObject dailyObj = timeSeriesObj[dateStr].toObject();
        DailyStockData dailyData;
        dailyData.date = date;
        dailyData.open = dailyObj["1. open"].toString().toDouble();
        dailyData.high = dailyObj["2. high"].toString().toDouble();
        dailyData.low = dailyObj["3. low"].toString().toDouble();
        dailyData.close = dailyObj["4. close"].toString().toDouble();
        dailyData.volume = dailyObj["5. volume"].toString().toDouble();
        // 解析新字段
        dailyData.ma5 = dailyObj["6. ma5"].toString().toDouble();
        dailyData.ma10 = dailyObj["7. ma10"].toString().toDouble();
        dailyData.ma20 = dailyObj["8. ma20"].toString().toDouble();
        dailyData.k = dailyObj["9. k"].toString().toDouble();
        dailyData.d = dailyObj["10. d"].toString().toDouble();
        dailyData.rsi = dailyObj["11. rsi"].toString().toDouble();
        dailyData.macd_line = dailyObj["12. macd_line"].toString().toDouble();
        dailyData.signal_line = dailyObj["13. signal_line"].toString().toDouble();
        dailyData.histogram = dailyObj["14. histogram"].toString().toDouble();
        dailyData.price_change_percent = dailyObj["15. price_change_percent"].toString().toDouble();
        dailyData.signal = dailyObj["16. signal"].toString();
        dailyData.strength = dailyObj["17. strength"].toString();
        dailyData.body_size = dailyObj["18. body_size"].toString().toDouble();
        dailyData.body_type = dailyObj["19. body_type"].toString();
        dailyData.upper_shadow = dailyObj["20. upper_shadow"].toString().toDouble();
        dailyData.lower_shadow = dailyObj["21. lower_shadow"].toString().toDouble();

        dataManager.addDailyData(date, dailyData);
    }
    return true;
}

bool StockDataReader::readMultipleJsonFiles(const QVector<QString> &filePaths, StockDataManager &dataManager) {
    bool success = true;
    for (const QString &filePath : filePaths) {
        SingleStockDataManager singleStockData;
        if (readJsonFile(filePath, singleStockData)) {
            QString symbol = singleStockData.getMetaData().symbol;
            dataManager.addStockData(symbol, singleStockData);
            qDebug() << "Successfully loaded stock data for" << symbol;
        } else {
            success = false;
            qDebug() << "Failed to load stock data from" << filePath;
        }
    }
    return success;
}

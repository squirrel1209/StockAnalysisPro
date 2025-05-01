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
        dailyData.volume = dailyObj["5. volume"].toString().toInt();

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

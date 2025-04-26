#include "stockdataparser.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>  // 用來打 log

SingleStockDataManager StockDataParser::parseSingleStock(const QString& jsonString)
{
    SingleStockDataManager manager;

    // 解析 JSON 字串
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());
    if (!doc.isObject()) {
        qWarning() << "[StockDataParser] JSON 格式錯誤，無法解析！";
        return manager;
    }

    QJsonObject obj = doc.object();

    // 解析 Meta Data 區塊
    if (obj.contains("Meta Data")) {
        QJsonObject metaObj = obj["Meta Data"].toObject();
        StockMetaData meta;
        meta.symbol = metaObj.value("2. Symbol").toString();
        meta.lastRefreshed = metaObj.value("3. Last Refreshed").toString();
        meta.timeZone = metaObj.value("5. Time Zone").toString();

        manager.setMetaData(meta);

        qDebug() << "[StockDataParser] 成功解析 MetaData:"
                 << "Symbol =" << meta.symbol
                 << ", Last Refreshed =" << meta.lastRefreshed
                 << ", TimeZone =" << meta.timeZone;
    } else {
        qWarning() << "[StockDataParser] 缺少 Meta Data 欄位！";
    }

    // 解析 Time Series (Daily) 區塊
    if (obj.contains("Time Series (Daily)")) {
        QJsonObject timeSeriesObj = obj["Time Series (Daily)"].toObject();
        int count = 0;
        for (auto it = timeSeriesObj.begin(); it != timeSeriesObj.end(); ++it) {
            QDate date = QDate::fromString(it.key(), "yyyy-MM-dd");
            if (!date.isValid()) {
                qWarning() << "[StockDataParser] 日期格式錯誤:" << it.key();
                continue;
            }

            QJsonObject dayData = it.value().toObject();
            DailyStockData data;
            data.date = date;
            data.open = dayData.value("1. open").toString().toDouble();
            data.high = dayData.value("2. high").toString().toDouble();
            data.low = dayData.value("3. low").toString().toDouble();
            data.close = dayData.value("4. close").toString().toDouble();
            data.volume = dayData.value("5. volume").toString().toInt();

            manager.addDailyData(date, data);
            count++;
        }
        qDebug() << "[StockDataParser] 成功解析" << count << "筆每日資料。";
    } else {
        qWarning() << "[StockDataParser] 缺少 Time Series (Daily) 欄位！";
    }

    return manager;
}

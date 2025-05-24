#include "stockdatasocketreceiver.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QDataStream>

StockDataSocketReceiver::StockDataSocketReceiver(QObject *parent) : QObject(parent)
{
    socket = new QTcpSocket(this);
    reconnectTimer = new QTimer(this);
    connect(socket, &QTcpSocket::readyRead, this, &StockDataSocketReceiver::onReadyRead);
    connect(socket, &QTcpSocket::connected, this, &StockDataSocketReceiver::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &StockDataSocketReceiver::onDisconnected);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, &StockDataSocketReceiver::onErrorOccurred);
    connect(reconnectTimer, &QTimer::timeout, this, &StockDataSocketReceiver::tryReconnect);
}

bool StockDataSocketReceiver::connectToServer(const QString &host, quint16 port)
{
    this->host = host;
    this->port = port;
    socket->connectToHost(host, port);
    return socket->waitForConnected(5000); // 等待 5 秒
}

void StockDataSocketReceiver::disconnectFromServer()
{
    reconnectTimer->stop();
    socket->disconnectFromHost();
}

bool StockDataSocketReceiver::receiveJsonData(const QByteArray &jsonData, SingleStockDataManager &dataManager)
{
    return parseJsonData(jsonData, dataManager);
}

bool StockDataSocketReceiver::receiveMultipleJsonData(StockDataManager &dataManager)
{
    QMutexLocker locker(&dataMutex);
    QString bufferStr = QString(buffer);
    qDebug() << "Raw buffer data:" << bufferStr.left(200) << "...";

    // 清除 "JSON|" 前綴（如果存在）
    QString cleanedJson = bufferStr.startsWith("JSON|") ? bufferStr.mid(5) : bufferStr;
    qDebug() << "Cleaned JSON data:" << cleanedJson.left(200) << "...";

    QJsonDocument doc = QJsonDocument::fromJson(cleanedJson.toUtf8());
    if (doc.isNull() || !doc.isArray()) {
        qDebug() << "No valid JSON array start found in buffer";
        return false;
    }

    // 繼續解析邏輯
    bool success = parseJsonArray(cleanedJson.toUtf8(), dataManager);
    return success;
}

bool StockDataSocketReceiver::parseJsonData(const QByteArray &jsonData, SingleStockDataManager &dataManager)
{
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull() || !doc.isObject()) {
        qDebug() << "Invalid JSON format. Data:" << QString(jsonData).left(100) << "...";
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

bool StockDataSocketReceiver::parseJsonArray(const QByteArray &jsonData, StockDataManager &dataManager)
{
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull() || !doc.isArray()) {
        qDebug() << "Invalid JSON array format. Data:" << QString(jsonData).left(100) << "...";
        return false;
    }

    QJsonArray jsonArray = doc.array();
    bool success = true;

    for (const QJsonValue &value : jsonArray) {
        if (!value.isObject()) {
            qDebug() << "Invalid JSON object in array";
            success = false;
            continue;
        }

        SingleStockDataManager singleStockData;
        QJsonDocument singleDoc(value.toObject());
        if (receiveJsonData(singleDoc.toJson(), singleStockData)) {
            QString symbol = singleStockData.getMetaData().symbol;
            dataManager.addStockData(symbol, singleStockData);
            qDebug() << "Successfully parsed stock data for" << symbol << "from array";
        } else {
            qDebug() << "Failed to parse JSON object in array";
            success = false;
        }
    }

    return success;
}

void StockDataSocketReceiver::onReadyRead()
{
    static quint32 expectedDataLen = 0;
    static QByteArray tempBuffer;

    QDataStream stream(socket);
    stream.setVersion(QDataStream::Qt_5_15);

    while (socket->bytesAvailable() > 0) {
        // 如果尚未讀取數據長度，嘗試讀取
        if (expectedDataLen == 0 && socket->bytesAvailable() >= sizeof(quint32)) {
            stream.startTransaction();
            stream >> expectedDataLen;
            if (!stream.commitTransaction()) {
                qDebug() << "Failed to read data length, waiting for more data...";
                return;
            }

            // 驗證數據長度
            if (expectedDataLen == 0 || expectedDataLen > 1000000) {
                qDebug() << "Invalid data length:" << expectedDataLen << ", clearing buffer and skipping...";
                expectedDataLen = 0;
                tempBuffer.clear();
                socket->readAll();
                continue;
            }

            qDebug() << "Expecting data length:" << expectedDataLen;
            tempBuffer.clear();
        }

        // 檢查是否有足夠數據
        if (expectedDataLen > 0 && socket->bytesAvailable() >= expectedDataLen - tempBuffer.size()) {
            QByteArray chunk = socket->read(expectedDataLen - tempBuffer.size());
            tempBuffer.append(chunk);

            if (tempBuffer.size() == expectedDataLen) {
                qDebug() << "Received complete data (length:" << expectedDataLen << "):" << QString(tempBuffer).left(200) << "...";

                buffer.append(tempBuffer);
                StockDataManager dataManager;
                if (receiveMultipleJsonData(dataManager)) {
                    qDebug() << "Successfully processed JSON data";
                    emit dataReceived();
                } else {
                    qDebug() << "Failed to process JSON data";
                }

                expectedDataLen = 0;
                tempBuffer.clear();
                buffer.clear();
            } else {
                qDebug() << "Incomplete data, waiting for more... Expected:" << expectedDataLen << "Received:" << tempBuffer.size();
                return;
            }
        } else if (expectedDataLen > 0) {
            QByteArray chunk = socket->readAll();
            tempBuffer.append(chunk);
            qDebug() << "Partial data received, total:" << tempBuffer.size() << "of" << expectedDataLen;
            return;
        } else {
            // 處理無效或空數據
            qDebug() << "No valid data length expected, clearing socket buffer...";
            socket->readAll();
            return;
        }
    }
}

void StockDataSocketReceiver::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
    QString errorMsg;
    switch (socketError) {
    case QAbstractSocket::ConnectionRefusedError:
        errorMsg = "Connection refused by the server";
        break;
    case QAbstractSocket::HostNotFoundError:
        errorMsg = "Host not found";
        break;
    case QAbstractSocket::SocketTimeoutError:
        errorMsg = "Socket operation timed out";
        break;
    case QAbstractSocket::RemoteHostClosedError:
        errorMsg = "The remote host closed the connection";
        buffer.clear(); // 清空緩衝區
        reconnectTimer->start(3000); // 3 秒後重連
        break;
    default:
        errorMsg = socket->errorString();
        break;
    }
    qDebug() << "Socket error:" << errorMsg;
    emit errorOccurred(errorMsg);
}

void StockDataSocketReceiver::onConnected()
{
    qDebug() << "Connected to server";
    reconnectTimer->stop();
    buffer.clear(); // 清空緩衝區，準備接收新數據
}

void StockDataSocketReceiver::onDisconnected()
{
    qDebug() << "Disconnected from server";
    buffer.clear(); // 清空緩衝區
    reconnectTimer->start(3000);
}

void StockDataSocketReceiver::tryReconnect()
{
    qDebug() << "Attempting to reconnect to server...";
    socket->connectToHost(host, port);
}

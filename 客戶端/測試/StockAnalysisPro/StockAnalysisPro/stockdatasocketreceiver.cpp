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
    bool success = true;

    // 記錄原始數據
    qDebug() << "Raw buffer data:" << QString(buffer).left(200) << "...";

    // 移除前綴（JSON|）
    QString bufferStr = QString(buffer);
    int jsonStart = bufferStr.indexOf('['); // JSON 陣列以 [ 開頭
    if (jsonStart == -1) {
        qDebug() << "No valid JSON array start found in buffer";
        return false;
    }

    QString cleanedJson = bufferStr.mid(jsonStart);
    qDebug() << "Cleaned JSON data:" << cleanedJson.left(200) << "...";

    // 解析 JSON 陣列
    if (!parseJsonArray(cleanedJson.toUtf8(), dataManager)) {
        qDebug() << "Failed to parse JSON array:" << cleanedJson.left(100) << "...";
        success = false;
    } else {
        qDebug() << "Successfully processed JSON array";
    }

    // 清空緩衝區
    buffer.clear();
    qDebug() << "Buffer cleared after processing";

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
        dailyData.volume = dailyObj["5. volume"].toString().toInt();

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
    QDataStream stream(socket);
    stream.setVersion(QDataStream::Qt_5_15); // 設置數據流版本

    while (socket->bytesAvailable() >= sizeof(quint32)) {
        // 讀取數據長度（4 字節）
        quint32 dataLen;
        stream.startTransaction();
        stream >> dataLen;

        if (!stream.commitTransaction()) {
            qDebug() << "Failed to read data length";
            break;
        }

        // 驗證數據長度是否合理
        if (dataLen > 1000000) { // 假設 JSON 陣列不超過 1MB
            qDebug() << "Invalid data length:" << dataLen << ", skipping...";
            buffer.clear(); // 清空緩衝區以防錯位
            break;
        }

        // 等待接收完整數據
        if (socket->bytesAvailable() < dataLen) {
            qDebug() << "Incomplete data, waiting for more... Expected:" << dataLen << "Available:" << socket->bytesAvailable();
            break;
        }

        // 讀取數據
        QByteArray data = socket->read(dataLen);
        buffer.append(data);

        // 記錄接收到的原始數據
        qDebug() << "Received raw data (length:" << dataLen << "):" << QString(data).left(200) << "...";
    }

    // 處理緩衝區中的數據
    if (!buffer.isEmpty()) {
        emit dataReceived();
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
    reconnectTimer->start(3000); // 3 秒後嘗試重連
}

void StockDataSocketReceiver::tryReconnect()
{
    qDebug() << "Attempting to reconnect to server...";
    socket->connectToHost(host, port);
}

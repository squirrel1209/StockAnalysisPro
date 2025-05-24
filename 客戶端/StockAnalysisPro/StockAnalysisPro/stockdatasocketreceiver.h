#ifndef STOCKDATASOCKETRECEIVER_H
#define STOCKDATASOCKETRECEIVER_H

#include "stockdatamanager.h"
#include <QTcpSocket>
#include <QString>
#include <QObject>
#include <QTimer>
#include <QMutex>

class StockDataSocketReceiver : public QObject
{
    Q_OBJECT
private:
    QMutex dataMutex;
public:
    explicit StockDataSocketReceiver(QObject *parent = nullptr);

    // 連接到後端伺服器
    bool connectToServer(const QString &host, quint16 port);

    // 接收單筆 JSON 數據並填充到 SingleStockDataManager
    bool receiveJsonData(const QByteArray &jsonData, SingleStockDataManager &dataManager);

    // 接收多筆 JSON 數據並填充到 StockDataManager
    bool receiveMultipleJsonData(StockDataManager &dataManager);

    // 斷開連線
    void disconnectFromServer();

signals:
    // 當接收到新數據時發出信號
    void dataReceived();
    // 當發生錯誤時發出信號
    void errorOccurred(const QString &error);

private slots:
    // 處理 socket 的 readyRead 信號
    void onReadyRead();
    // 處理 socket 錯誤
    void onErrorOccurred(QAbstractSocket::SocketError socketError);
    // 處理連線成功
    void onConnected();
    // 處理斷線
    void onDisconnected();
    // 嘗試重新連線
    void tryReconnect();

private:
    QTcpSocket *socket; // TCP socket
    QByteArray buffer; // 用於儲存接收到的數據
    QString host; // 伺服器主機
    quint16 port; // 伺服器端口
    QTimer *reconnectTimer; // 重連計時器
    bool parseJsonData(const QByteArray &jsonData, SingleStockDataManager &dataManager); // 解析單筆 JSON 數據
    bool parseJsonArray(const QByteArray &jsonData, StockDataManager &dataManager); // 解析 JSON 陣列
};

#endif // STOCKDATASOCKETRECEIVER_H

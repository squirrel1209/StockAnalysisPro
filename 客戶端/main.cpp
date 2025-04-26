#include <QCoreApplication>
#include <QDebug>
#include <QElapsedTimer> // ⭐ 加這個

#include "ClientApplication.h"
#include "JsonPacket.h"
#include "StockDataParser.h"
#include "StockDataManager.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    std::string server_ip = "127.0.0.1";  // 改成你的 Server IP
    int server_port = 8080;               // 改成你的 Server Port
    ClientApplication client(server_ip, server_port);

    StockDataManager stockManager;

    int packetCount = 0;
    QElapsedTimer timer;
    timer.start(); // ⭐ 開始計時

    const int timeoutMs = 10000; // ⭐ 10 秒超時（單位是毫秒）

    while (true) {
        // 檢查是否超過 10 秒沒收到資料
        if (timer.elapsed() > timeoutMs) {
            qWarning() << "[Main] 超過 10 秒無封包，停止接收。";
            break;
        }

        // 取得封包
        std::unique_ptr<PacketInterface> packet = client.fetchPacket();
        if (!packet) {
            qWarning() << "[Main] 沒有收到封包，結束接收。";
            break;
        }

        std::string jsonPayload = packet->getPayload();
        if (jsonPayload.empty()) {
            qWarning() << "[Main] 封包 Payload 為空，跳過。";
            continue;
        }

        qDebug() << "[Main] 收到封包，解析中...";

        // 成功收封包了，重新啟動計時器 ⭐
        timer.restart();

        // 解析
        SingleStockDataManager stockData = StockDataParser::parseSingleStock(QString::fromStdString(jsonPayload));

        // 加入資料庫
        stockManager.addStockData(stockData.getMetaData().symbol, stockData);

        packetCount++;
    }

    qDebug() << "[Main] 共收到" << packetCount << "個封包，解析完成。";

    // 顯示所有股票的資料
    QVector<QString> symbols = stockManager.getStockSymbols();
    qDebug() << "總共收到股票數:" << symbols.size();

    for (const QString& symbol : symbols) {
        SingleStockDataManager stockData = stockManager.getStockData(symbol);
        StockMetaData meta = stockData.getMetaData();
        QVector<QDate> dates = stockData.getDates();

        if (!dates.isEmpty()) {
            DailyStockData latest = stockData.getDailyData(dates.first());
            qDebug() << "------------------------";
            qDebug() << "股票代碼:" << meta.symbol;
            qDebug() << "最新交易日:" << latest.date.toString("yyyy-MM-dd");
            qDebug() << "開盤:" << latest.open
                     << "最高:" << latest.high
                     << "最低:" << latest.low
                     << "收盤:" << latest.close
                     << "成交量:" << latest.volume;
        }
    }

    return 0;
}

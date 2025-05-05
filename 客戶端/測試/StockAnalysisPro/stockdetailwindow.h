#ifndef STOCKDETAILWINDOW_H
#define STOCKDETAILWINDOW_H

#include <QDialog>
#include <QTableWidget>
#include <QTabWidget>
#include <QLabel>
#include "stockdatamanager.h"
#include "qcustomplot.h"

class StockDetailWindow : public QDialog
{
    Q_OBJECT
public:
    StockDetailWindow(const QString &symbol, const SingleStockDataManager &stockData, QWidget *parent = nullptr);

protected:
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QTabWidget *tabWidget; // 分頁控件
    QTableWidget *detailTable; // 股票詳情表格
    QCustomPlot *technicalPlot; // 技術詳情圖表
    QLabel *dataLabel; // 用於顯示開盤、低點、高點、收盤、成交量
    QVector<DailyStockData> dailyData; // 儲存股票數據
    QVector<double> timestamps; // 儲存每個 K 線的時間戳
    QCPItemLine *horizontalLinePrice; // 價格水平線（K 線圖）
    QCPItemLine *verticalLinePrice;   // 日期垂直線（K 線圖）
    QCPItemLine *horizontalLineVolume; // 成交量水平線（成交量圖）
    QCPItemLine *verticalLineVolume;   // 日期垂直線（成交量圖）
    QCPAxisRect *priceAxisRect;  // K 線圖區域
    QCPAxisRect *volumeAxisRect; // 成交量圖區域
    QCPAxisRect *rsiAxisRect;    // RSI 圖區域
    QCPAxisRect *macdAxisRect;   // MACD 圖區域
    QCPItemLine *horizontalLineRsi; // RSI 十字線（水平）
    QCPItemLine *verticalLineRsi;   // RSI 十字線（垂直）
    QCPItemLine *horizontalLineMacd; // MACD 十字線（水平）
    QCPItemLine *verticalLineMacd;   // MACD 十字線（垂直）

    void updateCrosshair(double x, double y, bool inPricePlot, bool inRsiPlot, bool inMacdPlot);
    void updateDataDisplay(double timestamp);
    void setupStockDetailTab(const QString &symbol, const SingleStockDataManager &stockData);
    void setupTechnicalDetailTab(const SingleStockDataManager &stockData);
    QString formatNumberWithCommas(double number); // 格式化數字，添加千位分號
    QVector<double> calculateSMA(const QVector<DailyStockData> &dailyData, int period);
};

#endif // STOCKDETAILWINDOW_H

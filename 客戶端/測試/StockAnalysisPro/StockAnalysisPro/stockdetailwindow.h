#ifndef STOCKDETAILWINDOW_H
#define STOCKDETAILWINDOW_H

#include <QDialog>
#include <QTableWidget>
#include "stockdatamanager.h"
#include "qcustomplot.h"

class StockDetailWindow : public QDialog
{
    Q_OBJECT
public:
    StockDetailWindow(const QString &symbol, const SingleStockDataManager &stockData, QWidget *parent = nullptr);
private:
    QTabWidget *tabWidget; // 分頁控件
    QTableWidget *detailTable; // 股票詳情表格
    QCustomPlot *technicalPlot; // 技術詳情圖表
    void setupStockDetailTab(const QString &symbol, const SingleStockDataManager &stockData);
    void setupTechnicalDetailTab(const SingleStockDataManager &stockData);
    QString formatNumberWithCommas(double number); // 格式化數字，添加千位分號
    QVector<double> calculateSMA(const QVector<DailyStockData> &dailyData, int period);
};

#endif // STOCKDETAILWINDOW_H

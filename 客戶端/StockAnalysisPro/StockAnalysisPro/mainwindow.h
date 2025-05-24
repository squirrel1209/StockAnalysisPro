#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QVector>
#include <QDate>
#include "stockdatamanager.h"
#include "stockdatasocketreceiver.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

struct StockData {
    QString name;       // 股票名稱
    double price;       // 價格
    double buyPrice;    // 買價
    double sellPrice;   // 賣價
    double change;      // 漲跌
    double changePercent; // 漲跌%
    int volume;         // 成交量
    int totalVolume;    // 總量
    QDate date;         // 日期
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void wheelEvent(QWheelEvent *event) override; // 重寫滾輪事件

private slots:
    void onStockTableDoubleClicked(const QModelIndex &index);
    void onHeaderClicked(int column); // 處理列標頭點擊事件
    void onDataReceived(); // 處理接收到的 socket 數據
    void onSocketError(const QString &error); // 處理 socket 錯誤
    void openColumnSelectorDialog(); // 新增槽函數

private:
    Ui::MainWindow *ui;
    QVector<StockData> stockList;
    StockDataManager stockDataManager;
    StockDataSocketReceiver *socketReceiver; // socket 接收器

    qreal zoomFactor = 1.0; // 縮放比例，初始為 1.0
    const qreal zoomStep = 0.1; // 每次縮放的步長
    const qreal minZoom = 0.5; // 最小縮放比例
    const qreal maxZoom = 1.5; // 最大縮放比例
    int baseColumnWidth = 100; // 基礎欄寬
    int baseRowHeight = 30; // 基礎行高
    int baseFontSize = 10; // 基礎字體大小
    int baseHeaderFontSize = 14; // 標頭字體大小

    // 儲存初始數據以恢復預設排序
    QVector<QPair<QString, DailyStockData>> originalData;

    // 追蹤每個列的排序狀態
    QMap<int, int> sortStates; // 列索引 -> 排序狀態 (0: 預設, 1: 高到低, 2: 低到高)

    // 新增：用於追蹤所有欄位和可見狀態
    QStringList allColumns;
    QMap<QString, bool> visibleColumns;

    QString formatNumberWithCommas(double number); // 格式化數字，添加千位分號

    void setupStockTable(); //初始化表單
    void populateStockTable(); //表格資料
    void updateTableZoom(); // 更新表格的縮放
    void restoreDefaultSort(); // 恢復預設排序
    void sortTableByColumn(int column, Qt::SortOrder order);
};
#endif // MAINWINDOW_H

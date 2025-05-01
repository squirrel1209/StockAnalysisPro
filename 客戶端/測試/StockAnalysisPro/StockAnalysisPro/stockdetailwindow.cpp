#include "stockdetailwindow.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QFont>
#include <QDateTime>

StockDetailWindow::StockDetailWindow(const QString &symbol, const SingleStockDataManager &stockData, QWidget *parent)
    : QDialog(parent){
    // 設置窗口標題
    setWindowTitle(QString("股票詳細資訊 - %1").arg(symbol));

    // 設置窗口大小
    setMinimumSize(800, 600);

    // 設置背景為黑色
    setStyleSheet("QDialog { background-color: black; }");

    // 創建分頁控件
    tabWidget = new QTabWidget(this);
    tabWidget->setStyleSheet(
        "QTabWidget::pane { background-color: black; border: 1px solid gray; }"
        "QTabBar::tab { background-color: #1e90ff; color: white; padding: 5px; }"
        "QTabBar::tab:selected { background-color: #4682b4; }"
        );

    // 創建股票詳情分頁
    QWidget *stockDetailTab = new QWidget();
    detailTable = new QTableWidget(stockDetailTab);
    QVBoxLayout *stockDetailLayout = new QVBoxLayout(stockDetailTab);
    stockDetailLayout->addWidget(detailTable);
    stockDetailLayout->setContentsMargins(0, 0, 0, 0);
    stockDetailTab->setLayout(stockDetailLayout);
    tabWidget->addTab(stockDetailTab, "股票詳情");

    // 創建技術詳情分頁
    QWidget *technicalDetailTab = new QWidget();
    technicalPlot = new QCustomPlot(technicalDetailTab);
    QVBoxLayout *technicalDetailLayout = new QVBoxLayout(technicalDetailTab);
    technicalDetailLayout->addWidget(technicalPlot);
    technicalDetailLayout->setContentsMargins(0, 0, 0, 0);
    technicalDetailTab->setLayout(technicalDetailLayout);
    tabWidget->addTab(technicalDetailTab, "技術詳情");

    // 設置主佈局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(tabWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);

    // 設置股票詳情分頁
    setupStockDetailTab(symbol, stockData);

    // 設置技術詳情分頁
    setupTechnicalDetailTab(stockData);
}

void StockDetailWindow::setupStockDetailTab(const QString &symbol, const SingleStockDataManager &stockData) {
    // 設置表格欄位
    detailTable->setColumnCount(6);
    detailTable->setHorizontalHeaderLabels({
        "日期", "開盤價", "最高價", "最低價", "收盤價", "成交量"
    });

    // 設置表格樣式（與主視窗一致）
    detailTable->setStyleSheet(
        "QTableWidget { background-color: black; color: white; gridline-color: gray; }"
        "QTableWidget { border: 1 px solid gray; }" // 加粗表格邊線
        "QHeaderView::section { background-color: #1773cd; color: white; }" // 標頭邊線
        "QTableWidget::item { border: 1 px solid gray; }" // 每個單元格邊線
        "QTableWidget::item:selected { background-color: #4682b4; color: white; }"
        );

    // 設置表格屬性
    detailTable->setEditTriggers(QAbstractItemView::NoEditTriggers); // 禁止編輯
    detailTable->setSelectionBehavior(QAbstractItemView::SelectRows); // 選擇整行
    detailTable->setSelectionMode(QAbstractItemView::SingleSelection); // 單選

    // 使表格自動延伸到視窗邊界
    detailTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 允許使用者調整欄寬
    detailTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    // 允許使用者調整行高
    detailTable->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    detailTable->verticalHeader()->setVisible(true); // 顯示垂直標頭以便調整行高

    // 設置初始行高和欄寬
    detailTable->horizontalHeader()->setDefaultSectionSize(100);
    detailTable->verticalHeader()->setDefaultSectionSize(30);

    // 設置表格內容字體（粗體，更改字型）
    QFont contentFont("微軟正黑體", 12, QFont::Bold); // 表格內容字體
    detailTable->setFont(contentFont);

    // 設置標頭字體（粗體，更改字型，字體更大）
    QFont headerFont("微軟正黑體", 18, QFont::Bold); // 標頭字體
    detailTable->horizontalHeader()->setFont(headerFont);
    detailTable->verticalHeader()->setFont(headerFont);

    // 填充歷史數據
    QVector<DailyStockData> dailyData = stockData.getAllDailyData();
    detailTable->setRowCount(dailyData.size());

    for (int i = 0; i < dailyData.size(); ++i) {
        const DailyStockData &data = dailyData[i];
        detailTable->setItem(i, 0, new QTableWidgetItem(data.date.toString("yyyy-MM-dd"))); // 日期
        detailTable->setItem(i, 1, new QTableWidgetItem(QString::number(data.open, 'f', 2))); // 開盤價
        detailTable->setItem(i, 2, new QTableWidgetItem(QString::number(data.high, 'f', 2))); // 最高價
        detailTable->setItem(i, 3, new QTableWidgetItem(QString::number(data.low, 'f', 2))); // 最低價
        detailTable->setItem(i, 4, new QTableWidgetItem(QString::number(data.close, 'f', 2))); // 收盤價
        detailTable->setItem(i, 5, new QTableWidgetItem(formatNumberWithCommas(data.volume))); // 成交量
    }
}

void StockDetailWindow::setupTechnicalDetailTab(const SingleStockDataManager &stockData) {
    // 獲取歷史數據
    QVector<DailyStockData> dailyData = stockData.getAllDailyData();
    if (dailyData.isEmpty()) {
        return;
    }

    // 準備數據
    QVector<double> timestamps, opens, highs, lows, closes, volumes;
    for (int i = 0; i < dailyData.size(); ++i) {
        const DailyStockData &data = dailyData[i];
        timestamps.append(QDateTime(data.date,QTime(0, 0)).toSecsSinceEpoch());
        opens.append(data.open);
        highs.append(data.high);
        lows.append(data.low);
        closes.append(data.close);
        volumes.append(data.volume);
    }

    // 計算 SMA(5) 和 SMA(20)
    QVector<double> sma5 = calculateSMA(dailyData, 5);
    QVector<double> sma20 = calculateSMA(dailyData, 20);

    // 設置背景色
    technicalPlot->setBackground(QBrush(Qt::black));

    // 創建 K 線圖（上部）
    QCPFinancial *candlesticks = new QCPFinancial(technicalPlot->xAxis, technicalPlot->yAxis);
    candlesticks->setName("K 線");
    candlesticks->setChartStyle(QCPFinancial::csCandlestick);
    candlesticks->setData(timestamps, opens, highs, lows, closes);
    candlesticks->setWidth(0.6 * (timestamps[1] - timestamps[0]));// 綠色表示上漲，紅色表示下跌
    candlesticks->setBrushPositive(QColor(Qt::green));
    candlesticks->setBrushNegative(QColor(Qt::red));
    candlesticks->setPenPositive(QPen(QColor(Qt::green)));
    candlesticks->setPenNegative(QPen(QColor(Qt::red)));

    // 創建 SMA 線
    QCPGraph *sma5Graph = technicalPlot->addGraph();
    sma5Graph->setData(timestamps, sma5);
    sma5Graph->setPen(QPen(Qt::cyan));
    sma5Graph->setName("SMA(5)");

    QCPGraph *sma20Graph = technicalPlot->addGraph();
    sma20Graph->setData(timestamps, sma20);
    sma20Graph->setPen(QPen(Qt::yellow));
    sma20Graph->setName("SMA(20)");

    // 設置 K 線圖的 Y 軸範圍
    technicalPlot->yAxis->setRange(*std::min_element(lows.constBegin(), lows.constEnd()) * 0.99,
                                   *std::max_element(highs.constBegin(), highs.constEnd()) * 1.01);
    technicalPlot->yAxis->setLabel("價格");
    technicalPlot->yAxis->setLabelColor(Qt::white);
    technicalPlot->yAxis->setTickLabelColor(Qt::white);
    technicalPlot->yAxis->setBasePen(QPen(Qt::gray));
    technicalPlot->yAxis->setTickPen(QPen(Qt::gray));
    technicalPlot->yAxis->setSubTickPen(QPen(Qt::gray));

    // 創建成交量圖（下部）
    QCPAxisRect *volumeAxisRect = new QCPAxisRect(technicalPlot);
    technicalPlot->plotLayout()->addElement(1, 0, volumeAxisRect); // 放在 K 線圖下方
    QCPBars *volumeBars = new QCPBars(volumeAxisRect->axis(QCPAxis::atBottom), volumeAxisRect->axis(QCPAxis::atLeft));
    volumeBars->setData(timestamps, volumes);
    volumeBars->setWidth(0.6 * (timestamps[1] - timestamps[0]));
    volumeBars->setPen(QPen(Qt::gray));
    volumeBars->setBrush(QBrush(Qt::gray));
    volumeBars->setName("成交量");

    // 設置成交量圖的 Y 軸範圍
    volumeAxisRect->axis(QCPAxis::atLeft)->setRange(0, *std::max_element(volumes.constBegin(), volumes.constEnd()) * 1.1);
    volumeAxisRect->axis(QCPAxis::atLeft)->setLabel("成交量");
    volumeAxisRect->axis(QCPAxis::atLeft)->setLabelColor(Qt::white);
    volumeAxisRect->axis(QCPAxis::atLeft)->setTickLabelColor(Qt::white);
    volumeAxisRect->axis(QCPAxis::atLeft)->setBasePen(QPen(Qt::gray));
    volumeAxisRect->axis(QCPAxis::atLeft)->setTickPen(QPen(Qt::gray));
    volumeAxisRect->axis(QCPAxis::atLeft)->setSubTickPen(QPen(Qt::gray));

    // 設置 X 軸（日期）
    technicalPlot->xAxis->setBasePen(QPen(Qt::gray));
    technicalPlot->xAxis->setTickPen(QPen(Qt::gray));
    technicalPlot->xAxis->setSubTickPen(QPen(Qt::gray));
    technicalPlot->xAxis->setTickLabelColor(Qt::white);
    technicalPlot->xAxis->setLabelColor(Qt::white);
    auto dateTicker = QSharedPointer<QCPAxisTickerDateTime>(new QCPAxisTickerDateTime);
    dateTicker->setDateTimeFormat("yyyy/MM/dd");
    technicalPlot->xAxis->setTicker(dateTicker);
    technicalPlot->xAxis->setRange(timestamps.first(), timestamps.last());

    // 同步成交量圖的 X 軸
    volumeAxisRect->axis(QCPAxis::atBottom)->setBasePen(QPen(Qt::gray));
    volumeAxisRect->axis(QCPAxis::atBottom)->setTickPen(QPen(Qt::gray));
    volumeAxisRect->axis(QCPAxis::atBottom)->setSubTickPen(QPen(Qt::gray));
    volumeAxisRect->axis(QCPAxis::atBottom)->setTickLabelColor(Qt::white);
    volumeAxisRect->axis(QCPAxis::atBottom)->setLabelColor(Qt::white);
    auto volumeDateTicker = QSharedPointer<QCPAxisTickerDateTime>(new QCPAxisTickerDateTime);
    volumeDateTicker->setDateTimeFormat("yyyy/MM/dd");
    volumeAxisRect->axis(QCPAxis::atBottom)->setTicker(volumeDateTicker);
    volumeAxisRect->axis(QCPAxis::atBottom)->setRange(timestamps.first(), timestamps.last());

    // 添加圖例
    technicalPlot->legend->setVisible(true);
    technicalPlot->legend->setFont(QFont("微軟正黑體", 9));
    technicalPlot->legend->setTextColor(Qt::white);
    technicalPlot->legend->setBrush(QBrush(Qt::black));
    technicalPlot->legend->setBorderPen(QPen(Qt::gray));

    // 設置圖表間距
    technicalPlot->plotLayout()->setRowSpacing(0);
    technicalPlot->plotLayout()->setRowStretchFactors(QList<double>() << 3 << 1); // K 線圖佔 3/4，成交量圖佔 1/4

    // 刷新圖表
    technicalPlot->replot();
}

QVector<double> StockDetailWindow::calculateSMA(const QVector<DailyStockData> &dailyData, int period) {
    QVector<double> sma;
    sma.reserve(dailyData.size());

    for (int i = 0; i < dailyData.size(); ++i) {
        if (i < period - 1) {
            sma.append(0); // 前幾天沒有足夠數據計算 SMA
            continue;
        }

        double sum = 0;
        for (int j = 0; j < period; ++j) {
            sum += dailyData[i - j].close;
        }
        sma.append(sum / period);
    }

    return sma;
}

QString StockDetailWindow::formatNumberWithCommas(double number) {
    QLocale locale(QLocale::English); // 使用英文地區設定，千位分號為 ","
    return locale.toString(number, 'f', 0); // 格式化數字，無小數點
}

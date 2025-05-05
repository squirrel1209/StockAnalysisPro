#include "stockdetailwindow.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QFont>
#include <QDateTime>
#include <QMouseEvent>
#include <QDebug>

StockDetailWindow::StockDetailWindow(const QString &symbol, const SingleStockDataManager &stockData, QWidget *parent)
    : QDialog(parent) {
    // 設置窗口標題
    setWindowTitle(QString("股票詳細資訊 - %1").arg(symbol));

    // 設置窗口大小
    setMinimumSize(1200, 1000);

    // 設置背景為黑色
    setStyleSheet("QDialog { background-color: black; }");

    // 獲取股票數據
    dailyData = stockData.getAllDailyData();
    if (dailyData.isEmpty()) {
        qDebug() << "No data available for stock:" << symbol;
        return;
    }

    // 按日期升序排列 dailyData（確保一致性）
    std::sort(dailyData.begin(), dailyData.end(), [](const DailyStockData &a, const DailyStockData &b) {
        return a.date < b.date;
    });

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

    // 創建數據顯示標籤
    dataLabel = new QLabel(technicalDetailTab);
    dataLabel->setStyleSheet("QLabel { color: white; font-family: '微軟正黑體'; font-size: 12px; }");
    dataLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    dataLabel->setFixedHeight(30); // 限制 dataLabel 高度，避免佔據過多空間
    dataLabel->setAttribute(Qt::WA_TransparentForMouseEvents); // 讓滑鼠事件穿透 dataLabel

    QVBoxLayout *technicalDetailLayout = new QVBoxLayout(technicalDetailTab);
    technicalDetailLayout->addWidget(dataLabel);
    technicalDetailLayout->addWidget(technicalPlot, 1); // 設置 technicalPlot 拉伸因子為 1，佔據剩餘空間
    technicalDetailLayout->setContentsMargins(0, 0, 0, 0);
    technicalDetailLayout->setSpacing(0); // 移除佈局間距
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

    // 啟用滑鼠追蹤
    technicalPlot->setMouseTracking(true);
    technicalDetailTab->setMouseTracking(true);
    setMouseTracking(true);

    // 調試日誌
    qDebug() << "Mouse tracking enabled for technicalPlot:" << technicalPlot->hasMouseTracking();
    qDebug() << "Mouse tracking enabled for technicalDetailTab:" << technicalDetailTab->hasMouseTracking();
    qDebug() << "Mouse tracking enabled for StockDetailWindow:" << hasMouseTracking();
}

void StockDetailWindow::setupStockDetailTab(const QString &symbol, const SingleStockDataManager &stockData) {
    // 設置表格欄位
    detailTable->setColumnCount(6);
    detailTable->setHorizontalHeaderLabels({
        "日期", "開盤價", "最高價", "最低價", "收盤價", "成交量"
    });

    // 設置表格樣式
    detailTable->setStyleSheet(
        "QTableWidget { background-color: black; color: white; gridline-color: gray; }"
        "QTableWidget { border: 1px solid gray; }" // 加粗表格邊線
        "QHeaderView::section { background-color: #1773cd; color: white; }" // 標頭邊線
        "QTableWidget::item { border: 1px solid gray; }" // 每個單元格邊線
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
    // 準備數據
    QVector<double> opens, highs, lows, closes, volumes;
    QVector<double> ma5, ma10, ma20, k, d, rsi, macdLine, signalLine, histogram;
    timestamps.clear();

    for (int i = 0; i < dailyData.size(); ++i) {
        const DailyStockData &data = dailyData[i];
        timestamps.append(QDateTime(data.date, QTime(0, 0)).toSecsSinceEpoch());
        opens.append(data.open);
        highs.append(data.high);
        lows.append(data.low);
        closes.append(data.close);
        volumes.append(data.volume);
        ma5.append(data.ma5);
        ma10.append(data.ma10);
        ma20.append(data.ma20);
        k.append(data.k);
        d.append(data.d);
        rsi.append(data.rsi);
        macdLine.append(data.macd_line);
        signalLine.append(data.signal_line);
        histogram.append(data.histogram);
    }

    // 過濾無效數據（例如 0 值）
    QVector<double> ma5Filtered, ma10Filtered, ma20Filtered, kFiltered, dFiltered, rsiFiltered, macdFiltered, signalFiltered, histoFiltered;
    QVector<double> ma5Times, ma10Times, ma20Times, kTimes, dTimes, rsiTimes, macdTimes, signalTimes, histoTimes;
    for (int i = 0; i < dailyData.size(); ++i) {
        if (ma5[i] != 0) {
            ma5Filtered.append(ma5[i]);
            ma5Times.append(timestamps[i]);
        }
        if (ma10[i] != 0) {
            ma10Filtered.append(ma10[i]);
            ma10Times.append(timestamps[i]);
        }
        if (ma20[i] != 0) {
            ma20Filtered.append(ma20[i]);
            ma20Times.append(timestamps[i]);
        }
        if (k[i] != 0) {
            kFiltered.append(k[i]);
            kTimes.append(timestamps[i]);
        }
        if (d[i] != 0) {
            dFiltered.append(d[i]);
            dTimes.append(timestamps[i]);
        }
        if (rsi[i] != 0) {
            rsiFiltered.append(rsi[i]);
            rsiTimes.append(timestamps[i]);
        }
        if (macdLine[i] != 0) {
            macdFiltered.append(macdLine[i]);
            macdTimes.append(timestamps[i]);
        }
        if (signalLine[i] != 0) {
            signalFiltered.append(signalLine[i]);
            signalTimes.append(timestamps[i]);
        }
        if (histogram[i] != 0) {
            histoFiltered.append(histogram[i]);
            histoTimes.append(timestamps[i]);
        }
    }

    // 設置背景色
    technicalPlot->setBackground(QBrush(Qt::black));
    technicalPlot->plotLayout()->clear();

    // K 線圖
    priceAxisRect = new QCPAxisRect(technicalPlot);
    technicalPlot->plotLayout()->addElement(0, 0, priceAxisRect);
    QCPFinancial *candlesticks = new QCPFinancial(priceAxisRect->axis(QCPAxis::atBottom), priceAxisRect->axis(QCPAxis::atLeft));
    candlesticks->setName("K 線");
    candlesticks->setChartStyle(QCPFinancial::csCandlestick);
    candlesticks->setData(timestamps, opens, highs, lows, closes);
    candlesticks->setWidth(0.25 * (timestamps[1] - timestamps[0]));
    candlesticks->setBrushPositive(QColor(Qt::red));
    candlesticks->setBrushNegative(QColor(Qt::green));
    candlesticks->setPenPositive(QPen(QColor(Qt::red)));
    candlesticks->setPenNegative(QPen(QColor(Qt::green)));

    // MA 線
    QCPGraph *ma5Graph = new QCPGraph(priceAxisRect->axis(QCPAxis::atBottom), priceAxisRect->axis(QCPAxis::atLeft));
    ma5Graph->setData(ma5Times, ma5Filtered);
    ma5Graph->setPen(QPen(Qt::cyan));
    ma5Graph->setName("MA5");

    QCPGraph *ma10Graph = new QCPGraph(priceAxisRect->axis(QCPAxis::atBottom), priceAxisRect->axis(QCPAxis::atLeft));
    ma10Graph->setData(ma10Times, ma10Filtered);
    ma10Graph->setPen(QPen(Qt::yellow));
    ma10Graph->setName("MA10");

    QCPGraph *ma20Graph = new QCPGraph(priceAxisRect->axis(QCPAxis::atBottom), priceAxisRect->axis(QCPAxis::atLeft));
    ma20Graph->setData(ma20Times, ma20Filtered);
    ma20Graph->setPen(QPen(Qt::magenta));
    ma20Graph->setName("MA20");

    // 設置 K 線圖 Y 軸範圍
    double minLow = *std::min_element(lows.constBegin(), lows.constEnd());
    double maxHigh = *std::max_element(highs.constBegin(), highs.constEnd());
    priceAxisRect->axis(QCPAxis::atLeft)->setRange(minLow * 0.99, maxHigh * 1.01);
    priceAxisRect->axis(QCPAxis::atLeft)->setLabel("價格");
    priceAxisRect->axis(QCPAxis::atLeft)->setLabelColor(Qt::white);
    priceAxisRect->axis(QCPAxis::atLeft)->setTickLabelColor(Qt::white);
    priceAxisRect->axis(QCPAxis::atLeft)->setBasePen(QPen(Qt::gray));
    priceAxisRect->axis(QCPAxis::atLeft)->setTickPen(QPen(Qt::gray));
    priceAxisRect->axis(QCPAxis::atLeft)->setSubTickPen(QPen(Qt::gray));

    // 成交量圖
    volumeAxisRect = new QCPAxisRect(technicalPlot);
    technicalPlot->plotLayout()->addElement(1, 0, volumeAxisRect);
    QCPBars *volumeBarsUp = new QCPBars(volumeAxisRect->axis(QCPAxis::atBottom), volumeAxisRect->axis(QCPAxis::atLeft));
    QCPBars *volumeBarsDown = new QCPBars(volumeAxisRect->axis(QCPAxis::atBottom), volumeAxisRect->axis(QCPAxis::atLeft));
    volumeBarsUp->setName("成交量");
    volumeBarsDown->setName("成交量");

    QVector<double> volumesUp, volumesDown, timesUp, timesDown;
    for (int i = 0; i < dailyData.size(); ++i) {
        double previousClose = (i > 0) ? dailyData[i - 1].close : dailyData[i].close;
        bool isUp = dailyData[i].close >= previousClose;
        if (isUp) {
            volumesUp.append(volumes[i]);
            timesUp.append(timestamps[i]);
        } else {
            volumesDown.append(volumes[i]);
            timesDown.append(timestamps[i]);
        }
    }

    volumeBarsUp->setData(timesUp, volumesUp);
    volumeBarsUp->setWidth(0.25 * (timestamps[1] - timestamps[0]));
    volumeBarsUp->setPen(QPen(Qt::red));
    volumeBarsUp->setBrush(QBrush(Qt::red));

    volumeBarsDown->setData(timesDown, volumesDown);
    volumeBarsDown->setWidth(0.25 * (timestamps[1] - timestamps[0]));
    volumeBarsDown->setPen(QPen(Qt::green));
    volumeBarsDown->setBrush(QBrush(Qt::green));

    double maxVolume = *std::max_element(volumes.constBegin(), volumes.constEnd());
    volumeAxisRect->axis(QCPAxis::atLeft)->setRange(0, maxVolume * 1.1);
    volumeAxisRect->axis(QCPAxis::atLeft)->setLabel("成交量");
    volumeAxisRect->axis(QCPAxis::atLeft)->setLabelColor(Qt::white);
    volumeAxisRect->axis(QCPAxis::atLeft)->setTickLabelColor(Qt::white);

    // RSI 圖
    rsiAxisRect = new QCPAxisRect(technicalPlot);
    technicalPlot->plotLayout()->addElement(2, 0, rsiAxisRect);
    QCPGraph *rsiGraph = new QCPGraph(rsiAxisRect->axis(QCPAxis::atBottom), rsiAxisRect->axis(QCPAxis::atLeft));
    rsiGraph->setData(rsiTimes, rsiFiltered);
    rsiGraph->setPen(QPen(Qt::cyan, 2));
    rsiGraph->setName("RSI");

    // 添加 RSI 參考線（30 和 70）
    QCPItemLine *rsiOverbought = new QCPItemLine(technicalPlot);
    rsiOverbought->start->setType(QCPItemPosition::ptPlotCoords);
    rsiOverbought->end->setType(QCPItemPosition::ptPlotCoords);
    rsiOverbought->start->setAxes(rsiAxisRect->axis(QCPAxis::atBottom), rsiAxisRect->axis(QCPAxis::atLeft));
    rsiOverbought->end->setAxes(rsiAxisRect->axis(QCPAxis::atBottom), rsiAxisRect->axis(QCPAxis::atLeft));
    rsiOverbought->start->setCoords(timestamps.first(), 70);
    rsiOverbought->end->setCoords(timestamps.last(), 70);
    rsiOverbought->setPen(QPen(Qt::red, 1, Qt::DashLine));

    QCPItemLine *rsiOversold = new QCPItemLine(technicalPlot);
    rsiOversold->start->setType(QCPItemPosition::ptPlotCoords);
    rsiOversold->end->setType(QCPItemPosition::ptPlotCoords);
    rsiOversold->start->setAxes(rsiAxisRect->axis(QCPAxis::atBottom), rsiAxisRect->axis(QCPAxis::atLeft));
    rsiOversold->end->setAxes(rsiAxisRect->axis(QCPAxis::atBottom), rsiAxisRect->axis(QCPAxis::atLeft));
    rsiOversold->start->setCoords(timestamps.first(), 30);
    rsiOversold->end->setCoords(timestamps.last(), 30);
    rsiOversold->setPen(QPen(Qt::green, 1, Qt::DashLine));

    rsiAxisRect->axis(QCPAxis::atLeft)->setRange(0, 100);
    rsiAxisRect->axis(QCPAxis::atLeft)->setLabel("RSI");
    rsiAxisRect->axis(QCPAxis::atLeft)->setLabelColor(Qt::white);
    rsiAxisRect->axis(QCPAxis::atLeft)->setTickLabelColor(Qt::white);

    // MACD 圖
    macdAxisRect = new QCPAxisRect(technicalPlot);
    technicalPlot->plotLayout()->addElement(3, 0, macdAxisRect);
    QCPGraph *macdGraph = new QCPGraph(macdAxisRect->axis(QCPAxis::atBottom), macdAxisRect->axis(QCPAxis::atLeft));
    macdGraph->setData(macdTimes, macdFiltered);
    macdGraph->setPen(QPen(QColorConstants::Svg::mediumspringgreen, 2));
    macdGraph->setName("MACD");

    QCPGraph *signalGraph = new QCPGraph(macdAxisRect->axis(QCPAxis::atBottom), macdAxisRect->axis(QCPAxis::atLeft));
    signalGraph->setData(signalTimes, signalFiltered);
    signalGraph->setPen(QPen(Qt::yellow, 2));
    signalGraph->setName("Signal");

    QCPBars *histoBars = new QCPBars(macdAxisRect->axis(QCPAxis::atBottom), macdAxisRect->axis(QCPAxis::atLeft));
    histoBars->setData(histoTimes, histoFiltered);
    histoBars->setWidth(0.25 * (timestamps[1] - timestamps[0]));
    histoBars->setPen(QPen(QColorConstants::Svg::lightpink, 1));
    histoBars->setBrush(QBrush(QColorConstants::Svg::lightpink));
    histoBars->setName("Histogram");

    double maxMacd = *std::max_element(macdFiltered.constBegin(), macdFiltered.constEnd());
    double minMacd = *std::min_element(macdFiltered.constBegin(), macdFiltered.constEnd());
    double maxSignal = *std::max_element(signalFiltered.constBegin(), signalFiltered.constEnd());
    double minSignal = *std::min_element(signalFiltered.constBegin(), signalFiltered.constEnd());
    double maxHisto = *std::max_element(histoFiltered.constBegin(), histoFiltered.constEnd());
    double minHisto = *std::min_element(histoFiltered.constBegin(), histoFiltered.constEnd());
    double maxY = qMax(maxMacd, qMax(maxSignal, maxHisto));
    double minY = qMin(minMacd, qMin(minSignal, minHisto));
    macdAxisRect->axis(QCPAxis::atLeft)->setRange(minY * 1.1, maxY * 1.1);
    macdAxisRect->axis(QCPAxis::atLeft)->setLabel("MACD");
    macdAxisRect->axis(QCPAxis::atLeft)->setLabelColor(Qt::white);
    macdAxisRect->axis(QCPAxis::atLeft)->setTickLabelColor(Qt::white);

    // 設置 X 軸（同步日期）
    auto dateTicker = QSharedPointer<QCPAxisTickerDateTime>(new QCPAxisTickerDateTime);
    dateTicker->setDateTimeFormat("yyyy/MM/dd");
    for (auto axisRect : {priceAxisRect, volumeAxisRect, rsiAxisRect, macdAxisRect}) {
        axisRect->axis(QCPAxis::atBottom)->setTicker(dateTicker);
        axisRect->axis(QCPAxis::atBottom)->setRange(timestamps.first(), timestamps.last());
        axisRect->axis(QCPAxis::atBottom)->setTickLabelRotation(0);
        axisRect->axis(QCPAxis::atBottom)->setTickLabelColor(Qt::white);
        axisRect->axis(QCPAxis::atBottom)->setLabelColor(Qt::white);
        axisRect->axis(QCPAxis::atBottom)->setBasePen(QPen(Qt::gray));
        axisRect->axis(QCPAxis::atBottom)->setTickPen(QPen(Qt::gray));
        axisRect->axis(QCPAxis::atBottom)->setSubTickPen(QPen(Qt::gray));
    }

    // 設置圖例
    QCPLegend *legend = new QCPLegend();
    priceAxisRect->insetLayout()->addElement(legend, Qt::AlignTop | Qt::AlignRight);
    legend->setVisible(true);
    legend->setFont(QFont("微軟正黑體", 9));
    legend->setTextColor(Qt::white);
    legend->setBrush(QBrush(Qt::black));
    legend->setBorderPen(QPen(Qt::gray));
    legend->addItem(new QCPPlottableLegendItem(legend, candlesticks));
    legend->addItem(new QCPPlottableLegendItem(legend, ma5Graph));
    legend->addItem(new QCPPlottableLegendItem(legend, ma10Graph));
    legend->addItem(new QCPPlottableLegendItem(legend, ma20Graph));
    legend->addItem(new QCPPlottableLegendItem(legend, rsiGraph));
    legend->addItem(new QCPPlottableLegendItem(legend, macdGraph));
    legend->addItem(new QCPPlottableLegendItem(legend, signalGraph));
    legend->addItem(new QCPPlottableLegendItem(legend, histoBars));

    // 初始化十字線 - K 線圖
    horizontalLinePrice = new QCPItemLine(technicalPlot);
    horizontalLinePrice->setPen(QPen(Qt::white, 1, Qt::DashLine));
    horizontalLinePrice->start->setType(QCPItemPosition::ptPlotCoords);
    horizontalLinePrice->end->setType(QCPItemPosition::ptPlotCoords);
    horizontalLinePrice->start->setAxes(priceAxisRect->axis(QCPAxis::atBottom), priceAxisRect->axis(QCPAxis::atLeft));
    horizontalLinePrice->end->setAxes(priceAxisRect->axis(QCPAxis::atBottom), priceAxisRect->axis(QCPAxis::atLeft));
    horizontalLinePrice->setVisible(false);

    verticalLinePrice = new QCPItemLine(technicalPlot);
    verticalLinePrice->setPen(QPen(Qt::white, 1, Qt::DashLine));
    verticalLinePrice->start->setType(QCPItemPosition::ptPlotCoords);
    verticalLinePrice->end->setType(QCPItemPosition::ptPlotCoords);
    verticalLinePrice->start->setAxes(priceAxisRect->axis(QCPAxis::atBottom), priceAxisRect->axis(QCPAxis::atLeft));
    verticalLinePrice->end->setAxes(priceAxisRect->axis(QCPAxis::atBottom), priceAxisRect->axis(QCPAxis::atLeft));
    verticalLinePrice->setVisible(false);

    // 初始化十字線 - 成交量圖
    horizontalLineVolume = new QCPItemLine(technicalPlot);
    horizontalLineVolume->setPen(QPen(Qt::white, 1, Qt::DashLine));
    horizontalLineVolume->start->setType(QCPItemPosition::ptPlotCoords);
    horizontalLineVolume->end->setType(QCPItemPosition::ptPlotCoords);
    horizontalLineVolume->start->setAxes(volumeAxisRect->axis(QCPAxis::atBottom), volumeAxisRect->axis(QCPAxis::atLeft));
    horizontalLineVolume->end->setAxes(volumeAxisRect->axis(QCPAxis::atBottom), volumeAxisRect->axis(QCPAxis::atLeft));
    horizontalLineVolume->setVisible(false);

    verticalLineVolume = new QCPItemLine(technicalPlot);
    verticalLineVolume->setPen(QPen(Qt::white, 1, Qt::DashLine));
    verticalLineVolume->start->setType(QCPItemPosition::ptPlotCoords);
    verticalLineVolume->end->setType(QCPItemPosition::ptPlotCoords);
    verticalLineVolume->start->setAxes(volumeAxisRect->axis(QCPAxis::atBottom), volumeAxisRect->axis(QCPAxis::atLeft));
    verticalLineVolume->end->setAxes(volumeAxisRect->axis(QCPAxis::atBottom), volumeAxisRect->axis(QCPAxis::atLeft));
    verticalLineVolume->setVisible(false);

    // 初始化十字線 - RSI 圖
    horizontalLineRsi = new QCPItemLine(technicalPlot);
    horizontalLineRsi->setPen(QPen(Qt::white, 1, Qt::DashLine));
    horizontalLineRsi->start->setType(QCPItemPosition::ptPlotCoords);
    horizontalLineRsi->end->setType(QCPItemPosition::ptPlotCoords);
    horizontalLineRsi->start->setAxes(rsiAxisRect->axis(QCPAxis::atBottom), rsiAxisRect->axis(QCPAxis::atLeft));
    horizontalLineRsi->end->setAxes(rsiAxisRect->axis(QCPAxis::atBottom), rsiAxisRect->axis(QCPAxis::atLeft));
    horizontalLineRsi->setVisible(false);

    verticalLineRsi = new QCPItemLine(technicalPlot);
    verticalLineRsi->setPen(QPen(Qt::white, 1, Qt::DashLine));
    verticalLineRsi->start->setType(QCPItemPosition::ptPlotCoords);
    verticalLineRsi->end->setType(QCPItemPosition::ptPlotCoords);
    verticalLineRsi->start->setAxes(rsiAxisRect->axis(QCPAxis::atBottom), rsiAxisRect->axis(QCPAxis::atLeft));
    verticalLineRsi->end->setAxes(rsiAxisRect->axis(QCPAxis::atBottom), rsiAxisRect->axis(QCPAxis::atLeft));
    verticalLineRsi->setVisible(false);

    // 初始化十字線 - MACD 圖
    horizontalLineMacd = new QCPItemLine(technicalPlot);
    horizontalLineMacd->setPen(QPen(Qt::white, 1, Qt::DashLine));
    horizontalLineMacd->start->setType(QCPItemPosition::ptPlotCoords);
    horizontalLineMacd->end->setType(QCPItemPosition::ptPlotCoords);
    horizontalLineMacd->start->setAxes(macdAxisRect->axis(QCPAxis::atBottom), macdAxisRect->axis(QCPAxis::atLeft));
    horizontalLineMacd->end->setAxes(macdAxisRect->axis(QCPAxis::atBottom), macdAxisRect->axis(QCPAxis::atLeft));
    horizontalLineMacd->setVisible(false);

    verticalLineMacd = new QCPItemLine(technicalPlot);
    verticalLineMacd->setPen(QPen(Qt::white, 1, Qt::DashLine));
    verticalLineMacd->start->setType(QCPItemPosition::ptPlotCoords);
    verticalLineMacd->end->setType(QCPItemPosition::ptPlotCoords);
    verticalLineMacd->start->setAxes(macdAxisRect->axis(QCPAxis::atBottom), macdAxisRect->axis(QCPAxis::atLeft));
    verticalLineMacd->end->setAxes(macdAxisRect->axis(QCPAxis::atBottom), macdAxisRect->axis(QCPAxis::atLeft));
    verticalLineMacd->setVisible(false);

    // 設置圖表佈局比例
    technicalPlot->plotLayout()->setRowStretchFactors(QList<double>() << 3 << 1 << 1 << 1); // K 線、成交量、RSI、MACD
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

void StockDetailWindow::mouseMoveEvent(QMouseEvent *event) {
    // 確保滑鼠在技術詳情分頁
    if (tabWidget->currentIndex() != 1) { // 技術詳情分頁是第二個分頁（索引 1）
        QDialog::mouseMoveEvent(event);
        return;
    }

    // 獲取滑鼠坐標（相對於 technicalPlot）
    QPoint pos = technicalPlot->mapFromGlobal(event->globalPosition().toPoint());

    // 調試日誌
    qDebug() << "Mouse position (pixels):" << pos;

    // 獲取 K 線圖和成交量圖的繪製範圍（考慮邊距）
    QRect priceRect = priceAxisRect->rect().adjusted(0, 0, 0, 0); // 調整邊距以匹配實際繪製區域
    QRect volumeRect = volumeAxisRect->rect().adjusted(0, 0, 0, 0);

    // 調試日誌
    qDebug() << "Price axis rect (adjusted):" << priceRect;
    qDebug() << "Volume axis rect (adjusted):" << volumeRect;

    // 檢查滑鼠是否在 K 線圖或成交量圖的繪製區域內
    bool inPricePlot = priceRect.contains(pos);
    bool inVolumePlot = volumeRect.contains(pos);

    if (!inPricePlot && !inVolumePlot) {
        qDebug() << "Mouse outside plot areas";
        horizontalLinePrice->setVisible(false);
        verticalLinePrice->setVisible(false);
        horizontalLineVolume->setVisible(false);
        verticalLineVolume->setVisible(false);
        dataLabel->clear();
        technicalPlot->replot();
        QDialog::mouseMoveEvent(event);
        return;
    }

    // 根據滑鼠所在區域選擇坐標系
    QCPAxis *xAxis = inPricePlot ? priceAxisRect->axis(QCPAxis::atBottom) : volumeAxisRect->axis(QCPAxis::atBottom);
    QCPAxis *yAxis = inPricePlot ? priceAxisRect->axis(QCPAxis::atLeft) : volumeAxisRect->axis(QCPAxis::atLeft);

    // 將像素坐標轉換為圖表坐標
    double x = xAxis->pixelToCoord(pos.x());
    double y = yAxis->pixelToCoord(pos.y());

    // 調試日誌
    qDebug() << "Chart coordinates (x, y):" << x << "," << y;
    qDebug() << "X-axis range:" << xAxis->range().lower << "to" << xAxis->range().upper;
    qDebug() << "Y-axis range:" << yAxis->range().lower << "to" << yAxis->range().upper;

    // 確保 X 坐標在數據範圍內
    double xMin = timestamps.first();
    double xMax = timestamps.last();
    if (x < xMin || x > xMax) {
        qDebug() << "X coordinate out of range:" << x << "not in" << xMin << "to" << xMax;
        x = (x < xMin) ? xMin : xMax;
    }

    // 確保 Y 坐標在範圍內
    double yMin = yAxis->range().lower;
    double yMax = yAxis->range().upper;
    if (y < yMin || y > yMax) {
        qDebug() << "Y coordinate out of range:" << y << "not in" << yMin << "to" << yMax;
        y = (y < yMin) ? yMin : yMax;
    }

    // 更新十字線
    if (inPricePlot) {
        updateCrosshair(x, y, true,false,false);
    } else if (rsiAxisRect->rect().contains(pos)) {
        updateCrosshair(x, y, false, true,false); // RSI 區域
    } else if (macdAxisRect->rect().contains(pos)) {
        updateCrosshair(x, y, false, false, true); // MACD 區域
    } else {
        // 隱藏所有十字線
    }

    // 更新數據顯示
    updateDataDisplay(x);

    // 刷新圖表
    technicalPlot->replot();

    QDialog::mouseMoveEvent(event);
}

void StockDetailWindow::updateCrosshair(double x, double y, bool inPricePlot, bool inRsiPlot , bool inMacdPlot) {
    // 找到最接近的時間戳
    int nearestIndex = 0;
    double minDiff = std::numeric_limits<double>::max();
    for (int i = 0; i < timestamps.size(); ++i) {
        double diff = qAbs(timestamps[i] - x);
        if (diff < minDiff) {
            minDiff = diff;
            nearestIndex = i;
        }
    }
    double snappedX = timestamps[nearestIndex];

    // 更新 K 線圖十字線
    if (inPricePlot) {
        horizontalLinePrice->start->setCoords(timestamps.first(), y);
        horizontalLinePrice->end->setCoords(timestamps.last(), y);
    } else {
        double midY = priceAxisRect->axis(QCPAxis::atLeft)->range().center();
        horizontalLinePrice->start->setCoords(timestamps.first(), midY);
        horizontalLinePrice->end->setCoords(timestamps.last(), midY);
    }
    verticalLinePrice->start->setCoords(snappedX, priceAxisRect->axis(QCPAxis::atLeft)->range().lower);
    verticalLinePrice->end->setCoords(snappedX, priceAxisRect->axis(QCPAxis::atLeft)->range().upper);

    // 更新 RSI 十字線
    if (inRsiPlot) {
        horizontalLineRsi->start->setCoords(timestamps.first(), y);
        horizontalLineRsi->end->setCoords(timestamps.last(), y);
    } else {
        double midY = rsiAxisRect->axis(QCPAxis::atLeft)->range().center();
        horizontalLineRsi->start->setCoords(timestamps.first(), midY);
        horizontalLineRsi->end->setCoords(timestamps.last(), midY);
    }
    verticalLineRsi->start->setCoords(snappedX, rsiAxisRect->axis(QCPAxis::atLeft)->range().lower);
    verticalLineRsi->end->setCoords(snappedX, rsiAxisRect->axis(QCPAxis::atLeft)->range().upper);

    // 更新 MACD 十字線
    if (inMacdPlot) {
        horizontalLineMacd->start->setCoords(timestamps.first(), y);
        horizontalLineMacd->end->setCoords(timestamps.last(), y);
    } else {
        double midY = macdAxisRect->axis(QCPAxis::atLeft)->range().center();
        horizontalLineMacd->start->setCoords(timestamps.first(), midY);
        horizontalLineMacd->end->setCoords(timestamps.last(), midY);
    }
    verticalLineMacd->start->setCoords(snappedX, macdAxisRect->axis(QCPAxis::atLeft)->range().lower);
    verticalLineMacd->end->setCoords(snappedX, macdAxisRect->axis(QCPAxis::atLeft)->range().upper);

    // 顯示十字線
    horizontalLinePrice->setVisible(true);
    verticalLinePrice->setVisible(true);
    horizontalLineRsi->setVisible(true);
    verticalLineRsi->setVisible(true);
    horizontalLineMacd->setVisible(true);
    verticalLineMacd->setVisible(true);
}

void StockDetailWindow::updateDataDisplay(double timestamp) {
    int nearestIndex = 0;
    double minDiff = std::numeric_limits<double>::max();
    for (int i = 0; i < timestamps.size(); ++i) {
        double diff = qAbs(timestamps[i] - timestamp);
        if (diff < minDiff) {
            minDiff = diff;
            nearestIndex = i;
        }
    }

    if (nearestIndex >= 0 && nearestIndex < dailyData.size()) {
        const DailyStockData &data = dailyData[nearestIndex];
        double previousClose = (nearestIndex > 0) ? dailyData[nearestIndex - 1].close : data.close;
        double change = data.close - previousClose;
        double changePercent = (previousClose != 0) ? (change / previousClose) * 100 : 0;

        QString text = QString("日期:%1 開:%2 低:%3 高:%4 收:%5 量:%6 漲跌:%7(%8%) RSI:%9 MACD:%10 Signal:%11")
                           .arg(data.date.toString("yyyy-MM-dd"))
                           .arg(QString::number(data.open, 'f', 2))
                           .arg(QString::number(data.low, 'f', 2))
                           .arg(QString::number(data.high, 'f', 2))
                           .arg(QString::number(data.close, 'f', 2))
                           .arg(formatNumberWithCommas(data.volume))
                           .arg(QString::number(change, 'f', 3))
                           .arg(QString::number(changePercent, 'f', 2))
                           .arg(QString::number(data.rsi, 'f', 2))
                           .arg(QString::number(data.macd_line, 'f', 2))
                           .arg(QString::number(data.signal_line, 'f', 2));
        dataLabel->setText(text);
    }
}

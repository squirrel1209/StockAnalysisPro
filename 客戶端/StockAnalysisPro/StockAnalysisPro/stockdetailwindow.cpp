#include "stockdetailwindow.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QFont>
#include <QDateTime>
#include <QMouseEvent>
#include <QDebug>


StockDetailWindow::StockDetailWindow(const QString &symbol, const SingleStockDataManager &stockData, QWidget *parent)
    : QDialog(parent), width(0.7), isCrosshairFixed(false), fixedX(0), fixedY(0),
    fixedInPricePlot(false), fixedInRsiPlot(false), fixedInMacdPlot(false) , isEventHandlingEnabled(false),
    candlesticks(nullptr), volumeBarsUp(nullptr), volumeBarsDown(nullptr), histoBars(nullptr){

    qDebug() << "Starting StockDetailWindow construction for symbol:" << symbol;

    // 設置窗口標誌，啟用系統按鈕
    setWindowFlags(Qt::Window | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    qDebug() << "Window flags set with system buttons";

    // 設置窗口標題
    setWindowTitle(QString("股票詳細資訊 - %1").arg(symbol));
    qDebug() << "Window title set";

    // 設置初始大小並記錄
    setMinimumSize(1000, 600);
    originalSize = QSize(1000, 600); // 記錄初始大小
    qDebug() << "Window size set";

    // 設置背景為黑色，確保覆蓋所有區域
    setStyleSheet(
        "QDialog { background-color: black; }"
        "QWidget { background-color: black; }"
        );
    qDebug() << "Stylesheet set for QDialog";

    // 設置背景為黑色
    setStyleSheet("QDialog { background-color: black; }");
    qDebug() << "Stylesheet set";

    // 獲取股票數據
    dailyData = stockData.getAllDailyData();
    if (dailyData.isEmpty()) {
        qDebug() << "No data available for stock:" << symbol;
        return;
    }
    qDebug() << "Daily data loaded, size:" << dailyData.size();

    // 檢查數據有效性
    bool hasValidData = false;
    for (const auto &data : dailyData) {
        if (!std::isnan(data.close) && !std::isinf(data.close) &&
            !std::isnan(data.open) && !std::isinf(data.open) &&
            !std::isnan(data.high) && !std::isinf(data.high) &&
            !std::isnan(data.low) && !std::isinf(data.low)) {
            hasValidData = true;
            break;
        }
    }
    if (!hasValidData) {
        qDebug() << "Stock data for" << symbol << "contains invalid values.";
        return;
    }

    // 按日期升序排列 dailyData（確保一致性）
    std::sort(dailyData.begin(), dailyData.end(), [](const DailyStockData &a, const DailyStockData &b) {
        return a.date < b.date;
    });
    qDebug() << "Daily data sorted";

    // 創建分頁控件
    tabWidget = new QTabWidget(this);
    tabWidget->setStyleSheet(
        "QTabWidget::pane { background-color: black; border: 1px solid gray; }"
        "QTabBar::tab { background-color: #1e90ff; color: white; padding: 5px; }"
        "QTabBar::tab:selected { background-color: #4682b4; }"
        "QTabWidget { background-color: black; }"
        );
    qDebug() << "Tab widget created";

    // 創建股票詳情分頁
    QWidget *stockDetailTab = new QWidget();
    stockDetailTab->setStyleSheet("QWidget { background-color: black; }");
    detailTable = new QTableWidget(stockDetailTab);

    // 設置 QTableWidget 的樣式，確保其背景和視圖區域為黑色
    detailTable->setStyleSheet(
        "QTableWidget { background-color: black; color: white; border: none; }"
        "QTableWidget::item { background-color: black; color: white; }"
        "QHeaderView::section { background-color: #1e90ff; color: white; }" // 表頭樣式
        "QTableCornerButton::section { background-color: black; }" // 角落按鈕背景
        );

    // 設置 QTableWidget 的視圖區域背景為黑色
    detailTable->viewport()->setStyleSheet("QWidget { background-color: black; }");

    QVBoxLayout *stockDetailLayout = new QVBoxLayout(stockDetailTab);
    stockDetailLayout->addWidget(detailTable);
    stockDetailLayout->setContentsMargins(0, 0, 0, 0);
    stockDetailTab->setLayout(stockDetailLayout);
    tabWidget->addTab(stockDetailTab, "股票詳情");
    qDebug() << "Stock detail tab created";

    // 創建技術詳情分頁
    QWidget *technicalDetailTab = new QWidget();
    technicalDetailTab->setStyleSheet("QWidget { background-color: black; }");
    technicalPlot = new QCustomPlot(technicalDetailTab);
    technicalPlot->setMinimumSize(900, 400); // 確保初始大小

    // 創建數據顯示標籤
    dataLabel = new QLabel(technicalDetailTab);
    dataLabel->setStyleSheet("QLabel { background-color: black; color: white; font-family: '微軟正黑體'; font-size: 12px; }");
    dataLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    dataLabel->setFixedHeight(30);
    dataLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    qDebug() << "Data label created";

    QVBoxLayout *technicalDetailLayout = new QVBoxLayout(technicalDetailTab);
    technicalDetailLayout->addWidget(dataLabel);
    technicalDetailLayout->addWidget(technicalPlot, 1);
    technicalDetailLayout->setContentsMargins(0, 0, 0, 0);
    technicalDetailLayout->setSpacing(0);
    technicalDetailTab->setLayout(technicalDetailLayout);
    tabWidget->addTab(technicalDetailTab, "技術詳情");
    qDebug() << "Technical detail tab created";

    // 設置主佈局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(tabWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);
    qDebug() << "Main layout set";

    // 設置股票詳情分頁
    setupStockDetailTab(symbol, stockData);
    qDebug() << "Stock detail tab setup completed";

    // 設置技術詳情分頁
    setupTechnicalDetailTab(stockData);
    qDebug() << "Technical detail tab setup completed";

    setMouseTracking(true);
    tabWidget->setMouseTracking(true);
    technicalDetailTab->setMouseTracking(true);
    technicalPlot->setMouseTracking(true);
    dataLabel->setMouseTracking(true);
    stockDetailTab->setMouseTracking(true);
    detailTable->setMouseTracking(true);

    // 安裝事件過濾器
    installEventFilter(this);
    technicalPlot->installEventFilter(this);
    technicalDetailTab->installEventFilter(this);
    tabWidget->installEventFilter(this);
    qDebug() << "Event filter installed on StockDetailWindow";

    replotTimer = new QTimer(this);
    replotTimer->setSingleShot(true);
    connect(replotTimer, &QTimer::timeout, this, [this]() {
        if (technicalPlot) {
            technicalPlot->replot(QCustomPlot::rpQueuedReplot);
            qDebug() << "Replot timer triggered, replot completed";
        }
    });

    // 強制初始化並重新繪製
    if (technicalPlot) {
        technicalPlot->replot();
        qDebug() << "Forced initial replot completed";
    }
    if (detailTable) {
        detailTable->update();
        qDebug() << "Forced table update completed";
    }

    // // 將事件處理啟用延遲到圖表繪製完成後
    // QTimer::singleShot(200, this, [this]() {
    //     isEventHandlingEnabled = true;
    //     qDebug() << "Event handling enabled after initialization";
    // });

    isEventHandlingEnabled = true;
    qDebug() << "Event handling enabled after forced initialization";

    adjustLayoutForScreen();
    qDebug() << "Initialization completed for StockDetailWindow";
    setAttribute(Qt::WA_DeleteOnClose);
}

StockDetailWindow::~StockDetailWindow() {
    qDebug() << "StockDetailWindow destructed for symbol:" << windowTitle();
    if (technicalPlot) {
        technicalPlot->clearPlottables();
        technicalPlot->clearItems();
        technicalPlot->plotLayout()->clear();
        delete technicalPlot;
        technicalPlot = nullptr;
    }
    priceAxisRect = nullptr;
    volumeAxisRect = nullptr;
    rsiAxisRect = nullptr;
    macdAxisRect = nullptr;
    horizontalLinePrice = nullptr;
    verticalLinePrice = nullptr;
    horizontalLineVolume = nullptr;
    verticalLineVolume = nullptr;
    horizontalLineRsi = nullptr;
    verticalLineRsi = nullptr;
    horizontalLineMacd = nullptr;
    verticalLineMacd = nullptr;
    candlesticks = nullptr;
    volumeBarsUp = nullptr;
    volumeBarsDown = nullptr;
    histoBars = nullptr;
    qDebug() << "All pointers cleared in destructor";
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
    QVector<QString> dateLabels;

    for (int i = 0; i < dailyData.size(); ++i) {
        const DailyStockData &data = dailyData[i];
        if (std::isnan(data.open) || std::isinf(data.open) ||
            std::isnan(data.high) || std::isinf(data.high) ||
            std::isnan(data.low) || std::isinf(data.low) ||
            std::isnan(data.close) || std::isinf(data.close)) {
            qDebug() << "Invalid data at index" << i << ":" << data.date;
            continue;
        }
        timestamps.append(static_cast<double>(i));
        dateLabels.append(data.date.toString("yyyy/MM/dd"));
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

    if (timestamps.size() < 2) {
        qDebug() << "Error: Insufficient valid data for plotting, timestamps size:" << timestamps.size();
        return;
    }

    // 過濾無效數據
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

    if (!technicalPlot) {
        qDebug() << "Error: technicalPlot is null";
        return;
    }

    technicalPlot->setBackground(QBrush(Qt::black));
    technicalPlot->setStyleSheet("QCustomPlot { background-color: black; }");
    technicalPlot->plotLayout()->clear();

    // K 線圖
    priceAxisRect = new QCPAxisRect(technicalPlot);
    priceAxisRect->setBackground(QBrush(Qt::black));
    technicalPlot->plotLayout()->addElement(0, 0, priceAxisRect);
    candlesticks = new QCPFinancial(priceAxisRect->axis(QCPAxis::atBottom), priceAxisRect->axis(QCPAxis::atLeft));
    candlesticks->setName("K 線");
    candlesticks->setChartStyle(QCPFinancial::csCandlestick);
    candlesticks->setData(timestamps, opens, highs, lows, closes);
    candlesticks->setWidth(width * (timestamps[1] - timestamps[0]));
    candlesticks->setBrushPositive(QColor(Qt::red));
    candlesticks->setBrushNegative(QColor(Qt::green));
    candlesticks->setPenPositive(QPen(QColor(Qt::red)));
    candlesticks->setPenNegative(QPen(QColor(Qt::green)));

    // MA 線
    QCPGraph *ma5Graph = nullptr;
    if (!ma5Filtered.isEmpty()) {
        ma5Graph = new QCPGraph(priceAxisRect->axis(QCPAxis::atBottom), priceAxisRect->axis(QCPAxis::atLeft));
        ma5Graph->setData(ma5Times, ma5Filtered);
        ma5Graph->setPen(QPen(Qt::cyan));
        ma5Graph->setName("MA5");
    }

    QCPGraph *ma10Graph = nullptr;
    if (!ma10Filtered.isEmpty()) {
        ma10Graph = new QCPGraph(priceAxisRect->axis(QCPAxis::atBottom), priceAxisRect->axis(QCPAxis::atLeft));
        ma10Graph->setData(ma10Times, ma10Filtered);
        ma10Graph->setPen(QPen(Qt::yellow));
        ma10Graph->setName("MA10");
    }

    QCPGraph *ma20Graph = nullptr;
    if (!ma20Filtered.isEmpty()) {
        ma20Graph = new QCPGraph(priceAxisRect->axis(QCPAxis::atBottom), priceAxisRect->axis(QCPAxis::atLeft));
        ma20Graph->setData(ma20Times, ma20Filtered);
        ma20Graph->setPen(QPen(Qt::magenta));
        ma20Graph->setName("MA20");
    }

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
    volumeAxisRect->setBackground(QBrush(Qt::black));
    technicalPlot->plotLayout()->addElement(1, 0, volumeAxisRect);
    volumeBarsUp = new QCPBars(volumeAxisRect->axis(QCPAxis::atBottom), volumeAxisRect->axis(QCPAxis::atLeft));
    volumeBarsDown = new QCPBars(volumeAxisRect->axis(QCPAxis::atBottom), volumeAxisRect->axis(QCPAxis::atLeft));
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
    volumeBarsUp->setWidth(width * (timestamps[1] - timestamps[0]));
    volumeBarsUp->setPen(QPen(Qt::red));
    volumeBarsUp->setBrush(QBrush(Qt::red));

    volumeBarsDown->setData(timesDown, volumesDown);
    volumeBarsDown->setWidth(width * (timestamps[1] - timestamps[0]));
    volumeBarsDown->setPen(QPen(Qt::green));
    volumeBarsDown->setBrush(QBrush(Qt::green));

    double maxVolume = *std::max_element(volumes.constBegin(), volumes.constEnd());
    volumeAxisRect->axis(QCPAxis::atLeft)->setRange(0, maxVolume * 1.1);
    volumeAxisRect->axis(QCPAxis::atLeft)->setLabel("成交量");
    volumeAxisRect->axis(QCPAxis::atLeft)->setLabelColor(Qt::white);
    volumeAxisRect->axis(QCPAxis::atLeft)->setTickLabelColor(Qt::white);

    // RSI 圖
    rsiAxisRect = new QCPAxisRect(technicalPlot);
    rsiAxisRect->setBackground(QBrush(Qt::black));
    technicalPlot->plotLayout()->addElement(2, 0, rsiAxisRect);
    QCPGraph *rsiGraph = nullptr;
    if (!rsiFiltered.isEmpty()) {
        rsiGraph = new QCPGraph(rsiAxisRect->axis(QCPAxis::atBottom), rsiAxisRect->axis(QCPAxis::atLeft));
        rsiGraph->setData(rsiTimes, rsiFiltered);
        rsiGraph->setPen(QPen(Qt::cyan, 2));
        rsiGraph->setName("RSI");

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
    }

    rsiAxisRect->axis(QCPAxis::atLeft)->setRange(0, 100);
    rsiAxisRect->axis(QCPAxis::atLeft)->setLabel("RSI");
    rsiAxisRect->axis(QCPAxis::atLeft)->setLabelColor(Qt::white);
    rsiAxisRect->axis(QCPAxis::atLeft)->setTickLabelColor(Qt::white);

    // MACD 圖
    macdAxisRect = new QCPAxisRect(technicalPlot);
    macdAxisRect->setBackground(QBrush(Qt::black));
    technicalPlot->plotLayout()->addElement(3, 0, macdAxisRect);
    QCPGraph *macdGraph = nullptr;
    QCPGraph *signalGraph = nullptr;
    histoBars = nullptr;
    if (!macdFiltered.isEmpty() && !signalFiltered.isEmpty() && !histoFiltered.isEmpty()) {
        macdGraph = new QCPGraph(macdAxisRect->axis(QCPAxis::atBottom), macdAxisRect->axis(QCPAxis::atLeft));
        macdGraph->setData(macdTimes, macdFiltered);
        macdGraph->setPen(QPen(QColorConstants::Svg::mediumspringgreen, 2));
        macdGraph->setName("MACD");

        signalGraph = new QCPGraph(macdAxisRect->axis(QCPAxis::atBottom), macdAxisRect->axis(QCPAxis::atLeft));
        signalGraph->setData(signalTimes, signalFiltered);
        signalGraph->setPen(QPen(Qt::yellow, 2));
        signalGraph->setName("Signal");

        histoBars = new QCPBars(macdAxisRect->axis(QCPAxis::atBottom), macdAxisRect->axis(QCPAxis::atLeft));
        histoBars->setData(histoTimes, histoFiltered);
        histoBars->setWidth(width * (timestamps[1] - timestamps[0]));
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
    }

    macdAxisRect->axis(QCPAxis::atLeft)->setLabel("MACD");
    macdAxisRect->axis(QCPAxis::atLeft)->setLabelColor(Qt::white);
    macdAxisRect->axis(QCPAxis::atLeft)->setTickLabelColor(Qt::white);

    // 設置 X 軸（使用自定義標籤）
    auto textTicker = QSharedPointer<QCPAxisTickerText>(new QCPAxisTickerText);
    for (int i = 0; i < timestamps.size(); i += 10) {
        if (i < dateLabels.size()) {
            textTicker->addTick(timestamps[i], dateLabels[i]);
        }
    }
    for (auto axisRect : {priceAxisRect, volumeAxisRect, rsiAxisRect, macdAxisRect}) {
        axisRect->axis(QCPAxis::atBottom)->setTicker(textTicker);
        axisRect->axis(QCPAxis::atBottom)->setRange(timestamps.first(), timestamps.last());
        axisRect->axis(QCPAxis::atBottom)->setTickLabelRotation(0);
        axisRect->axis(QCPAxis::atBottom)->setTickLabelColor(Qt::white);
        axisRect->axis(QCPAxis::atBottom)->setLabelColor(Qt::white);
        axisRect->axis(QCPAxis::atBottom)->setBasePen(QPen(Qt::gray));
        axisRect->axis(QCPAxis::atBottom)->setTickPen(QPen(Qt::gray));
        axisRect->axis(QCPAxis::atBottom)->setSubTickPen(QPen(Qt::gray));
    }

    // 設置圖表佈局比例
    technicalPlot->plotLayout()->setRowStretchFactors(QList<double>() << 3 << 1 << 1 << 1);

    QCPLegend *legend = new QCPLegend();
    technicalPlot->plotLayout()->addElement(0, 1, legend);
    technicalPlot->plotLayout()->setColumnStretchFactor(0, 7);
    technicalPlot->plotLayout()->setColumnStretchFactor(1, 1);
    legend->setVisible(true);
    legend->setFont(QFont("微軟正黑體", 9));
    legend->setTextColor(Qt::white);
    legend->setBrush(QBrush(Qt::black));
    legend->setBorderPen(QPen(Qt::gray));
    legend->addItem(new QCPPlottableLegendItem(legend, candlesticks));
    if (ma5Graph) legend->addItem(new QCPPlottableLegendItem(legend, ma5Graph));
    if (ma10Graph) legend->addItem(new QCPPlottableLegendItem(legend, ma10Graph));
    if (ma20Graph) legend->addItem(new QCPPlottableLegendItem(legend, ma20Graph));
    if (rsiGraph) legend->addItem(new QCPPlottableLegendItem(legend, rsiGraph));
    if (macdGraph) legend->addItem(new QCPPlottableLegendItem(legend, macdGraph));
    if (signalGraph) legend->addItem(new QCPPlottableLegendItem(legend, signalGraph));
    if (histoBars) legend->addItem(new QCPPlottableLegendItem(legend, histoBars));
    legend->setSelectableParts(QCPLegend::spItems);
    technicalPlot->setInteractions(QCP::iSelectLegend);

    connect(technicalPlot, &QCustomPlot::legendClick, this, [this](QCPLegend *legend, QCPAbstractLegendItem *item, QMouseEvent *event) {
        qDebug() << "Legend clicked, item:" << item << ", event:" << event;
        QCPPlottableLegendItem *plItem = qobject_cast<QCPPlottableLegendItem*>(item);
        if (plItem) {
            QCPAbstractPlottable *plottable = plItem->plottable();
            if (plottable) {
                plottable->setVisible(!plottable->visible());
                qDebug() << "Toggled visibility of" << plottable->name() << "to" << plottable->visible();
                item->setSelected(false);
                QTimer::singleShot(0, this, [this]() {
                    technicalPlot->replot();
                });
            }
        }
    });

    qreal legendFontSize = 9.0;
    const qreal minLegendFontSize = 6.0;
    const qreal maxLegendFontSize = 12.0;
    qreal zoomFactor = 1.0;
    connect(this, &StockDetailWindow::windowIconTextChanged, this, [this, &legendFontSize, minLegendFontSize, maxLegendFontSize, &zoomFactor, legend]() {
        zoomFactor = qMax(0.5, qMin(2.0, this->size().width() / 400.0));
        legendFontSize = qBound(minLegendFontSize, 9.0 * zoomFactor, maxLegendFontSize);
        QFont newFont("微軟正黑體", legendFontSize);
        legend->setFont(newFont);
        technicalPlot->replot();
    });

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

    qDebug() << "Setup technical plot completed, replot delayed until showEvent";
}

QString StockDetailWindow::formatNumberWithCommas(double number) {
    QLocale locale(QLocale::English); // 使用英文地區設定，千位分號為 ","
    return locale.toString(number, 'f', 0); // 格式化數字，無小數點
}

void StockDetailWindow::mouseMoveEvent(QMouseEvent *event) {
    qDebug() << "Entering mouseMoveEvent, global pos:" << event->globalPosition().toPoint();

    // 檢查指針是否有效
    if (!priceAxisRect || !volumeAxisRect || !rsiAxisRect || !macdAxisRect) {
        qDebug() << "Error: One or more axis rects are null in mouseMoveEvent";
        QDialog::mouseMoveEvent(event);
        return;
    }

    if (isCrosshairFixed) {
        qDebug() << "Crosshair is fixed, ignoring mouse move";
        QDialog::mouseMoveEvent(event);
        return;
    }

    // 將全局坐標轉換為 technicalPlot 的局部坐標
    QPoint pos = technicalPlot->mapFromGlobal(event->globalPosition().toPoint());
    qDebug() << "Mouse moved to position (pixels):" << pos << "Buttons:" << event->buttons();

    bool inPricePlot = priceAxisRect->rect().contains(pos);
    bool inVolumePlot = volumeAxisRect->rect().contains(pos);
    bool inRsiPlot = rsiAxisRect->rect().contains(pos);
    bool inMacdPlot = macdAxisRect->rect().contains(pos);
    qDebug() << "Mouse in plots - Price:" << inPricePlot << "Volume:" << inVolumePlot
             << "RSI:" << inRsiPlot << "MACD:" << inMacdPlot;

    if (!inPricePlot && !inVolumePlot && !inRsiPlot && !inMacdPlot) {
        qDebug() << "Mouse outside all plot areas - hiding crosshairs";
        horizontalLinePrice->setVisible(false);
        verticalLinePrice->setVisible(false);
        horizontalLineVolume->setVisible(false);
        verticalLineVolume->setVisible(false);
        horizontalLineRsi->setVisible(false);
        verticalLineRsi->setVisible(false);
        horizontalLineMacd->setVisible(false);
        verticalLineMacd->setVisible(false);
        dataLabel->clear();
        technicalPlot->replot();
        QDialog::mouseMoveEvent(event);
        return;
    }

    QCPAxis *xAxis = nullptr;
    QCPAxis *yAxis = nullptr;
    if (inPricePlot) {
        xAxis = priceAxisRect->axis(QCPAxis::atBottom);
        yAxis = priceAxisRect->axis(QCPAxis::atLeft);
    } else if (inVolumePlot) {
        xAxis = volumeAxisRect->axis(QCPAxis::atBottom);
        yAxis = volumeAxisRect->axis(QCPAxis::atLeft);
    } else if (inRsiPlot) {
        xAxis = rsiAxisRect->axis(QCPAxis::atBottom);
        yAxis = rsiAxisRect->axis(QCPAxis::atLeft);
    } else if (inMacdPlot) {
        xAxis = macdAxisRect->axis(QCPAxis::atBottom);
        yAxis = macdAxisRect->axis(QCPAxis::atLeft);
    }

    if (!xAxis || !yAxis) {
        qDebug() << "Error: xAxis or yAxis is null";
        QDialog::mouseMoveEvent(event);
        return;
    }

    double x = xAxis->pixelToCoord(pos.x());
    double y = yAxis->pixelToCoord(pos.y());
    qDebug() << "Converted chart coordinates (x, y):" << x << "," << y;

    double xMin = timestamps.first();
    double xMax = timestamps.last();
    if (x < xMin || x > xMax) {
        qDebug() << "X out of range, clamping:" << x << "to" << (x < xMin ? xMin : xMax);
        x = (x < xMin) ? xMin : xMax;
    }

    double yMin = yAxis->range().lower;
    double yMax = yAxis->range().upper;
    if (y < yMin || y > yMax) {
        qDebug() << "Y out of range, clamping:" << y << "to" << (y < yMin ? yMin : yMax);
        y = (y < yMin) ? yMin : yMax;
    }

    qDebug() << "Updating crosshair with x:" << x << "y:" << y;
    updateCrosshair(x, y, inPricePlot, inRsiPlot, inMacdPlot);

    qDebug() << "Updating data display with timestamp:" << x;
    updateDataDisplay(x);

    qDebug() << "Replotting technicalPlot";
    technicalPlot->replot();

    QDialog::mouseMoveEvent(event);
    qDebug() << "Exiting mouseMoveEvent";
}

void StockDetailWindow::updateCrosshair(double x, double y, bool inPricePlot, bool inRsiPlot , bool inMacdPlot) {
    // 找到最接近的索引
    int nearestIndex = qRound(x); // 因為 x 是連續索引，直接四捨五入
    if (nearestIndex < 0) nearestIndex = 0;
    if (nearestIndex >= timestamps.size()) nearestIndex = timestamps.size() - 1;
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

    replotTimer->start(10);
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

void StockDetailWindow::resizeEvent(QResizeEvent *event) {
    // 根據視窗寬度動態調整 width
    int minWidth = 400;
    int currentWidth = event->size().width();
    double widthFactor = qBound(0.2, static_cast<double>(currentWidth) / minWidth, 1.0);
    width = 0.7 * widthFactor;

    // 重新設置圖表寬度，確保指針有效
    if (candlesticks && volumeBarsUp && volumeBarsDown && histoBars && timestamps.size() > 1) {
        candlesticks->setWidth(width * (timestamps[1] - timestamps[0]));
        volumeBarsUp->setWidth(width * (timestamps[1] - timestamps[0]));
        volumeBarsDown->setWidth(width * (timestamps[1] - timestamps[0]));
        histoBars->setWidth(width * (timestamps[1] - timestamps[0]));
        qDebug() << "Updated chart widths in resizeEvent";
    } else {
        qDebug() << "Skipped chart width update in resizeEvent due to invalid pointers or insufficient timestamps";
    }

    QDialog::resizeEvent(event);
}

//覆寫 eventFilter
bool StockDetailWindow::eventFilter(QObject *obj, QEvent *event) {
    if (!isEventHandlingEnabled) {
        qDebug() << "Event handling disabled during initialization";
        return true; // 阻止事件處理
    }
    if (event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        qDebug() << "EventFilter: Mouse move detected on" << obj << "Position:" << mouseEvent->globalPosition().toPoint();
        // 檢查窗口是否完全初始化
        if (!technicalPlot || !priceAxisRect || !volumeAxisRect || !rsiAxisRect || !macdAxisRect) {
            qDebug() << "EventFilter: Axis rects or technicalPlot not fully initialized, ignoring mouse move";
            return true; // 阻止事件處理
        }
        mouseMoveEvent(mouseEvent);
        if (obj == technicalPlot || technicalPlot->isAncestorOf(qobject_cast<QWidget*>(obj))) {
            return true;
        }
        return false;
    } else if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        qDebug() << "EventFilter: Mouse button press detected on" << obj << "Button:" << mouseEvent->button()
                 << "Position:" << mouseEvent->globalPosition().toPoint();
        if (mouseEvent->button() == Qt::LeftButton) {
            QPoint localPos = technicalPlot->mapFromGlobal(mouseEvent->globalPosition().toPoint());
            QCPLegend *legend = technicalPlot->legend;
            if (legend && legend->rect().contains(localPos)) {
                qDebug() << "Mouse press detected in legend area, skipping crosshair fix";
                return false;
            }
            mousePressEvent(mouseEvent);
            if (obj == technicalPlot || technicalPlot->isAncestorOf(qobject_cast<QWidget*>(obj))) {
                return true;
            }
        }
        return false;
    }
    return QDialog::eventFilter(obj, event);
}
void StockDetailWindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        // 切換固定狀態
        isCrosshairFixed = !isCrosshairFixed;
        qDebug() << "Crosshair fixed state toggled to:" << isCrosshairFixed;

        if (isCrosshairFixed) {
            // 固定十字線：記錄當前位置
            QPoint pos = technicalPlot->mapFromGlobal(event->globalPosition().toPoint());
            bool inPricePlot = priceAxisRect && priceAxisRect->rect().contains(pos);
            bool inVolumePlot = volumeAxisRect && volumeAxisRect->rect().contains(pos);
            bool inRsiPlot = rsiAxisRect && rsiAxisRect->rect().contains(pos);
            bool inMacdPlot = macdAxisRect && macdAxisRect->rect().contains(pos);

            if (!inPricePlot && !inVolumePlot && !inRsiPlot && !inMacdPlot) {
                qDebug() << "Click outside plot areas - cannot fix crosshair";
                isCrosshairFixed = false; // 如果點擊不在圖表區域，取消固定
                return;
            }

            QCPAxis *xAxis = nullptr;
            QCPAxis *yAxis = nullptr;
            if (inPricePlot) {
                xAxis = priceAxisRect->axis(QCPAxis::atBottom);
                yAxis = priceAxisRect->axis(QCPAxis::atLeft);
            } else if (inVolumePlot) {
                xAxis = volumeAxisRect->axis(QCPAxis::atBottom);
                yAxis = volumeAxisRect->axis(QCPAxis::atLeft);
            } else if (inRsiPlot) {
                xAxis = rsiAxisRect->axis(QCPAxis::atBottom);
                yAxis = rsiAxisRect->axis(QCPAxis::atLeft);
            } else if (inMacdPlot) {
                xAxis = macdAxisRect->axis(QCPAxis::atBottom);
                yAxis = macdAxisRect->axis(QCPAxis::atLeft);
            }

            fixedX = xAxis->pixelToCoord(pos.x());
            fixedY = yAxis->pixelToCoord(pos.y());

            // 限制 x 在 timestamps 範圍內
            double xMin = timestamps.first();
            double xMax = timestamps.last();
            if (fixedX < xMin || fixedX > xMax) {
                fixedX = (fixedX < xMin) ? xMin : xMax;
            }

            // 限制 y 在當前圖表範圍內
            double yMin = yAxis->range().lower;
            double yMax = yAxis->range().upper;
            if (fixedY < yMin || fixedY > yMax) {
                fixedY = (fixedY < yMin) ? yMin : yMax;
            }

            // 記錄固定時的圖表區域
            fixedInPricePlot = inPricePlot;
            fixedInRsiPlot = inRsiPlot;
            fixedInMacdPlot = inMacdPlot;

            qDebug() << "Crosshair fixed at x:" << fixedX << "y:" << fixedY
                     << "In Price:" << fixedInPricePlot << "In RSI:" << fixedInRsiPlot
                     << "In MACD:" << fixedInMacdPlot;

            // 更新十字線和數據顯示
            updateCrosshair(fixedX, fixedY, fixedInPricePlot, fixedInRsiPlot, fixedInMacdPlot);
            updateDataDisplay(fixedX);
            technicalPlot->replot();
        } else {
            // 取消固定：恢復滑鼠移動更新模式
            qDebug() << "Crosshair unfixed, resuming mouse tracking";
            // 滑鼠可能已經移出圖表區域，檢查當前位置
            QPoint pos = technicalPlot->mapFromGlobal(event->globalPosition().toPoint());
            bool inPricePlot = priceAxisRect && priceAxisRect->rect().contains(pos);
            bool inVolumePlot = volumeAxisRect && volumeAxisRect->rect().contains(pos);
            bool inRsiPlot = rsiAxisRect && rsiAxisRect->rect().contains(pos);
            bool inMacdPlot = macdAxisRect && macdAxisRect->rect().contains(pos);

            if (!inPricePlot && !inVolumePlot && !inRsiPlot && !inMacdPlot) {
                // 如果當前滑鼠不在圖表區域，隱藏十字線
                horizontalLinePrice->setVisible(false);
                verticalLinePrice->setVisible(false);
                horizontalLineVolume->setVisible(false);
                verticalLineVolume->setVisible(false);
                horizontalLineRsi->setVisible(false);
                verticalLineRsi->setVisible(false);
                horizontalLineMacd->setVisible(false);
                verticalLineMacd->setVisible(false);
                dataLabel->clear();
                technicalPlot->replot();
            }
        }
    }
    QDialog::mousePressEvent(event);
}

void StockDetailWindow::adjustLayoutForScreen() {
    qDebug() << "Entering adjustLayoutForScreen";
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) {
        qDebug() << "Error: No primary screen available";
        return;
    }
    QRect screenGeometry = screen->availableGeometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();
    qDebug() << "Screen size:" << screenWidth << "x" << screenHeight;

    int newWidth = qMin(static_cast<int>(screenWidth * 0.8), originalSize.width());
    int newHeight = qMin(static_cast<int>(screenHeight * 0.8), originalSize.height());
    qDebug() << "Resizing to:" << newWidth << "x" << newHeight;
    resize(newWidth, newHeight);

    if (technicalPlot) {
        qDebug() << "Adjusting technicalPlot size";
        technicalPlot->setMinimumSize(this->size().width() * 0.9, height() * 0.7);
    } else {
        qDebug() << "Error: technicalPlot is null";
    }
    if (detailTable) {
        qDebug() << "Adjusting detailTable size";
        detailTable->setMinimumSize(this->size().width() * 0.9, height() * 0.2);
    } else {
        qDebug() << "Error: detailTable is null";
    }

    if (isCrosshairFixed && technicalPlot) {
        qDebug() << "Crosshair is fixed, updating with x:" << fixedX << "y:" << fixedY;
        updateCrosshair(fixedX, fixedY, fixedInPricePlot, fixedInRsiPlot, fixedInMacdPlot);
    }
    if (technicalPlot) {
        technicalPlot->replot();
        qDebug() << "Replot completed";
    }
    qDebug() << "Exiting adjustLayoutForScreen";
}

void StockDetailWindow::showEvent(QShowEvent *event) {
    qDebug() << "Show event triggered, technicalPlot:" << (technicalPlot ? "valid" : "null");

    // 確保 technicalPlot 已初始化
    if (!technicalPlot) {
        qDebug() << "Error: technicalPlot is null in showEvent";
        QDialog::showEvent(event);
        return;
    }

    // 執行調整佈局
    adjustLayoutForScreen();
    qDebug() << "Show event: adjustLayoutForScreen completed";

    // 延遲啟用事件處理並初始化
    QTimer::singleShot(200, this, [this]() {
        if (technicalPlot && priceAxisRect && volumeAxisRect && rsiAxisRect && macdAxisRect) {
            isEventHandlingEnabled = true;
            qDebug() << "Event handling enabled after initialization";

            // 初始顯示十字線和數據
            if (!timestamps.isEmpty()) {
                double initialX = timestamps.last(); // 使用最後一個數據點
                double initialY = priceAxisRect->axis(QCPAxis::atLeft)->range().center();
                updateCrosshair(initialX, initialY, true, false, false);
                updateDataDisplay(initialX);
                technicalPlot->replot();
            }
        } else {
            qDebug() << "Error: Some components are null during delayed initialization";
        }
    });

    QDialog::showEvent(event);
}


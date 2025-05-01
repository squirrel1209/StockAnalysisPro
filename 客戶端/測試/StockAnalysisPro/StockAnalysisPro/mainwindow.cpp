#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "stockdetailwindow.h"
#include "stockdatareader.h"
#include <QDate>
#include <QDebug>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QFont>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);

    // 設置窗口標題
    setWindowTitle("股票分析工具");

    // 設置背景為黑色
    setStyleSheet("QMainWindow { background-color: black; }");

    // 設置佈局以確保表格適應窗口大小
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    layout->addWidget(ui->stockTable);
    layout->setContentsMargins(0, 0, 0, 0); // 移除邊距
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    // 設置表格
    setupStockTable();

    // 初始化 socket 接收器
    socketReceiver = new StockDataSocketReceiver(this);
    connect(socketReceiver, &StockDataSocketReceiver::dataReceived, this, &MainWindow::onDataReceived);
    connect(socketReceiver, &StockDataSocketReceiver::errorOccurred, this, &MainWindow::onSocketError);

    // 連接到後端伺服器（請根據實際後端 IP 和端口修改）
    if (!socketReceiver->connectToServer("127.0.0.1", 8080)) {
        qDebug() << "Failed to connect to server";
    } else {
        qDebug() << "Connecting to server...";
    }

    // // 動態掃描 JSON 檔案
    // QDir dir("C:/c++/QT/StockAnalysisPro/json"); // 指定 JSON 檔案目錄
    // if (!dir.exists()) {
    //     qDebug() << "Directory does not exist:" << dir.absolutePath();
    // }

    // // 設置檔案過濾器，只選擇 .json 檔案
    // QStringList filters;
    // filters << "*.json";
    // dir.setNameFilters(filters);

    // // 獲取所有 JSON 檔案的路徑
    // QVector<QString> filePaths;
    // for (const QString &fileName : dir.entryList()) {
    //     filePaths.append(dir.absoluteFilePath(fileName));
    //     qDebug() << "Found JSON file:" << dir.absoluteFilePath(fileName);
    // }

    // // 如果沒有找到 JSON 檔案，記錄訊息
    // if (filePaths.isEmpty()) {
    //     qDebug() << "No JSON files found in directory:" << dir.absolutePath();
    // }

    // // 讀取所有 JSON 檔案
    // StockDataReader reader;
    // if (reader.readMultipleJsonFiles(filePaths, stockDataManager)) {
    //     qDebug() << "Successfully loaded all stock data";
    // } else {
    //     qDebug() << "Failed to load some stock data";
    // }

    // //模擬股票數據
    // populateStockTable();

    // 連接到雙擊事件
    connect(ui->stockTable, &QTableWidget::doubleClicked, this, &MainWindow::onStockTableDoubleClicked);

    // 連接到標頭點擊事件
    connect(ui->stockTable->horizontalHeader(), &QHeaderView::sectionClicked, this, &MainWindow::onHeaderClicked);
}

MainWindow::~MainWindow()
{
    socketReceiver->disconnectFromServer();
    delete ui;
}

void MainWindow::setupStockTable() {
    // 設置表格欄位
    ui->stockTable->setColumnCount(11);
    ui->stockTable->setHorizontalHeaderLabels({
        "名稱", "價格", "買價", "賣價", "漲跌", "漲跌%", "成交量", "開盤價", "高點", "低點", "日期"
    });

    // 設置表格樣式（黑色背景，白色文字）
    ui->stockTable->setStyleSheet(
        "QTableWidget { background-color: black; color: white; gridline-color: gray; }"
        "QTableWidget { border: 1 px solid gray; }" // 加粗表格邊線
        "QHeaderView::section { background-color: #1773cd; color: white; }" // 標頭邊線
        "QTableWidget::item { border: 1 px solid gray; }" // 每個單元格邊線
        "QTableWidget::item:selected { background-color: #4682b4; color: white; }"
        );

    // 設置表格屬性
    ui->stockTable->setEditTriggers(QAbstractItemView::NoEditTriggers); // 禁止編輯
    ui->stockTable->setSelectionBehavior(QAbstractItemView::SelectRows); // 選擇整行
    ui->stockTable->setSelectionMode(QAbstractItemView::SingleSelection); // 單選

    // 使表格自動延伸到視窗邊界
    ui->stockTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 允許使用者調整欄寬
    ui->stockTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    // 允許使用者調整行高
    ui->stockTable->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->stockTable->verticalHeader()->setVisible(true); // 顯示垂直標頭以便調整行高

    // 設置初始行高和欄寬
    ui->stockTable->horizontalHeader()->setDefaultSectionSize(100); // 默認欄寬
    ui->stockTable->verticalHeader()->setDefaultSectionSize(30); // 默認行高

    // 設置初始字體（粗體，更改字型）
    QFont font("微軟正黑體", baseFontSize, QFont::Bold); // 字型設為微軟正黑體，加粗
    ui->stockTable->setFont(font);
    // 設置標頭字體（粗體，更改字型，字體更大）
    QFont headerFont("微軟正黑體", baseHeaderFontSize, QFont::Bold); // 標頭字體
    ui->stockTable->horizontalHeader()->setFont(headerFont);
    ui->stockTable->verticalHeader()->setFont(headerFont);
}

void MainWindow::populateStockTable() {
    // 清空表格
    ui->stockTable->setRowCount(0);

    // 從 StockDataManager 獲取所有股票的最新數據
    originalData = stockDataManager.getLatestDataForAllStocks();

    // 按股票代碼升序排序（預設排序）
    std::sort(originalData.begin(), originalData.end(),
              [](const QPair<QString, DailyStockData> &a, const QPair<QString, DailyStockData> &b) {
                  return a.first < b.first;
              });

    // 設置表格行數
    ui->stockTable->setRowCount(originalData.size());

    // 填充表格
    for (int i = 0; i < originalData.size(); ++i) {
        const QString &symbol = originalData[i].first;
        const DailyStockData &data = originalData[i].second;

        // 獲取前一天的收盤價以計算漲跌
        SingleStockDataManager stockData = stockDataManager.getStockData(symbol);
        QVector<DailyStockData> dailyData = stockData.getAllDailyData();
        double close = data.close;
        double previousClose = close;
        if (dailyData.size() > 1) {
            previousClose = dailyData[1].close; // 前一天的收盤價
        }
        double change = close - previousClose;
        double changePercent = (previousClose != 0) ? (change / previousClose) * 100 : 0;

        ui->stockTable->setItem(i, 0, new QTableWidgetItem(symbol)); // 名稱（使用股票代碼）
        ui->stockTable->setItem(i, 1, new QTableWidgetItem(QString::number(close, 'f', 2))); // 價格（收盤價）
        ui->stockTable->setItem(i, 2, new QTableWidgetItem(QString::number(close - 0.05, 'f', 2))); // 買價（模擬）
        ui->stockTable->setItem(i, 3, new QTableWidgetItem(QString::number(close + 0.05, 'f', 2))); // 賣價（模擬）
        ui->stockTable->setItem(i, 4, new QTableWidgetItem(QString::number(change, 'f', 2))); // 漲跌
        ui->stockTable->setItem(i, 5, new QTableWidgetItem(QString::number(changePercent, 'f', 2) + "%")); // 漲跌%
        ui->stockTable->setItem(i, 6, new QTableWidgetItem(formatNumberWithCommas(data.volume))); // 成交量
        ui->stockTable->setItem(i, 7, new QTableWidgetItem(QString::number(data.open))); // 開盤價
        ui->stockTable->setItem(i, 8, new QTableWidgetItem(QString::number(data.high))); // 高點
        ui->stockTable->setItem(i, 9, new QTableWidgetItem(QString::number(data.low))); // 低點
        ui->stockTable->setItem(i, 10, new QTableWidgetItem(data.date.toString("yyyy-MM-dd"))); // 日期

        // 為數字欄位儲存原始數值，以便正確排序
        ui->stockTable->item(i, 1)->setData(Qt::UserRole, close); // 價格
        ui->stockTable->item(i, 2)->setData(Qt::UserRole, close - 0.05); // 買價
        ui->stockTable->item(i, 3)->setData(Qt::UserRole, close + 0.05); // 賣價
        ui->stockTable->item(i, 4)->setData(Qt::UserRole, change); // 漲跌
        ui->stockTable->item(i, 5)->setData(Qt::UserRole, changePercent); // 漲跌%
        ui->stockTable->item(i, 6)->setData(Qt::UserRole, data.volume); // 成交量
        ui->stockTable->item(i, 7)->setData(Qt::UserRole, data.open); // 開盤價
        ui->stockTable->item(i, 8)->setData(Qt::UserRole, data.high); // 高點
        ui->stockTable->item(i, 9)->setData(Qt::UserRole, data.low); // 低點
        ui->stockTable->item(i, 10)->setData(Qt::UserRole, data.date.toString("yyyy-MM-dd")); // 日期

        // 根據漲跌設置顏色
        QTableWidgetItem *changeItem = ui->stockTable->item(i, 4);
        QTableWidgetItem *changePercentItem = ui->stockTable->item(i, 5);
        if (change > 0) {
            changeItem->setForeground(Qt::red); // 上漲顯示紅色
            changePercentItem->setForeground(Qt::red);
        } else if (change < 0) {
            changeItem->setForeground(Qt::green); // 下跌顯示綠色
            changePercentItem->setForeground(Qt::green);
        }
    }
}

void MainWindow::onStockTableDoubleClicked(const QModelIndex &index) {
    int row = index.row();
    if (row < 0 || row >= ui->stockTable->rowCount()) {
        return; // 無效的行
    }

    // 獲取股票代碼
    QString symbol = ui->stockTable->item(row, 0)->text();

    // 從 StockDataManager 獲取該股票的數據
    SingleStockDataManager stockData = stockDataManager.getStockData(symbol);
    if (stockData.getAllDailyData().isEmpty()) {
        qDebug() << "No data available for stock:" << symbol;
        return;
    }

    // 創建並顯示詳細資訊視窗
    StockDetailWindow *detailWindow = new StockDetailWindow(symbol, stockData, this);
    detailWindow->setAttribute(Qt::WA_DeleteOnClose); // 關閉時自動銷毀
    detailWindow->show();
}

void MainWindow::wheelEvent(QWheelEvent *event) {
    // 檢查是否按下 Ctrl 鍵
    if (event->modifiers() & Qt::ControlModifier) {
        // 根據滾輪方向調整縮放比例
        int delta = event->angleDelta().y();
        if (delta > 0) {
            // 向上滾動，放大
            zoomFactor += zoomStep;
        } else if (delta < 0) {
            // 向下滾動，縮小
            zoomFactor -= zoomStep;
        }

        // 限制縮放範圍
        zoomFactor = qBound(minZoom, zoomFactor, maxZoom);

        // 更新表格的縮放
        updateTableZoom();

        event->accept();
    } else {
        // 如果未按下 Ctrl 鍵，執行默認滾輪行為（例如滾動表格）
        QMainWindow::wheelEvent(event);
    }
}

void MainWindow::updateTableZoom() {
    // 調整表格內容字體大小（保持粗體和字型）
    QFont contentFont("微軟正黑體", baseFontSize * zoomFactor, QFont::Bold); // 表格內容字體
    ui->stockTable->setFont(contentFont);

    // 調整標頭字體大小（保持粗體和字型）
    QFont headerFont("微軟正黑體", baseHeaderFontSize * zoomFactor, QFont::Bold); // 標頭字體
    ui->stockTable->horizontalHeader()->setFont(headerFont);
    ui->stockTable->verticalHeader()->setFont(headerFont);

    // 調整欄寬
    for (int col = 0; col < ui->stockTable->columnCount(); ++col) {
        ui->stockTable->horizontalHeader()->resizeSection(col, baseColumnWidth * zoomFactor);
    }

    // 調整行高
    for (int row = 0; row < ui->stockTable->rowCount(); ++row) {
        ui->stockTable->verticalHeader()->resizeSection(row, baseRowHeight * zoomFactor);
    }
}

QString MainWindow::formatNumberWithCommas(double number) {
    QLocale locale(QLocale::English); // 使用英文地區設定，千位分號為 ","
    return locale.toString(number, 'f', 0); // 格式化數字，無小數點
}

void MainWindow::onHeaderClicked(int column) {
    // 獲取當前列的排序狀態，初始為 0（預設）
    int currentState = sortStates.value(column, 0);

    // 根據點擊次數更新排序狀態
    currentState = (currentState + 1) % 3; // 0 -> 1 -> 2 -> 0
    sortStates[column] = currentState;

    if (currentState == 0) {
        // 第三次點擊：恢復預設排序（按股票代碼升序）
        restoreDefaultSort();
    } else {
        // 第一次點擊（高到低）或第二次點擊（低到高）
        Qt::SortOrder order = (currentState == 1) ? Qt::DescendingOrder : Qt::AscendingOrder;

        ui->stockTable->sortItems(column, order);
    }
}

void MainWindow::restoreDefaultSort() {
    // 清空表格並按原始數據（股票代碼升序）重新填充
    ui->stockTable->setRowCount(0);
    ui->stockTable->setRowCount(originalData.size());

    for (int i = 0; i < originalData.size(); ++i) {
        const QString &symbol = originalData[i].first;
        const DailyStockData &data = originalData[i].second;

        // 獲取前一天的收盤價以計算漲跌
        SingleStockDataManager stockData = stockDataManager.getStockData(symbol);
        QVector<DailyStockData> dailyData = stockData.getAllDailyData();
        double close = data.close;
        double previousClose = close;
        if (dailyData.size() > 1) {
            previousClose = dailyData[1].close; // 前一天的收盤價
        }
        double change = close - previousClose;
        double changePercent = (previousClose != 0) ? (change / previousClose) * 100 : 0;

        ui->stockTable->setItem(i, 0, new QTableWidgetItem(symbol)); // 名稱（使用股票代碼）
        ui->stockTable->setItem(i, 1, new QTableWidgetItem(QString::number(close, 'f', 2))); // 價格（收盤價）
        ui->stockTable->setItem(i, 2, new QTableWidgetItem(QString::number(close - 0.05, 'f', 2))); // 買價（模擬）
        ui->stockTable->setItem(i, 3, new QTableWidgetItem(QString::number(close + 0.05, 'f', 2))); // 賣價（模擬）
        ui->stockTable->setItem(i, 4, new QTableWidgetItem(QString::number(change, 'f', 2))); // 漲跌
        ui->stockTable->setItem(i, 5, new QTableWidgetItem(QString::number(changePercent, 'f', 2) + "%")); // 漲跌%
        ui->stockTable->setItem(i, 6, new QTableWidgetItem(formatNumberWithCommas(data.volume))); // 成交量
        ui->stockTable->setItem(i, 7, new QTableWidgetItem(QString::number(data.open))); // 開盤價
        ui->stockTable->setItem(i, 8, new QTableWidgetItem(QString::number(data.high))); // 高點
        ui->stockTable->setItem(i, 9, new QTableWidgetItem(QString::number(data.low))); // 低點
        ui->stockTable->setItem(i, 10, new QTableWidgetItem(data.date.toString("yyyy-MM-dd"))); // 日期

        // 為數字欄位儲存原始數值，以便正確排序
        ui->stockTable->item(i, 1)->setData(Qt::UserRole, close); // 價格
        ui->stockTable->item(i, 2)->setData(Qt::UserRole, close - 0.05); // 買價
        ui->stockTable->item(i, 3)->setData(Qt::UserRole, close + 0.05); // 賣價
        ui->stockTable->item(i, 4)->setData(Qt::UserRole, change); // 漲跌
        ui->stockTable->item(i, 5)->setData(Qt::UserRole, changePercent); // 漲跌%
        ui->stockTable->item(i, 6)->setData(Qt::UserRole, data.volume); // 成交量
        ui->stockTable->item(i, 7)->setData(Qt::UserRole, data.open); // 開盤價
        ui->stockTable->item(i, 8)->setData(Qt::UserRole, data.high); // 高點
        ui->stockTable->item(i, 9)->setData(Qt::UserRole, data.low); // 低點
        ui->stockTable->item(i, 10)->setData(Qt::UserRole, data.date.toString("yyyy-MM-dd")); // 日期

        // 根據漲跌設置顏色
        QTableWidgetItem *changeItem = ui->stockTable->item(i, 4);
        QTableWidgetItem *changePercentItem = ui->stockTable->item(i, 5);
        if (change > 0) {
            changeItem->setForeground(Qt::red); // 上漲顯示紅色
            changePercentItem->setForeground(Qt::red);
        } else if (change < 0) {
            changeItem->setForeground(Qt::green); // 下跌顯示綠色
            changePercentItem->setForeground(Qt::green);
        }
    }
}

void MainWindow::onDataReceived()
{
    // 接收並處理 socket 數據
    if (socketReceiver->receiveMultipleJsonData(stockDataManager)) {
        qDebug() << "Successfully received and processed stock data";
        populateStockTable(); // 更新表格
    } else {
        qDebug() << "Failed to process some stock data";
    }
}

void MainWindow::onSocketError(const QString &error)
{
    qDebug() << "Socket error occurred:" << error;
    // 可選：顯示錯誤訊息給用戶
}

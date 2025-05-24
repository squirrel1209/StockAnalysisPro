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
#include <QPushButton>
#include <QDialog>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMap>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);

    // 設置窗口標題
    setWindowTitle("股票分析工具");

    setMinimumSize(1200, 600);
    //showFullScreen();

    // 設置背景為黑色
    setStyleSheet("QMainWindow { background-color: black; }");

    // 設置佈局以確保表格適應窗口大小
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    // 添加一個按鈕讓使用者打開欄位選擇對話框
    QPushButton *columnSelectorButton = new QPushButton("選擇顯示欄位", centralWidget);
    columnSelectorButton->setStyleSheet("QPushButton { background-color: #1773cd; color: white; padding: 5px; }");
    layout->addWidget(columnSelectorButton);
    connect(columnSelectorButton, &QPushButton::clicked, this, &MainWindow::openColumnSelectorDialog);

    layout->addWidget(ui->stockTable);
    layout->setContentsMargins(0, 0, 0, 0); // 移除邊距
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    // 初始化所有欄位名稱和顯示狀態
    allColumns = {
        "名稱", "價格", "買價", "賣價", "漲跌", "漲跌%", "成交量", "開盤價", "高點", "低點", "日期",
        "MA5", "MA10", "MA20", "K值", "D值", "RSI", "MACD線", "訊號線", "直方圖", "價格變動%",
        "交易訊號", "訊號強度", "K線實體大小", "K線類型", "上影線", "下影線"
    };
    for (const QString &column : allColumns) {
        visibleColumns[column] = true; // 預設全部顯示
    }

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

    setMouseTracking(true);
    ui->stockTable->setMouseTracking(true); // 如果 stockTable 是表格
    qDebug() << "MainWindow mouse tracking enabled:" << hasMouseTracking();

    //模擬股票數據
    populateStockTable();

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
    // 設置表格欄位（根據可見欄位動態生成）
    QStringList visibleHeaders;
    for (const QString &column : allColumns) {
        if (visibleColumns.value(column, true)) {
            visibleHeaders.append(column);
        }
    }
    ui->stockTable->setColumnCount(visibleHeaders.size());
    ui->stockTable->setHorizontalHeaderLabels(visibleHeaders);

    // 設置表格樣式（黑色背景，白色文字）
    ui->stockTable->setStyleSheet(
        "QTableWidget { background-color: black; color: white; gridline-color: gray; border: 1px solid gray; }"
        "QTableWidget::item { border: 1px solid gray; }"
        "QTableWidget::item:selected { background-color: #4682b4; color: white; }"
        "QHeaderView::section { background-color: #1773cd; color: white; padding: 5px; }"
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

    // 設置初始行高和欄寬（考慮縮放比例）
    ui->stockTable->horizontalHeader()->setDefaultSectionSize(baseColumnWidth * zoomFactor);
    ui->stockTable->verticalHeader()->setDefaultSectionSize(baseRowHeight * zoomFactor);

    // 設置字型（考慮縮放比例）
    QFont contentFont("微軟正黑體", baseFontSize * zoomFactor, QFont::Bold);
    ui->stockTable->setFont(contentFont);

    QFont headerFont("微軟正黑體", baseHeaderFontSize * zoomFactor, QFont::Bold);
    ui->stockTable->horizontalHeader()->setFont(headerFont);
    ui->stockTable->verticalHeader()->setFont(headerFont);

    // 強制刷新標頭樣式
    ui->stockTable->horizontalHeader()->setStyleSheet(
        QString("QHeaderView::section {"
                "background-color: #1773cd;"
                "color: white;"
                "padding: 5px;"
                "font-family: '微軟正黑體';"
                "font-size: %1pt;"
                "font-weight: bold;"
                "}").arg(baseHeaderFontSize * zoomFactor)
        );
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
            previousClose = dailyData[1].close;
        }
        double change = close - previousClose;
        double changePercent = (previousClose != 0) ? (change / previousClose) * 100 : 0;

        int colIndex = 0;
        for (const QString &column : allColumns) {
            if (!visibleColumns.value(column, true)) {
                continue; // 跳過隱藏的欄位
            }

            if (column == "名稱") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(symbol));
            } else if (column == "價格") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(close, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, close);
            } else if (column == "買價") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(close - 0.05, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, close - 0.05);
            } else if (column == "賣價") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(close + 0.05, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, close + 0.05);
            } else if (column == "漲跌") {
                QString changeText = QString("%1%2")
                .arg(change >= 0 ? "▲" : "▼")
                    .arg(QString::number(qAbs(change), 'f', 2));
                QTableWidgetItem *changeItem = new QTableWidgetItem(changeText);
                changeItem->setForeground(change >= 0 ? Qt::red : Qt::green);
                ui->stockTable->setItem(i, colIndex, changeItem);
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, change);
            } else if (column == "漲跌%") {
                QString percentText = QString("%1%2")
                .arg(changePercent >= 0 ? "+" : "-")
                    .arg(QString::number(qAbs(changePercent), 'f', 2));
                QTableWidgetItem *changePercentItem = new QTableWidgetItem(percentText);
                changePercentItem->setForeground(changePercent >= 0 ? Qt::red : Qt::green);
                ui->stockTable->setItem(i, colIndex, changePercentItem);
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, changePercent);
            } else if (column == "成交量") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(formatNumberWithCommas(data.volume)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.volume);
            } else if (column == "開盤價") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(data.open, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.open);
            } else if (column == "高點") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(data.high, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.high);
            } else if (column == "低點") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(data.low, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.low);
            } else if (column == "日期") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(data.date.toString("yyyy-MM-dd")));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.date.toString("yyyy-MM-dd"));
            } else if (column == "MA5") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(data.ma5, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.ma5);
            } else if (column == "MA10") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(data.ma10, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.ma10);
            } else if (column == "MA20") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(data.ma20, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.ma20);
            } else if (column == "K值") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(data.k, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.k);
            } else if (column == "D值") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(data.d, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.d);
            } else if (column == "RSI") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(data.rsi, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.rsi);
            } else if (column == "MACD線") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(data.macd_line, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.macd_line);
            } else if (column == "訊號線") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(data.signal_line, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.signal_line);
            } else if (column == "直方圖") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(data.histogram, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.histogram);
            } else if (column == "價格變動%") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(data.price_change_percent, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.price_change_percent);
            } else if (column == "交易訊號") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(data.signal.isEmpty() ? "N/A" : data.signal));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.signal.isEmpty() ? "N/A" : data.signal);
            } else if (column == "訊號強度") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(data.strength.isEmpty() ? "N/A" : data.strength));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.strength.isEmpty() ? "N/A" : data.strength);
            } else if (column == "K線實體大小") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(data.body_size, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.body_size);
            } else if (column == "K線類型") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(data.body_type));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.body_type);
            } else if (column == "上影線") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(data.upper_shadow, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.upper_shadow);
            } else if (column == "下影線") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(data.lower_shadow, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.lower_shadow);
            }
            colIndex++;
        }
    }
}

void MainWindow::openColumnSelectorDialog() {
    QDialog dialog(this);
    dialog.setWindowTitle("選擇顯示欄位");
    dialog.setStyleSheet("QDialog { background-color: black; color: white; }");

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);

    // 為每個欄位創建一個複選框，並分成兩排
    QMap<QString, QCheckBox*> checkBoxes;
    QHBoxLayout *columnsLayout = new QHBoxLayout(); // 用於兩排佈局
    QVBoxLayout *leftColumnLayout = new QVBoxLayout(); // 左邊一排
    QVBoxLayout *rightColumnLayout = new QVBoxLayout(); // 右邊一排

    int halfSize = allColumns.size() / 2; // 將欄位分成兩半
    int index = 0;
    for (const QString &column : allColumns) {
        QCheckBox *checkBox = new QCheckBox(column, &dialog);
        checkBox->setStyleSheet("QCheckBox { color: white; }");
        checkBox->setChecked(visibleColumns.value(column, true));
        checkBoxes[column] = checkBox;

        // 根據索引將複選框放入左邊或右邊佈局
        if (index < halfSize) {
            leftColumnLayout->addWidget(checkBox);
        } else {
            rightColumnLayout->addWidget(checkBox);
        }
        index++;
    }

    // 將左右兩排佈局加入到 columnsLayout 中
    columnsLayout->addLayout(leftColumnLayout);
    columnsLayout->addLayout(rightColumnLayout);
    mainLayout->addLayout(columnsLayout);

    // 添加確認和取消按鈕
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("確定", &dialog);
    okButton->setStyleSheet("QPushButton { background-color: #1773cd; color: white; padding: 5px; }");
    QPushButton *cancelButton = new QPushButton("取消", &dialog);
    cancelButton->setStyleSheet("QPushButton { background-color: #1773cd; color: white; padding: 5px; }");
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);

    connect(okButton, &QPushButton::clicked, [this, &dialog, &checkBoxes]() {
        for (const QString &column : allColumns) {
            visibleColumns[column] = checkBoxes[column]->isChecked();
        }
        setupStockTable();
        populateStockTable();
        updateTableZoom();
        dialog.accept();
    });
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    dialog.exec();
}

void MainWindow::onStockTableDoubleClicked(const QModelIndex &index) {
    qDebug() << "onStockTableDoubleClicked triggered for row:" << index.row();
    int row = index.row();
    if (row < 0 || row >= ui->stockTable->rowCount()) {
        qDebug() << "Invalid row index:" << row;
        return;
    }

    QString symbol = ui->stockTable->item(row, 0)->text();
    qDebug() << "Symbol:" << symbol;

    SingleStockDataManager stockData = stockDataManager.getStockData(symbol);
    QVector<DailyStockData> dailyData = stockData.getAllDailyData();
    if (dailyData.isEmpty()) {
        qDebug() << "No data available for stock:" << symbol;
        return;
    }

    // 驗證數據完整性
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

    try {
        StockDetailWindow *detailWindow = new StockDetailWindow(symbol, stockData, this);
        detailWindow->setAttribute(Qt::WA_DeleteOnClose);
        qDebug() << "Detail window created for symbol:" << symbol;

        // 確保初始化完成後再顯示
        QTimer::singleShot(100, detailWindow, [detailWindow]() {
            detailWindow->show();
            qDebug() << "Detail window shown for symbol:" << detailWindow->windowTitle();
        });
    } catch (const std::exception &e) {
        qDebug() << "Exception caught while creating StockDetailWindow:" << e.what();
    }
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
    // 限制字型大小
    int contentFontSize = qRound(baseFontSize * zoomFactor);
    contentFontSize = qBound(8, contentFontSize, 16); // 限制在 8-16pt
    int headerFontSize = qRound(baseHeaderFontSize * zoomFactor);
    headerFontSize = qBound(12, headerFontSize, 20); // 限制在 12-20pt

    QFont contentFont("微軟正黑體", contentFontSize, QFont::Bold);
    ui->stockTable->setFont(contentFont);

    QFont headerFont("微軟正黑體", headerFontSize, QFont::Bold);
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

    // 強制刷新標頭樣式
    ui->stockTable->horizontalHeader()->setStyleSheet(
        QString("QHeaderView::section {"
                "background-color: #1773cd;"
                "color: white;"
                "padding: 5px;"
                "font-family: '微軟正黑體';"
                "font-size: %1pt;"
                "font-weight: bold;"
                "}").arg(qRound(baseHeaderFontSize * zoomFactor))
        );

    // 強制重繪表格
    ui->stockTable->viewport()->update();
}

QString MainWindow::formatNumberWithCommas(double number) {
    QLocale locale(QLocale::English); // 使用英文地區設定，千位分號為 ","
    return locale.toString(number, 'f', 0); // 格式化數字，無小數點
}

void MainWindow::onHeaderClicked(int column) {
    // 獲取當前列的排序狀態，初始為 0（預設）
    int currentState = sortStates.value(column, 0);

    // 更新排序狀態
    currentState = (currentState + 1) % 3; // 0 -> 1 -> 2 -> 0
    sortStates[column] = currentState;

    if (currentState == 0) {
        // 恢復預設排序（按股票代碼升序）
        restoreDefaultSort();
    } else {
        // 第一次點擊（高到低）或第二次點擊（低到高）
        Qt::SortOrder order = (currentState == 1) ? Qt::DescendingOrder : Qt::AscendingOrder;
        sortTableByColumn(column, order);
    }
}

void MainWindow::restoreDefaultSort() {
    ui->stockTable->setRowCount(0);
    ui->stockTable->setRowCount(originalData.size());

    for (int i = 0; i < originalData.size(); ++i) {
        const QString &symbol = originalData[i].first;
        const DailyStockData &data = originalData[i].second;

        SingleStockDataManager stockData = stockDataManager.getStockData(symbol);
        QVector<DailyStockData> dailyData = stockData.getAllDailyData();
        double close = data.close;
        double previousClose = close;
        if (dailyData.size() > 1) {
            previousClose = dailyData[1].close;
        }
        double change = close - previousClose;
        double changePercent = (previousClose != 0) ? (change / previousClose) * 100 : 0;

        int colIndex = 0;
        for (const QString &column : allColumns) {
            if (!visibleColumns.value(column, true)) {
                continue;
            }

            if (column == "名稱") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(symbol));
            } else if (column == "價格") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(close, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, close);
            } else if (column == "買價") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(close - 0.05, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, close - 0.05);
            } else if (column == "賣價") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(close + 0.05, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, close + 0.05);
            } else if (column == "漲跌") {
                QString changeText = QString("%1%2")
                .arg(change >= 0 ? "▲" : "▼")
                    .arg(QString::number(qAbs(change), 'f', 2));
                QTableWidgetItem *changeItem = new QTableWidgetItem(changeText);
                changeItem->setForeground(change >= 0 ? Qt::red : Qt::green);
                ui->stockTable->setItem(i, colIndex, changeItem);
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, change);
            } else if (column == "漲跌%") {
                QString percentText = QString("%1%2")
                .arg(changePercent >= 0 ? "+" : "-")
                    .arg(QString::number(qAbs(changePercent), 'f', 2));
                QTableWidgetItem *changePercentItem = new QTableWidgetItem(percentText);
                changePercentItem->setForeground(changePercent >= 0 ? Qt::red : Qt::green);
                ui->stockTable->setItem(i, colIndex, changePercentItem);
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, changePercent);
            } else if (column == "成交量") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(formatNumberWithCommas(data.volume)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.volume);
            } else if (column == "開盤價") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(data.open, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.open);
            } else if (column == "高點") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(data.high, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.high);
            } else if (column == "低點") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(data.low, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.low);
            } else if (column == "日期") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(data.date.toString("yyyy-MM-dd")));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.date.toString("yyyy-MM-dd"));
            } else if (column == "MA5") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(data.ma5, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.ma5);
            } else if (column == "MA10") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(data.ma10, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.ma10);
            } else if (column == "MA20") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(data.ma20, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.ma20);
            } else if (column == "K值") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(data.k, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.k);
            } else if (column == "D值") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(data.d, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.d);
            } else if (column == "RSI") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(data.rsi, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.rsi);
            } else if (column == "MACD線") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(data.macd_line, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.macd_line);
            } else if (column == "訊號線") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(data.signal_line, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.signal_line);
            } else if (column == "直方圖") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(data.histogram, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.histogram);
            } else if (column == "價格變動%") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(data.price_change_percent, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.price_change_percent);
            } else if (column == "交易訊號") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(data.signal.isEmpty() ? "N/A" : data.signal));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.signal.isEmpty() ? "N/A" : data.signal);
            } else if (column == "訊號強度") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(data.strength.isEmpty() ? "N/A" : data.strength));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.strength.isEmpty() ? "N/A" : data.strength);
            } else if (column == "K線實體大小") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(data.body_size, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.body_size);
            } else if (column == "K線類型") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(data.body_type));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.body_type);
            } else if (column == "上影線") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(data.upper_shadow, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.upper_shadow);
            } else if (column == "下影線") {
                ui->stockTable->setItem(i, colIndex, new QTableWidgetItem(QString::number(data.lower_shadow, 'f', 2)));
                ui->stockTable->item(i, colIndex)->setData(Qt::UserRole, data.lower_shadow);
            }
            colIndex++;
        }
    }
}

void MainWindow::onDataReceived()
{
    // 接收並處理 socket 數據
    if (socketReceiver->receiveMultipleJsonData(stockDataManager)) {
        qDebug() << "Successfully received and processed stock data";
        QMetaObject::invokeMethod(this, [this]() {
            populateStockTable(); // 確保在主執行緒中更新 UI
        }, Qt::QueuedConnection);
    } else {
        qDebug() << "Failed to process some stock data";
    }
}

void MainWindow::onSocketError(const QString &error)
{
    qDebug() << "Socket error occurred:" << error;
    // 可選：顯示錯誤訊息給用戶
}

void MainWindow::sortTableByColumn(int column, Qt::SortOrder order) {
    static const QStringList numericColumns = {
        "價格", "買價", "賣價", "漲跌", "漲跌%", "成交量", "開盤價", "高點", "低點",
        "MA5", "MA10", "MA20", "K值", "D值", "RSI", "MACD線", "訊號線", "直方圖",
        "價格變動%", "K線實體大小", "上影線", "下影線"
    };
    // 獲取當前欄位的標頭名稱
    QString columnName = ui->stockTable->horizontalHeaderItem(column)->text();

    // 檢查是否為數值欄位
    bool isNumeric = numericColumns.contains(columnName);

    // 儲存當前表格數據
    QVector<QVector<QTableWidgetItem*>> tableData(ui->stockTable->rowCount());
    for (int row = 0; row < ui->stockTable->rowCount(); ++row) {
        tableData[row].resize(ui->stockTable->columnCount());
        for (int col = 0; col < ui->stockTable->columnCount(); ++col) {
            tableData[row][col] = ui->stockTable->takeItem(row, col);
        }
    }

    // 根據欄位進行排序
    std::sort(tableData.begin(), tableData.end(), [&](const QVector<QTableWidgetItem*>& row1, const QVector<QTableWidgetItem*>& row2) {
        QTableWidgetItem* item1 = row1[column];
        QTableWidgetItem* item2 = row2[column];

        if (isNumeric) {
            // 數值比較
            QVariant value1 = item1->data(Qt::UserRole);
            QVariant value2 = item2->data(Qt::UserRole);

            // 處理不同類型的數值（double 或 int）
            if (value1.typeId() == QMetaType::Double || value1.typeId() == QMetaType::Float) {
                double val1 = value1.toDouble();
                double val2 = value2.toDouble();
                return order == Qt::AscendingOrder ? val1 < val2 : val1 > val2;
            } else {
                // 假設其他數值類型為 int（例如成交量）
                int val1 = value1.toInt();
                int val2 = value2.toInt();
                return order == Qt::AscendingOrder ? val1 < val2 : val1 > val2;
            }
        } else {
            // 字串比較
            QString text1 = item1->text();
            QString text2 = item2->text();
            return order == Qt::AscendingOrder ? text1 < text2 : text1 > text2;
        }
    });

    // 將排序後的數據放回表格
    for (int row = 0; row < tableData.size(); ++row) {
        for (int col = 0; col < tableData[row].size(); ++col) {
            ui->stockTable->setItem(row, col, tableData[row][col]);
        }
    }

    // 強制重繪表格
    ui->stockTable->viewport()->update();
}

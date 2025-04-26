#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    networkThread = new NetworkThread("127.0.0.1", 8080, this); // 假設伺服器地址和埠
    setupUi();

    // 連線 NetworkThread 的信號到 MainWindow 的槽
    connect(networkThread, &NetworkThread::dataReceived, this, &MainWindow::handleDataReceived);
    connect(networkThread, &NetworkThread::errorOccurred, this, &MainWindow::handleErrorOccurred);
}

MainWindow::~MainWindow() {
    // NetworkThread 會在父物件銷毀時自動銷毀
}

void MainWindow::setupUi() {
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // 按鈕和股票選擇
    QHBoxLayout *controlLayout = new QHBoxLayout();
    connectButton = new QPushButton("Connect to Server", this);
    stockComboBox = new QComboBox(this);
    controlLayout->addWidget(connectButton);
    controlLayout->addWidget(stockComboBox);
    mainLayout->addLayout(controlLayout);

    // 表格
    tableWidget = new QTableWidget(this);
    tableWidget->setColumnCount(6);
    tableWidget->setHorizontalHeaderLabels({"Date", "Open", "High", "Low", "Close", "Volume"});
    mainLayout->addWidget(tableWidget);

    // 圖表
    chartView = new QtCharts::QChartView(this);
    mainLayout->addWidget(chartView);

    // 連線信號和槽
    connect(connectButton, &QPushButton::clicked, this, &MainWindow::onConnectButtonClicked);
    connect(stockComboBox, &QComboBox::currentTextChanged, this, &MainWindow::onStockComboBoxChanged);
}

void MainWindow::onConnectButtonClicked() {
    if (networkThread->isRunning()) {
        QMessageBox::warning(this, "Warning", "Connection is already in progress.");
        return;
    }
    networkThread->start(); // 啟動執行緒以連線伺服器並接收數據
}

void MainWindow::handleDataReceived(const QString &jsonData) {
    // 解析 JSON 並更新 stockDataManager
    if (!stockDataReader.parseMultipleJsonString(jsonData, stockDataManager)) {
        QMessageBox::critical(this, "Error", "Failed to parse JSON data.");
        return;
    }

    // 更新股票選擇下拉選單
    stockComboBox->clear();
    stockComboBox->addItems(stockDataManager.getStockSymbols());

    // 顯示第一支股票的數據
    if (!stockDataManager.getStockSymbols().isEmpty()) {
        QString firstSymbol = stockDataManager.getStockSymbols().first();
        updateTable(stockDataManager.getStockData(firstSymbol));
        updateChart(stockDataManager.getStockData(firstSymbol));
    }
}

void MainWindow::handleErrorOccurred(const QString &error) {
    QMessageBox::critical(this, "Error", error);
}

void MainWindow::onStockComboBoxChanged(const QString &symbol) {
    if (symbol.isEmpty()) return; // 防止空選擇
    SingleStockDataManager stockData = stockDataManager.getStockData(symbol);
    updateTable(stockData);
    updateChart(stockData);
}

void MainWindow::updateTable(const SingleStockDataManager &stockData) {
    tableWidget->setRowCount(0);
    QVector<DailyStockData> dailyData = stockData.getAllDailyData();
    tableWidget->setRowCount(dailyData.size());

    for (int i = 0; i < dailyData.size(); ++i) {
        const DailyStockData &data = dailyData[i];
        tableWidget->setItem(i, 0, new QTableWidgetItem(data.date.toString("yyyy-MM-dd")));
        tableWidget->setItem(i, 1, new QTableWidgetItem(QString::number(data.open)));
        tableWidget->setItem(i, 2, new QTableWidgetItem(QString::number(data.high)));
        tableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(data.low)));
        tableWidget->setItem(i, 4, new QTableWidgetItem(QString::number(data.close)));
        tableWidget->setItem(i, 5, new QTableWidgetItem(QString::number(data.volume)));
    }
}

void MainWindow::updateChart(const SingleStockDataManager &stockData) {
    QtCharts::QChart *chart = new QtCharts::QChart();
    QtCharts::QLineSeries *series = new QtCharts::QLineSeries();
    series->setName(stockData.getMetaData().symbol);

    QVector<DailyStockData> dailyData = stockData.getAllDailyData();
    for (const DailyStockData &data : dailyData) {
        QDateTime dateTime(data.date);
        series->append(dateTime.toMSecsSinceEpoch(), data.close);
    }

    chart->addSeries(series);
    QtCharts::QDateTimeAxis *axisX = new QtCharts::QDateTimeAxis();
    axisX->setFormat("yyyy-MM-dd");
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QtCharts::QValueAxis *axisY = new QtCharts::QValueAxis();
    axisY->setTitleText("Close Price");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chartView->setChart(chart);
}
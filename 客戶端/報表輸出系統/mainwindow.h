#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QtCharts/QChartView>
#include "networkthread.h"
#include "stockdatamanager.h"
#include "stockdatareader.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onConnectButtonClicked();
    void onStockComboBoxChanged(const QString &symbol);
    void handleDataReceived(const QString &jsonData);
    void handleErrorOccurred(const QString &error);

private:
    void setupUi();
    void updateTable(const SingleStockDataManager &stockData);
    void updateChart(const SingleStockDataManager &stockData);

    NetworkThread *networkThread;
    StockDataManager stockDataManager;
    StockDataReader stockDataReader;

    QTableWidget *tableWidget;
    QComboBox *stockComboBox;
    QPushButton *connectButton;
    QtCharts::QChartView *chartView;
};

#endif // MAINWINDOW_H
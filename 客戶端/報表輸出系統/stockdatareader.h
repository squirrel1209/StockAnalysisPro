#ifndef STOCKDATAREADER_H
#define STOCKDATAREADER_H

#include "stockdatamanager.h"
#include <QString>

class StockDataReader {
public:
    StockDataReader();

    // 讀取單個 JSON 檔案並填充到 SingleStockDataManager
    bool readJsonFile(const QString &filePath, SingleStockDataManager &dataManager);

    // 讀取多個 JSON 檔案並填充到 StockDataManager
    bool readMultipleJsonFiles(const QVector<QString> &filePaths, StockDataManager &dataManager);

    // 解析 JSON 字串並填充到 SingleStockDataManager
    bool parseJsonString(const QString &jsonString, SingleStockDataManager &dataManager);

    // 解析多支股票的 JSON 字串並填充到 StockDataManager
    bool parseMultipleJsonString(const QString &jsonString, StockDataManager &dataManager);
};

#endif // STOCKDATAREADER_H
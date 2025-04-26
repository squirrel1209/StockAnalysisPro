#ifndef STOCKDATAPARSER_H
#define STOCKDATAPARSER_H

#include "stockdatamanager.h"
#include <QString>

/**
 * @brief 股票資料解析器
 * StockDataParser 負責將從伺服器收到的 JSON 封包
 * 解析成程式內可使用的股票資料物件 (SingleStockDataManager)。
 */
class StockDataParser
{
public:
    /**
     * @brief 將 JSON 字串解析為單一支股票的資料管理器
     * @param jsonString 來自伺服器的 JSON 字串
     * @return 解析後的 SingleStockDataManager
     */
    static SingleStockDataManager parseSingleStock(const QString& jsonString);
};

#endif // STOCKDATAPARSER_H

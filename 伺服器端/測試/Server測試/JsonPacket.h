#ifndef JSON_PACKET_H
#define JSON_PACKET_H

#include "PacketInterface.h"
#include <string>

class JsonPacket : public PacketInterface {
public:
    // 定義資料類型常數
    static const std::string DATA_TYPE;

    // 構造函數
    JsonPacket(); // 新增無參構造函數
    JsonPacket(const std::string& json_data);

    // 實現 PacketInterface 的純虛函數
    std::string encapsulate() const override;
    bool decapsulate(const std::string& packet) override;
    std::string getDataType() const override;
    std::string getPayload() const override;

private:
    std::string json_data_; // 儲存 JSON 數據
};

#endif // JSON_PACKET_H
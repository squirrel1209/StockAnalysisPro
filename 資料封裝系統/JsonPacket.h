#ifndef JSON_PACKET_H
#define JSON_PACKET_H

#include "PacketInterface.h"
#include <string>

// 處理 JSON 資料的封包
class JsonPacket : public PacketInterface {
public:
    // 以 JSON 資料初始化封包
    JsonPacket(const std::string& json_data);

    // 允許後續設置資料
    JsonPacket() = default;

    // 封裝 JSON 資料成封包字串
    std::string encapsulate() const override;

    // 解封裝封包字串為 JSON 資料
    bool decapsulate(const std::string& packet) override;

    // 取得封包資料類型
    std::string getDataType() const override;

    // 取得封包有效載荷
    std::string getPayload() const override;

private:
    // 儲存 JSON 資料
    std::string json_data_;

    // 資料類型常數
    static const std::string DATA_TYPE;
};

#endif // JSON_PACKET_H
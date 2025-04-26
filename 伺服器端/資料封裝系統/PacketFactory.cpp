#include "PacketFactory.h"
#include "JsonPacket.h"
#include "TechnicalIndicatorPacket.h"

// 根據資料類型創建封包物件
std::unique_ptr<PacketInterface> PacketFactory::createPacket(const std::string& data_type) {
    if (data_type == "JSON") {
        return std::make_unique<JsonPacket>();
    }
    if (data_type == "TECHNICAL_INDICATOR") {
        return std::make_unique<TechnicalIndicatorPacket>();
    }
    return nullptr;
}
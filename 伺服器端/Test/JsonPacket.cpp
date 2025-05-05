#include "JsonPacket.h"

// 定義資料類型常數
const std::string JsonPacket::DATA_TYPE = "JSON";

JsonPacket::JsonPacket() : json_data_("") {}  // 空 JSON 數據

JsonPacket::JsonPacket(const std::string& json_data) : json_data_(json_data) {}

std::string JsonPacket::encapsulate() const {
    // 封裝：添加資料類型前綴
    return DATA_TYPE + "|" + json_data_;
}

bool JsonPacket::decapsulate(const std::string& packet) {
    // 檢查封包格式
    size_t pos = packet.find('|');
    if (pos == std::string::npos || packet.substr(0, pos) != DATA_TYPE) {
        return false;
    }

    // 提取 JSON 資料
    json_data_ = packet.substr(pos + 1);
    return true;
}

std::string JsonPacket::getDataType() const {
    return DATA_TYPE;
}

std::string JsonPacket::getPayload() const {
    return json_data_;
}
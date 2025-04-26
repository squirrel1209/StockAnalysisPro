#include "JsonPacket.h"
#include <sstream>

// 定義資料類型常數
const std::string JsonPacket::DATA_TYPE = "JSON";

// 以 JSON 資料初始化
JsonPacket::JsonPacket(const std::string& json_data) : json_data_(json_data) {}

// 封裝 JSON 資料成封包
std::string JsonPacket::encapsulate() const {
    std::stringstream packet;
    packet << DATA_TYPE << "|" << json_data_;
    return packet.str();
}

// 解封裝封包字串
bool JsonPacket::decapsulate(const std::string& packet) {
    size_t delimiter_pos = packet.find('|');
    if (delimiter_pos == std::string::npos || packet.substr(0, delimiter_pos) != DATA_TYPE) {
        return false;
    }
    json_data_ = packet.substr(delimiter_pos + 1);
    return true;
}

// 取得資料類型
std::string JsonPacket::getDataType() const {
    return DATA_TYPE;
}

// 取得有效載荷
std::string JsonPacket::getPayload() const {
    return json_data_;
}
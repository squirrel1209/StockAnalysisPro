#ifndef PACKET_INTERFACE_H
#define PACKET_INTERFACE_H

#include <string>

// 定義封包的抽象介面，用於資料封裝與解封裝
class PacketInterface {
public:
    // 確保衍生類物件正確清理
    virtual ~PacketInterface() = default;

    // 封裝資料成封包字串
    virtual std::string encapsulate() const = 0;

    // 解封裝封包字串為資料
    virtual bool decapsulate(const std::string& packet) = 0;

    // 取得封包資料類型
    virtual std::string getDataType() const = 0;

    // 取得封包有效載荷
    virtual std::string getPayload() const = 0;
};

#endif // PACKET_INTERFACE_H
#ifndef PACKET_FACTORY_H
#define PACKET_FACTORY_H

#include "PacketInterface.h"
#include <memory>
#include <string>

// 封包工廠，根據資料類型動態創建封包物件
class PacketFactory {
public:
    // 創建對應資料類型的封包物件
    static std::unique_ptr<PacketInterface> createPacket(const std::string& data_type);
};

#endif // PACKET_FACTORY_H
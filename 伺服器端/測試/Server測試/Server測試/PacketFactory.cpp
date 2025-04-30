#include "PacketFactory.h"
#include "JsonPacket.h"

std::unique_ptr<PacketInterface> PacketFactory::createPacket(const std::string& dataType, const std::string& data) {
    if (dataType == JsonPacket::DATA_TYPE) {
        return std::make_unique<JsonPacket>(data);
    }
    return nullptr;
}
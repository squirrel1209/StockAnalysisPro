#ifndef PACKET_FACTORY_H
#define PACKET_FACTORY_H

#include <memory>
#include <string>
#include "PacketInterface.h"

class PacketFactory {
public:
    static std::unique_ptr<PacketInterface> createPacket(const std::string& dataType, const std::string& data = "");
};

#endif // PACKET_FACTORY_H
#include "ClientApplication.h"
#include "PacketFactory.h" // 工廠模式，用於產生對應封包類型物件
#include <iostream>

ClientApplication::ClientApplication(const std::string& ip, int port)
    : network_client_(ip, port) {}

// 取得伺服器傳送過來的封包，並解析成對應物件
std::unique_ptr<PacketInterface> ClientApplication::fetchPacket() {
    if (!network_client_.initialize()) {
        std::cerr << "初始化失敗" << std::endl;
        return nullptr;
    }

    if (!network_client_.connect()) {
        std::cerr << "連線失敗" << std::endl;
        return nullptr;
    }

    std::string raw_packet = network_client_.receive();
    std::cout << "📦 [DEBUG] 收到原始資料: " << raw_packet << std::endl; // ✅ 正常印出

    if (raw_packet.empty()) {
        std::cerr << "收到的資料是空的！" << std::endl;
        return nullptr;
    }

    size_t delimiter_pos = raw_packet.find('|');
    if (delimiter_pos == std::string::npos) {
        std::cerr << "封包格式錯誤，找不到分隔符 |" << std::endl;
        return nullptr;
    }

    std::string data_type = raw_packet.substr(0, delimiter_pos);

    auto packet = PacketFactory::createPacket(data_type);
    if (!packet) {
        std::cerr << "找不到資料型別對應的封包類型！" << std::endl;
        return nullptr;
    }

    if (!packet->decapsulate(raw_packet)) {
        std::cerr << "解封封包失敗！" << std::endl;
        return nullptr;
    }

    return packet;
}


bool ClientApplication::receiveData(char* buffer, int length) {
    return network_client_.receiveRaw(buffer, length);
}
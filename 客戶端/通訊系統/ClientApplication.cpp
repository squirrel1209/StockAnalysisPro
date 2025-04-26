#include "ClientApplication.h"
#include "PacketFactory.h"
#include <iostream>
#include <memory>

// 初始化網路客戶端和檔案處理器
ClientApplication::ClientApplication(const std::string& ip, int port, const std::string& output_filename)
    : network_client_(ip, port), file_handler_(output_filename) {}

// 執行客戶端應用程式邏輯
bool ClientApplication::run() {
    // 初始化網路客戶端
    if (!network_client_.initialize()) {
        std::cerr << "網路初始化失敗" << std::endl;
        return false;
    }

    // 連線到伺服器
    if (!network_client_.connect()) {
        std::cerr << "連線失敗" << std::endl;
        return false;
    }

    // 接收封包資料
    std::string packet_data = network_client_.receive();
    if (packet_data.empty()) {
        std::cerr << "未接收到資料" << std::endl;
        return false;
    }

    // 解析封包取得資料類型
    size_t delimiter_pos = packet_data.find('|');
    if (delimiter_pos == std::string::npos) {
        std::cerr << "無效封包格式" << std::endl;
        return false;
    }
    std::string data_type = packet_data.substr(0, delimiter_pos);

    // 使用封包工廠創建封包物件
    std::unique_ptr<PacketInterface> packet = PacketFactory::createPacket(data_type);
    if (!packet) {
        std::cerr << "未知封包類型: " << data_type << std::endl;
        return false;
    }

    // 解封裝封包
    if (!packet->decapsulate(packet_data)) {
        std::cerr << "封包解封裝失敗" << std::endl;
        return false;
    }

    // 將有效載荷寫入檔案
    if (!file_handler_.write(packet->getPayload())) {
        std::cerr << "寫入檔案失敗" << std::endl;
        return false;
    }

    std::cout << "資料接收並儲存成功" << std::endl;
    return true;
}
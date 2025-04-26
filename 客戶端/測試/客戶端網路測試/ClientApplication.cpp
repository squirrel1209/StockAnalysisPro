#include "ClientApplication.h"
#include "JsonPacket.h"
#include <iostream>

ClientApplication::ClientApplication(const std::string& ip, int port, const std::string& output_filename)
    : network_client_(ip, port), file_handler_(output_filename) {}

bool ClientApplication::run() {
    // 初始化網路客戶端
    if (!network_client_.initialize()) {
        std::cerr << "客戶端初始化失敗" << std::endl;
        return false;
    }

    // 連線到伺服器
    if (!network_client_.connect()) {
        std::cerr << "連線到伺服器失敗" << std::endl;
        return false;
    }

    // 接收資料
    std::string received_data = network_client_.receive();
    if (received_data.empty()) {
        std::cerr << "未收到伺服器資料" << std::endl;
        return false;
    }

    // 創建 JSON 封包並解封裝
    JsonPacket packet;
    if (!packet.decapsulate(received_data)) {
        std::cerr << "解封裝資料失敗" << std::endl;
        return false;
    }

    // 取得有效載荷並寫入檔案
    std::string json_data = packet.getPayload();
    if (!file_handler_.write(json_data)) {
        std::cerr << "寫入檔案失敗" << std::endl;
        return false;
    }

    // 傳送確認回應
    JsonPacket response_packet("{\"status\":\"received\"}");
    if (!network_client_.sendPacket(response_packet)) {
        std::cerr << "傳送回應失敗" << std::endl;
        return false;
    }

    return true;
}
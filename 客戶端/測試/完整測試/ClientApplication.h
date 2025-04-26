#ifndef CLIENT_APPLICATION_H
#define CLIENT_APPLICATION_H

#include "NetworkClient.h"
#include <string>
#include <memory>
#include "PacketInterface.h"

// ClientApplication 類別：負責管理與伺服器溝通與封包解譯
class ClientApplication {
public:
    // 建構子：初始化時指定要連接的伺服器 IP 和 Port
    ClientApplication(const std::string& ip, int port);

    // 從伺服器取得一個封包（由 PacketFactory 自動解析封裝）
    std::unique_ptr<PacketInterface> fetchPacket();

private:
    // 網路客戶端，負責 TCP 連線與資料接收
    NetworkClient network_client_;
};

#endif // CLIENT_APPLICATION_H

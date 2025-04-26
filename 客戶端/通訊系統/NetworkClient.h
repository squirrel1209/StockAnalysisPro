#ifndef NETWORK_CLIENT_H
#define NETWORK_CLIENT_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include "PacketInterface.h"

#pragma comment(lib, "ws2_32.lib")

// 處理客戶端網路操作
class NetworkClient {
public:
    // 以伺服器 IP 和埠號初始化客戶端
    NetworkClient(const std::string& ip, int port);

    // 清理 socket 和 Winsock 資源
    ~NetworkClient();

    // 初始化 Winsock 和 socket
    bool initialize();

    // 連線到伺服器
    bool connect();

    // 接收封包字串
    std::string receive();

    // 傳送封包
    bool sendPacket(const PacketInterface& packet);

private:
    // 接收緩衝區大小
    static const int BUFFER_SIZE = 1024;

    // 伺服器 IP 地址
    std::string ip_;

    // 伺服器埠號
    int port_;

    // Socket 物件
    SOCKET sock_;

    // Winsock 資料結構
    WSADATA wsaData_;

    // 伺服器地址結構
    struct sockaddr_in serv_addr_;
};

#endif // NETWORK_CLIENT_H
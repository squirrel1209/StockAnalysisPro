#ifndef NETWORK_SERVER_H
#define NETWORK_SERVER_H

#include <winsock2.h>
#include <string>
#include "PacketInterface.h"

// 處理伺服器端網路操作
class NetworkServer {
public:
    // 以指定埠號初始化伺服器
    NetworkServer(int port);

    // 清理資源
    ~NetworkServer();

    // 初始化 Winsock 和 socket
    bool initialize();

    // 開始監聽客戶端連線
    bool startListening();

    // 接受客戶端連線
    bool acceptConnection();

    // 傳送封包
    bool sendPacket(const PacketInterface& packet);

    // 接收封包字串
    std::string receive();

    // 清理 socket 和 Winsock 資源
    void cleanup();

private:
    // Winsock 資料結構
    WSADATA wsaData;

    // 伺服器 socket
    SOCKET server_fd;

    // 客戶端 socket
    SOCKET client_socket;

    // 伺服器地址結構
    struct sockaddr_in address;

    // 地址結構長度
    int addrlen;

    // 伺服器埠號
    int port;

    // 初始化狀態
    bool initialized;

    // 創建 socket
    bool createSocket();

    // 設置 socket 選項
    bool setSocketOptions();

    // 綁定 socket 到地址
    bool bindSocket();
};

#endif // NETWORK_SERVER_H
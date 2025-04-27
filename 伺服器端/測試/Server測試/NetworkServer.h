// === NetworkServer.h ===
#ifndef NETWORK_SERVER_H
#define NETWORK_SERVER_H

#include <winsock2.h>
#include <string>
#include "PacketInterface.h"

// 處理伺服器端網路操作
class NetworkServer {
public:
    NetworkServer(int port);
    ~NetworkServer();

    bool initialize(); // 初始化 Winsock 和 socket
    bool startListening(); // 開始監聽客戶端連線
    bool acceptConnection(); // 接受一個客戶端連線

    bool sendPacket(const PacketInterface& packet); // 傳送封包給當前 client
    bool sendPacket(SOCKET client, const PacketInterface& packet); // 傳送封包給指定 client

    SOCKET getClientSocket() const; // 取得目前 client socket

    std::string receive(); // 接收封包字串
    void cleanup(); // 清理 socket 和 Winsock 資源

private:
    WSADATA wsaData;
    SOCKET server_fd;
    SOCKET client_socket;
    struct sockaddr_in address;
    int addrlen;
    int port;
    bool initialized;

    bool createSocket();
    bool setSocketOptions();
    bool bindSocket();
};

#endif // NETWORK_SERVER_H
#include "NetworkClient.h"
#include <iostream>

// 以伺cetrt IP 和埠號初始化
NetworkClient::NetworkClient(const std::string& ip, int port)
    : ip_(ip), port_(port), sock_(INVALID_SOCKET) {}

// 清理 socket 和 Winsock 資源
NetworkClient::~NetworkClient() {
    if (sock_ != INVALID_SOCKET) {
        closesocket(sock_);
    }
    WSACleanup();
}

// 初始化 Winsock 和 socket
bool NetworkClient::initialize() {
    if (WSAStartup(MAKEWORD(2, 2), &wsaData_) != 0) {
        std::cerr << "WSAStartup 失敗: " << WSAGetLastError() << std::endl;
        return false;
    }
    sock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock_ == INVALID_SOCKET) {
        std::cerr << "Socket 創建失敗: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return false;
    }
    serv_addr_.sin_family = AF_INET;
    serv_addr_.sin_port = htons(port_);
    if (inet_pton(AF_INET, ip_.c_str(), &serv_addr_.sin_addr) <= 0) {
        std::cerr << "無效地址: " << WSAGetLastError() << std::endl;
        closesocket(sock_);
        WSACleanup();
        return false;
    }
    return true;
}

// 連線到伺服器
bool NetworkClient::connect() {
    if (::connect(sock_, (struct sockaddr*)&serv_addr_, sizeof(serv_addr_)) == SOCKET_ERROR) {
        std::cerr << "連線失敗: " << WSAGetLastError() << std::endl;
        closesocket(sock_);
        WSACleanup();
        return false;
    }
    return true;
}

// 接收封包字串
std::string NetworkClient::receive() {
    char buffer[BUFFER_SIZE] = {0};
    int bytes_received = recv(sock_, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received == SOCKET_ERROR) {
        std::cerr << "接收失敗: " << WSAGetLastError() << std::endl;
        return "";
    }
    if (bytes_received == 0) {
        std::cerr << "伺服器關閉連線" << std::endl;
        return "";
    }
    return std::string(buffer, bytes_received);
}

// 傳送封包
bool NetworkClient::sendPacket(const PacketInterface& packet) {
    std::string packet_data = packet.encapsulate();
    int result = send(sock_, packet_data.c_str(), packet_data.length(), 0);
    if (result == SOCKET_ERROR) {
        std::cerr << "傳送失敗: " << WSAGetLastError() << std::endl;
        return false;
    }
    return true;
}
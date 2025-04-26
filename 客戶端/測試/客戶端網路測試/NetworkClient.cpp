#include "NetworkClient.h"
#include <iostream>

NetworkClient::NetworkClient(const std::string& ip, int port) : ip_(ip), port_(port), sock_(INVALID_SOCKET) {
    ZeroMemory(&serv_addr_, sizeof(serv_addr_));
}

NetworkClient::~NetworkClient() {
    if (sock_ != INVALID_SOCKET) {
        closesocket(sock_);
    }
    WSACleanup();
}

bool NetworkClient::initialize() {
    // 初始化 Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData_) != 0) {
        std::cerr << "WSAStartup 失敗: " << WSAGetLastError() << std::endl;
        return false;
    }

    // 創建 socket
    sock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock_ == INVALID_SOCKET) {
        std::cerr << "Socket 創建失敗: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return false;
    }

    // 設置伺服器地址
    serv_addr_.sin_family = AF_INET;
    serv_addr_.sin_port = htons(port_);
    if (inet_pton(AF_INET, ip_.c_str(), &serv_addr_.sin_addr) <= 0) {
        std::cerr << "無效的 IP 地址: " << ip_ << std::endl;
        closesocket(sock_);
        WSACleanup();
        return false;
    }

    return true;
}

bool NetworkClient::connect() {
    if (::connect(sock_, (struct sockaddr*)&serv_addr_, sizeof(serv_addr_)) == SOCKET_ERROR) {
        std::cerr << "連線失敗: " << WSAGetLastError() << std::endl;
        closesocket(sock_);
        return false;
    }
    std::cout << "成功連線到伺服器 " << ip_ << ":" << port_ << std::endl;
    return true;
}

std::string NetworkClient::receive() {
    char buffer[BUFFER_SIZE] = {0};
    int bytes_received = recv(sock_, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received == SOCKET_ERROR || bytes_received == 0) {
        std::cerr << "接收資料失敗或連線關閉: " << WSAGetLastError() << std::endl;
        return "";
    }
    return std::string(buffer, bytes_received);
}

bool NetworkClient::sendPacket(const PacketInterface& packet) {
    std::string data = packet.encapsulate();
    int bytes_sent = send(sock_, data.c_str(), data.length(), 0);
    if (bytes_sent == SOCKET_ERROR) {
        std::cerr << "傳送資料失敗: " << WSAGetLastError() << std::endl;
        return false;
    }
    return true;
}
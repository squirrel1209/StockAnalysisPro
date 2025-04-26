#include "NetworkServer.h"
#include <iostream>

// 以指定埠號初始化
NetworkServer::NetworkServer(int port) : port(port), server_fd(INVALID_SOCKET), client_socket(INVALID_SOCKET), initialized(false) {
    addrlen = sizeof(address);
}

// 清理資源
NetworkServer::~NetworkServer() {
    cleanup();
}

// 初始化 Winsock 和 socket
bool NetworkServer::initialize() {
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup 失敗: " << WSAGetLastError() << std::endl;
        return false;
    }
    if (!createSocket()) return false;
    if (!setSocketOptions()) return false;
    if (!bindSocket()) return false;
    initialized = true;
    return true;
}

// 創建 socket
bool NetworkServer::createSocket() {
    server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd == INVALID_SOCKET) {
        std::cerr << "Socket 創建失敗: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return false;
    }
    return true;
}

// 設置 socket 選項
bool NetworkServer::setSocketOptions() {
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) == SOCKET_ERROR) {
        std::cerr << "設置 socket 選項失敗: " << WSAGetLastError() << std::endl;
        closesocket(server_fd);
        WSACleanup();
        return false;
    }
    return true;
}

// 綁定 socket 到地址
bool NetworkServer::bindSocket() {
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        std::cerr << "綁定失敗: " << WSAGetLastError() << std::endl;
        closesocket(server_fd);
        WSACleanup();
        return false;
    }
    return true;
}

// 開始監聽客戶端連線
bool NetworkServer::startListening() {
    if (listen(server_fd, 3) == SOCKET_ERROR) {
        std::cerr << "監聽失敗: " << WSAGetLastError() << std::endl;
        closesocket(server_fd);
        WSACleanup();
        return false;
    }
    std::cout << "伺服器監聽於埠 " << port << std::endl;
    return true;
}

// 接受客戶端連線
bool NetworkServer::acceptConnection() {
    client_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
    if (client_socket == INVALID_SOCKET) {
        std::cerr << "接受連線失敗: " << WSAGetLastError() << std::endl;
        closesocket(server_fd);
        WSACleanup();
        return false;
    }
    std::cout << "連線已接受" << std::endl;
    return true;
}

// 傳送封包
bool NetworkServer::sendPacket(const PacketInterface& packet) {
    std::string packet_data = packet.encapsulate();
    int total_sent = 0;
    int data_length = packet_data.length();
    const char* data = packet_data.c_str();

    // 分段傳送數據
    while (total_sent < data_length) {
        int sent = send(client_socket, data + total_sent, data_length - total_sent, 0);
        if (sent == SOCKET_ERROR) {
            std::cerr << "傳送失敗: " << WSAGetLastError() << std::endl;
            return false;
        }
        total_sent += sent;
    }
    return true;
}

// 接收封包字串
std::string NetworkServer::receive() {
    std::string received_data;
    char buffer[1024] = {0};
    int bytes_received;
    int max_attempts = 10;
    int attempts = 0;

    // 循環接收直到收到確認訊息或達到最大嘗試次數
    while (attempts < max_attempts) {
        bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received == SOCKET_ERROR) {
            std::cerr << "接收失敗，錯誤碼: " << WSAGetLastError() << std::endl;
            return "";
        }
        if (bytes_received == 0) {
            if (received_data.empty()) {
                std::cerr << "客戶端關閉連線，無數據接收" << std::endl;
            }
            break;
        }
        received_data.append(buffer, bytes_received);
        if (received_data.find("JSON|ACK") != std::string::npos) {
            break;
        }
        attempts++;
    }

    return received_data;
}

// 清理 socket 和 Winsock 資源
void NetworkServer::cleanup() {
    if (client_socket != INVALID_SOCKET) {
        closesocket(client_socket);
    }
    if (server_fd != INVALID_SOCKET) {
        closesocket(server_fd);
    }
    WSACleanup();
}
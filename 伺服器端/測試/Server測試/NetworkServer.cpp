#include "NetworkServer.h"
#include <ws2tcpip.h>
#include <iostream>

NetworkServer::NetworkServer(int port) : port(port), server_fd(INVALID_SOCKET), client_socket(INVALID_SOCKET), initialized(false), addrlen(sizeof(address)) {
    ZeroMemory(&address, sizeof(address));
}

NetworkServer::~NetworkServer() {
    cleanup();
}

bool NetworkServer::initialize() {
    // 初始化 Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup 失敗: " << WSAGetLastError() << std::endl;
        return false;
    }

    // 創建 socket
    if (!createSocket()) {
        WSACleanup();
        return false;
    }

    // 設置 socket 選項
    if (!setSocketOptions()) {
        cleanup();
        return false;
    }

    // 綁定 socket
    if (!bindSocket()) {
        cleanup();
        return false;
    }

    initialized = true;
    return true;
}

bool NetworkServer::createSocket() {
    server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd == INVALID_SOCKET) {
        std::cerr << "Socket 創建失敗: " << WSAGetLastError() << std::endl;
        return false;
    }
    return true;
}

bool NetworkServer::setSocketOptions() {
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) == SOCKET_ERROR) {
        std::cerr << "設置 socket 選項失敗: " << WSAGetLastError() << std::endl;
        return false;
    }
    return true;
}

bool NetworkServer::bindSocket() {
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        std::cerr << "綁定 socket 失敗: " << WSAGetLastError() << std::endl;
        return false;
    }
    return true;
}

bool NetworkServer::startListening() {
    if (!initialized) {
        std::cerr << "伺服器未初始化" << std::endl;
        return false;
    }

    if (listen(server_fd, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "監聽失敗: " << WSAGetLastError() << std::endl;
        return false;
    }
    std::cout << "伺服器正在監聽埠 " << port << "..." << std::endl;
    return true;
}

bool NetworkServer::acceptConnection() {
    client_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
    if (client_socket == INVALID_SOCKET) {
        std::cerr << "接受連線失敗: " << WSAGetLastError() << std::endl;
        return false;
    }
    std::cout << "客戶端連線成功" << std::endl;
    return true;
}

bool NetworkServer::sendPacket(const PacketInterface& packet) {
    std::string data = packet.encapsulate();
    int bytes_sent = send(client_socket, data.c_str(), data.length(), 0);
    if (bytes_sent == SOCKET_ERROR) {
        std::cerr << "傳送資料失敗: " << WSAGetLastError() << std::endl;
        return false;
    }
    return true;
}

std::string NetworkServer::receive() {
    char buffer[4096] = {0};
    int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received == SOCKET_ERROR || bytes_received == 0) {
        std::cerr << "接收資料失敗或連線關閉: " << WSAGetLastError() << std::endl;
        return "";
    }
    return std::string(buffer, bytes_received);
}

void NetworkServer::cleanup() {
    if (client_socket != INVALID_SOCKET) {
        closesocket(client_socket);
        client_socket = INVALID_SOCKET;
    }
    if (server_fd != INVALID_SOCKET) {
        closesocket(server_fd);
        server_fd = INVALID_SOCKET;
    }
    WSACleanup();
}
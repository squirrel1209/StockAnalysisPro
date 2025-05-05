// NetworkServer.cpp
#include "NetworkServer.h"

#include <ws2tcpip.h>

#include <cstdint>
#include <iostream>

#include "JsonPacket.h"

NetworkServer::NetworkServer(int port) : port(port), server_fd(INVALID_SOCKET), client_socket(INVALID_SOCKET), initialized(false), addrlen(sizeof(address)), running(false), json_data("") {
    ZeroMemory(&address, sizeof(address));
}

NetworkServer::~NetworkServer() {
    cleanup();
}

bool NetworkServer::initialize() {
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "[ERROR] WSAStartup 失敗: " << WSAGetLastError() << std::endl;
        return false;
    }
    if (!createSocket()) {
        WSACleanup();
        return false;
    }
    if (!setSocketOptions()) {
        cleanup();
        return false;
    }
    if (!bindSocket()) {
        cleanup();
        return false;
    }
    initialized = true;
    std::cout << "[INFO] Winsock 初始化成功" << std::endl;
    return true;
}

bool NetworkServer::createSocket() {
    server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd == INVALID_SOCKET) {
        std::cerr << "[ERROR] Socket 創建失敗: " << WSAGetLastError() << std::endl;
        return false;
    }
    return true;
}

bool NetworkServer::setSocketOptions() {
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) == SOCKET_ERROR) {
        std::cerr << "[ERROR] 設置 socket 選項失敗: " << WSAGetLastError() << std::endl;
        return false;
    }
    return true;
}

bool NetworkServer::bindSocket() {
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        std::cerr << "[ERROR] 綁定 socket 失敗: " << WSAGetLastError() << std::endl;
        return false;
    }
    return true;
}

bool NetworkServer::startListening() {
    if (!initialized) {
        std::cerr << "[ERROR] 伺服器未初始化" << std::endl;
        return false;
    }
    if (listen(server_fd, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "[ERROR] 監聽失敗: " << WSAGetLastError() << std::endl;
        return false;
    }
    std::cout << "[INFO] 伺服器正在監聽埠 " << port << "..." << std::endl;
    return true;
}

bool NetworkServer::acceptConnection() {
    client_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
    if (client_socket == INVALID_SOCKET) {
        std::cerr << "[ERROR] 接受連線失敗: " << WSAGetLastError() << std::endl;
        return false;
    }
    std::cout << "[INFO] 客戶端連線成功，socket: " << client_socket << std::endl;
    return true;
}

void NetworkServer::run() {
    if (!startListening()) {
        return;
    }
    running = true;
    while (running) {
        std::cout << "[INFO] 等待客戶端連線..." << std::endl;
        if (acceptConnection()) {
            u_long mode = 1;
            ioctlsocket(client_socket, FIONBIO, &mode);

            bool keep_connection = true;
            bool data_sent = false;  // 跟踪是否已傳送數據

            while (keep_connection && running) {
                if (!json_data.empty() && !data_sent) {
                    JsonPacket packet(json_data);
                    if (sendPacket(client_socket, packet)) {
                        std::cout << "[INFO] [SOCKET " << client_socket << "] 傳送 JSON 陣列成功" << std::endl;
                        data_sent = true;  // 標記數據已傳送
                    } else {
                        std::cerr << "[ERROR] 傳送失敗，關閉連線" << std::endl;
                        keep_connection = false;
                    }
                }

                // 檢查客戶端是否斷開
                char buffer[1];
                int result = recv(client_socket, buffer, 1, MSG_PEEK);
                if (result == 0 || (result == SOCKET_ERROR && WSAGetLastError() == WSAECONNRESET)) {
                    std::cout << "[INFO] [SOCKET " << client_socket << "] 客戶端斷開連線" << std::endl;
                    keep_connection = false;
                }

                Sleep(3000);
            }

            closesocket(client_socket);
            client_socket = INVALID_SOCKET;
            std::cout << "[INFO] [SOCKET " << client_socket << "] 客戶端連線已關閉" << std::endl;
        }
    }
}

void NetworkServer::stop() {
    running = false;
    cleanup();
}

void NetworkServer::setJsonData(const std::string& jsonData) {
    json_data = jsonData;
}

bool NetworkServer::sendPacket(const PacketInterface& packet) {
    return sendPacket(client_socket, packet);
}

bool NetworkServer::sendPacket(SOCKET client, const PacketInterface& packet) {
    std::string data = packet.encapsulate();
    uint32_t data_len = htonl(data.length());

    // 傳送數據長度
    int total_sent = 0;
    int len_size = sizeof(data_len);
    while (total_sent < len_size) {
        int sent = send(client, (char*)&data_len + total_sent, len_size - total_sent, 0);
        if (sent == SOCKET_ERROR) {
            std::cerr << "[ERROR] 傳送資料長度失敗: " << WSAGetLastError() << std::endl;
            return false;
        }
        total_sent += sent;
    }

    // 傳送數據
    total_sent = 0;
    int data_size = data.length();
    while (total_sent < data_size) {
        int sent = send(client, data.c_str() + total_sent, data_size - total_sent, 0);
        if (sent == SOCKET_ERROR) {
            std::cerr << "[ERROR] 傳送資料失敗: " << WSAGetLastError() << std::endl;
            return false;
        }
        total_sent += sent;
    }

    std::cout << "[INFO] 成功傳送數據，長度: " << data_size << std::endl;
    return true;
}

SOCKET NetworkServer::getClientSocket() const {
    return client_socket;
}

std::string NetworkServer::receive() {
    uint32_t data_len = 0;
    int ret = recv(client_socket, (char*)&data_len, sizeof(data_len), 0);
    if (ret <= 0) {
        std::cerr << "[ERROR] 接收資料長度失敗: " << WSAGetLastError() << std::endl;
        return "";
    }
    data_len = ntohl(data_len);

    std::string result;
    result.resize(data_len);

    int total_received = 0;
    while (total_received < data_len) {
        int bytes_received = recv(client_socket, &result[total_received], data_len - total_received, 0);
        if (bytes_received <= 0) {
            std::cerr << "[ERROR] 接收資料失敗或連線關閉: " << WSAGetLastError() << std::endl;
            return "";
        }
        total_received += bytes_received;
    }
    return result;
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
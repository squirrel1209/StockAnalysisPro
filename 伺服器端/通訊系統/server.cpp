#include <iostream>
#include <fstream>
#include <string>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib") // 連結 Winsock 庫

#define PORT 8080
#define BUFFER_SIZE 1024

// 讀取 JSON 文件
std::string readJsonFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "無法開啟文件: " << filename << std::endl;
        return "";
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    return content;
}

int main() {
    WSADATA wsaData;
    SOCKET server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // 初始化 Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Winsock 初始化失敗，錯誤碼: " << WSAGetLastError() << std::endl;
        return -1;
    }

    // 創建 socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        std::cerr << "Socket 創建失敗，錯誤碼: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return -1;
    }

    // 設置 socket 選項
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) == SOCKET_ERROR) {
        std::cerr << "Setsockopt 失敗，錯誤碼: " << WSAGetLastError() << std::endl;
        closesocket(server_fd);
        WSACleanup();
        return -1;
    }

    // 綁定地址
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        std::cerr << "綁定失敗，錯誤碼: " << WSAGetLastError() << std::endl;
        closesocket(server_fd);
        WSACleanup();
        return -1;
    }

    // 監聽
    if (listen(server_fd, 3) == SOCKET_ERROR) {
        std::cerr << "監聽失敗，錯誤碼: " << WSAGetLastError() << std::endl;
        closesocket(server_fd);
        WSACleanup();
        return -1;
    }

    std::cout << "伺服器正在監聽端口 " << PORT << "..." << std::endl;

    // 接受客戶端連線
    if ((new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) == INVALID_SOCKET) {
        std::cerr << "接受連線失敗，錯誤碼: " << WSAGetLastError() << std::endl;
        closesocket(server_fd);
        WSACleanup();
        return -1;
    }

    // 讀取 JSON 文件
    std::string json_data = readJsonFile("stock_data_AAPL.json");
    std::cout << "讀取的 JSON 數據: " << json_data << std::endl;
    if (json_data.empty()) {
        closesocket(new_socket);
        closesocket(server_fd);
        WSACleanup();
        return -1;
    }

    // 發送數據長度
    uint32_t data_length = htonl(json_data.size());
    if (send(new_socket, (char*)&data_length, sizeof(data_length), 0) == SOCKET_ERROR) {
        std::cerr << "發送數據長度失敗，錯誤碼: " << WSAGetLastError() << std::endl;
        closesocket(new_socket);
        closesocket(server_fd);
        WSACleanup();
        return -1;
    }

    // 發送 JSON 數據
    size_t sent = 0;
    while (sent < json_data.size()) {
        int bytes_sent = send(new_socket, json_data.c_str() + sent, json_data.size() - sent, 0);
        if (bytes_sent == SOCKET_ERROR) {
            std::cerr << "發送數據失敗，錯誤碼: " << WSAGetLastError() << std::endl;
            break;
        }
        sent += bytes_sent;
    }

    std::cout << "JSON 數據已發送，長度: " << json_data.size() << " 字節" << std::endl;

    // 關閉 socket
    closesocket(new_socket);
    closesocket(server_fd);
    WSACleanup();
    return 0;
}
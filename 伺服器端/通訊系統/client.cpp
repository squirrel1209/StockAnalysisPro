#include <iostream>
#include <fstream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h> // 加入此標頭以支援 inet_pton
#include <windows.h>

#pragma comment(lib, "ws2_32.lib") // 保留以支援 MSVC，MinGW 使用 -lws2_32

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    WSADATA wsaData;
    SOCKET sock = INVALID_SOCKET;
    struct sockaddr_in serv_addr;

    // 初始化 Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Winsock 初始化失敗，錯誤碼: " << WSAGetLastError() << std::endl;
        return -1;
    }

    // 創建 socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        std::cerr << "Socket 創建失敗，錯誤碼: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return -1;
    }

    // 設置伺服器地址
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "無效地址，錯誤碼: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    // 連接到伺服器
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
        std::cerr << "連線失敗，錯誤碼: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    // 接收數據長度
    uint32_t data_length;
    int bytes_received = recv(sock, (char*)&data_length, sizeof(data_length), 0);
    if (bytes_received <= 0) {
        std::cerr << "接收數據長度失敗，錯誤碼: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return -1;
    }
    data_length = ntohl(data_length);

    // 接收 JSON 數據
    std::string json_data;
    char buffer[BUFFER_SIZE];
    size_t total_received = 0;

    while (total_received < data_length) {
        int bytes_received = recv(sock, buffer, std::min(BUFFER_SIZE, (int)(data_length - total_received)), 0);
        if (bytes_received <= 0) {
            std::cerr << "接收數據失敗，錯誤碼: " << WSAGetLastError() << std::endl;
            break;
        }
        json_data.append(buffer, bytes_received);
        total_received += bytes_received;
    }

    std::cout << "接收到 JSON 數據，長度: " << json_data.size() << " 字節" << std::endl;

    // 將接收到的數據保存到文件
    std::ofstream out_file("received_stock_data_AAPL.json");
    if (!out_file.is_open()) {
        std::cerr << "無法創建輸出文件" << std::endl;
        closesocket(sock);
        WSACleanup();
        return -1;
    }
    out_file << json_data;
    out_file.close();

    std::cout << "JSON 數據已保存到 received_stock_data_AAPL.json" << std::endl;

    // 關閉 socket
    closesocket(sock);
    WSACleanup();
    return 0;
}
// === main.cpp ===
#include "FileReader.h"
#include "NetworkServer.h"
#include "JsonPacket.h"
#include "task_pool.h"
#include <iostream>

#define PORT 8080

int main() {
    FileReader fileReader;
    std::string json_data = fileReader.readJsonFile("stock_data_AAPL.json");

    if (json_data.empty()) {
        std::cerr << "[ERROR] 讀取 JSON 檔案失敗" << std::endl;
        return -1;
    }

    std::cout << "[INFO] 讀取的 JSON 資料: " << json_data << std::endl;

    NetworkServer server(PORT);

    if (!server.initialize()) {
        std::cerr << "[ERROR] 伺服器初始化失敗" << std::endl;
        return -1;
    }

    if (!server.startListening()) {
        std::cerr << "[ERROR] 伺服器監聽失敗" << std::endl;
        return -1;
    }

    TaskPool tp;

    while (true) {
        if (!server.acceptConnection()) {
            std::cerr << "[ERROR] 接受連線失敗" << std::endl;
            continue;
        }

        SOCKET client = server.getClientSocket();

        tp.AddTask([client, json_data, &server]() {
            JsonPacket packet(json_data);
            if (server.sendPacket(client, packet)) {
                std::cout << "[INFO] [SOCKET " << client << "] 資料傳送完成" << std::endl;
            } else {
                std::cerr << "[ERROR] [SOCKET " << client << "] 傳送資料失敗" << std::endl;
            }
            closesocket(client);
            std::cout << "[INFO] [SOCKET " << client << "] 客戶端連線關閉" << std::endl;
        });
        
    }

    return 0;
}
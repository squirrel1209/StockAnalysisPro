#include "FileReader.h"
#include "NetworkServer.h"
#include "JsonPacket.h"
#include "task_pool.h"
#include <iostream>
#include <vector>

#define PORT 8080

int main() {
    FileReader fileReader;

    // 🛠 讀取五個 JSON 檔案
    std::vector<std::string> filenames = {
        "stock_data_AAPL.json",
        "stock_data_AMZN.json",
        "stock_data_GOOGL.json",
        "stock_data_MSFT.json",
        "stock_data_NVDA.json"
    };

    std::vector<std::string> json_data_list;

    for (const auto& filename : filenames) {
        std::string json = fileReader.readJsonFile(filename);
        if (json.empty()) {
            std::cerr << "[ERROR] 讀取檔案失敗: " << filename << std::endl;
            return -1;
        }
        json_data_list.push_back(json);
    }

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

        tp.AddTask([client, json_data_list, &server]() {
            // ✅ 直接一個一個傳，不傳總筆數了
            for (const auto& json_data : json_data_list) {
                JsonPacket packet(json_data);

                if (server.sendPacket(client, packet)) {
                    std::cout << "[INFO] [SOCKET " << client << "] 傳送一筆資料成功" << std::endl;
                } else {
                    std::cerr << "[ERROR] [SOCKET " << client << "] 傳送資料失敗" << std::endl;
                    break;
                }
            }

            closesocket(client);
            std::cout << "[INFO] [SOCKET " << client << "] 客戶端連線關閉" << std::endl;
        });
    }

    return 0;
}

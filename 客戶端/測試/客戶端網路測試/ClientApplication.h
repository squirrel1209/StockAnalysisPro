#ifndef CLIENT_APPLICATION_H
#define CLIENT_APPLICATION_H

#include "NetworkClient.h"
#include "FileHandler.h"
#include <string>

// 處理客戶端應用程式邏輯
class ClientApplication {
public:
    // 以伺服器 IP、埠號和輸出檔案名稱初始化
    ClientApplication(const std::string& ip, int port, const std::string& output_filename);

    // 執行客戶端邏輯
    bool run();

private:
    // 網路客戶端
    NetworkClient network_client_;

    // 檔案處理器
    FileHandler file_handler_;
};

#endif // CLIENT_APPLICATION_H
#ifndef CLIENT_APPLICATION_H
#define CLIENT_APPLICATION_H

#include "FileHandler.h"
#include "NetworkClient.h"

// ClientApplication 類別：協調網路連線和檔案操作，執行應用程式的核心邏輯。
class ClientApplication {
public:
    // 建構子：使用伺服器 IP、埠號和輸出檔案名稱初始化應用程式。
    ClientApplication(const std::string& ip, int port, const std::string& output_filename);

    // 執行應用程式的主要邏輯（初始化、連線、接收資料、儲存資料）。
    // 成功則回傳 true，失敗則回傳 false。
    bool run();

private:
    // 網路客戶端物件，用於處理網路操作。
    NetworkClient network_client_;

    // 檔案處理物件，用於處理檔案寫入。
    FileHandler file_handler_;
};

#endif // CLIENT_APPLICATION_H
#include <iostream>
#include "ClientApplication.h"

int main()
{
    // 建立 ClientApplication，指向本地 127.0.0.1:8080
    ClientApplication client("127.0.0.1", 8080);

    // 從伺服器取得封包
    auto packet = client.fetchPacket();
    if (!packet) {
        std::cerr << "連線或資料錯誤！" << std::endl;
        return 1;
    }

    // 成功取得封包後，印出 payload 內容
    std::cout << "封包類型: " << packet->getDataType() << std::endl;
    std::cout << "封包內容:\n" << packet->getPayload() << std::endl;

    return 0;
}

#include <iostream>
#include "ClientApplication.h"

int main()
{
    ClientApplication client("127.0.0.1", 8080);

    const int totalFiles = 5; // ✅ 直接寫死 5筆！

    for (int i = 0; i < totalFiles; ++i) {
        auto packet = client.fetchPacket();
        if (!packet) {
            std::cerr << "[ERROR] 第 " << (i + 1) << " 筆封包接收失敗，提前結束。" << std::endl;
            break;
        }
        std::cout << "=== 第 " << (i + 1) << " 筆封包 ===" << std::endl;
        std::cout << "封包類型: " << packet->getDataType() << std::endl;
        std::cout << "封包內容:\n" << packet->getPayload() << std::endl;
    }

    return 0;
}

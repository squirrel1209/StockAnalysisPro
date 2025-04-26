#include "FileHandler.h"
#include <fstream>
#include <iostream>

// 建構子：設定輸出檔案的名稱。
FileHandler::FileHandler(const std::string& filename) : filename_(filename) {}

// 將資料寫入由 filename_ 指定的檔案。
// 若檔案寫入成功則回傳 true，若無法開啟檔案則回傳 false。
bool FileHandler::write(const std::string& data) {
    // 以寫入模式開啟輸出檔案。
    std::ofstream out_file(filename_);
    if (!out_file.is_open()) {
        // 若無法開啟檔案，輸出錯誤訊息。
        std::cerr << "無法創建輸出文件: " << filename_ << std::endl;
        return false;
    }
    // 將資料寫入檔案。
    out_file << data;
    // 關閉檔案。
    out_file.close();
    return true;
}
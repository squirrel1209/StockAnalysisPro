#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <string>

// FileHandler 類別：負責管理檔案寫入操作，用於儲存接收到的資料。
class FileHandler {
public:
    // 建構子：使用輸出檔案名稱初始化 FileHandler。
    FileHandler(const std::string& filename);

    // 將提供的資料寫入檔案。
    // 成功則回傳 true，失敗則回傳 false。
    bool write(const std::string& data);

private:
    // 儲存輸出檔案的名稱。
    std::string filename_;
};

#endif // FILE_HANDLER_H
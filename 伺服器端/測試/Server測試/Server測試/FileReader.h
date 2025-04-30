#ifndef FILE_READER_H
#define FILE_READER_H

#include <string>

class FileReader {
public:
    // 默認構造函數
    FileReader() = default;
    // 析構函數
    ~FileReader() = default;

    // 讀取 JSON 文件並返回其內容
    std::string readJsonFile(const std::string& filename) const;
};

#endif // FILE_READER_H
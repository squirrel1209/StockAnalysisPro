#include "FileReader.h"
#include <fstream>
#include <iostream>
#include <iterator>

// 讀取 JSON 文件的實現
std::string FileReader::readJsonFile( const std::string& filename ) const {
    // 打開指定文件
    std::ifstream file( filename );

    // 檢查文件是否成功打開
    if ( !file.is_open() ) {
        std::cerr << "無法開啟文件: " << filename << std::endl;
        return "";
    } // end if

    // 將文件內容讀入字符串
    std::string content( (std::istreambuf_iterator<char>(file) ), std::istreambuf_iterator<char>() );
    
    // 關閉文件
    file.close();
    // 返回文件內容
    return content;
} // end readJsonFile
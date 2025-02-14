﻿//
// Created by m1393 on 2025/1/17.
//
#define BIT7Z_AUTO_FORMAT

#include <bit7z/bit7z.hpp>
#include <bit7z/bitextractor.hpp>
#include <iostream>

int main() {
    try {
        // 指定 7z.dll 的路径
        const bit7z::Bit7zLibrary lib{ R"(C:\Program Files\7-Zip\7z.dll)" };

        // **创建 RAR 格式的 BitInFormat 实例**
        bit7z::BitInFormat rarFormat{ 0x03 };
        // 创建解压器
        bit7z::BitExtractor<std::string> extractor{ lib,bit7z::BitFormat::Auto};

        // 要解压的文件
        std::string rarFile = "D:/test.7z";

        // 目标解压目录
        std::string outputDir = "D:/test1";

        bit7z::BitArchiveReader reader(lib,rarFile, bit7z::BitFormat::Auto);

        extractor.setPassword("11");
        try {
            extractor.test(rarFile); // 仅测试，不解压
        } catch (const bit7z::BitException& e) {
            std::string errorMsg = e.what();
            if (errorMsg.find("wrong password") != std::wstring::npos ||
                errorMsg.find("incorrect password") != std::wstring::npos ||
                    errorMsg.find("password is required but none was provided") != std::wstring::npos) {
                std::cout<<errorMsg<<std::endl;
                std::cout << "该文件需要密码！" << std::endl;
            } else {
                std::cout << "文件测试失败: " << errorMsg << std::endl;
            }
        }


        uint64_t totalSize = reader.size();

        extractor.setProgressCallback([&](uint64_t progress){
//                std::cout<<progress<<" "<<totalSize<<std::endl;
            std::cout<<(float(progress)/totalSize*100)<<std::endl;
            return true;
        });



        // 执行解压
//        extractor.extract(rarFile, outputDir);

        std::cout << "RAR 解压完成！" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "解压失败: " << e.what() << std::endl;
    }

    return 0;
}

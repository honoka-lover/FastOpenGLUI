﻿//
// Created by m1393 on 2025/1/11.
//

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "CommonFunc.h"

#include "archive_entry.h"
#include "archive.h"
#include "iostream"
#include "fstream"
#include "Shlwapi.h"
#include <ShlObj_core.h>


// 确保目标目录存在
bool EnsureDirectoryExists(const fs::path& directoryPath) {
    try {
        if (!fs::exists(directoryPath)) {
            return fs::create_directories(directoryPath);
        }
        return true;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Failed to create directory: " << e.what() << std::endl;
        return false;
    }
}

void extract_7z(const fs::path& archive_path, const fs::path& output_dir,const std::function<void(float)> & callback) {
    struct archive* archive;
    struct archive* extract;
    struct archive_entry* entry;
    int result;

    // 创建 archive 对象
    archive = archive_read_new();
    extract = archive_write_disk_new();

    if (!archive || !extract) {
        std::cerr << "Failed to allocate archive objects\n";
        return;
    }

    // 支持 7z 格式
    archive_read_support_format_7zip(archive);
    archive_read_support_filter_all(archive);

    // 打开归档文件
    if ((result = archive_read_open_filename(archive, archive_path.string().c_str(), 10240)) != ARCHIVE_OK) {
        std::cerr << "Failed to open archive: " << archive_error_string(archive) << "\n";
        archive_read_free(archive);
        archive_write_free(extract);
        return;
    }

    // 获取归档文件的总大小
    size_t total_size = 0;
    struct archive_entry* temp_entry;
    while ((result = archive_read_next_header(archive, &temp_entry)) == ARCHIVE_OK) {
        total_size += archive_entry_size(temp_entry);
    }

    if (result != ARCHIVE_EOF) {
        std::cerr << "Error reading archive: " << archive_error_string(archive) << "\n";
        archive_read_free(archive);
        archive_write_free(extract);
        return;
    }

    // 重置归档指针
    archive_read_close(archive);  // 关闭当前归档对象
    archive = archive_read_new();  // 重新创建一个新的归档对象
    archive_read_support_format_7zip(archive);  // 重新设置格式支持
    archive_read_support_filter_all(archive);  // 重新设置过滤器
    if ((result = archive_read_open_filename(archive, archive_path.string().c_str(), 10240)) != ARCHIVE_OK) {
        std::cerr << "Failed to open archive: " << archive_error_string(archive) << "\n";
        archive_read_free(archive);
        archive_write_free(extract);
        return;
    }

    size_t processed_size = 0;

    // 遍历归档文件
    while ((result = archive_read_next_header(archive, &entry)) == ARCHIVE_OK) {
        const char* current_file = archive_entry_pathname(entry);

        // 设置解压路径
        fs::path full_output_path = output_dir / current_file;
        archive_entry_set_pathname(entry, full_output_path.string().c_str());

        // 解压
        if ((result = archive_write_header(extract, entry)) != ARCHIVE_OK) {
            std::cerr << "Failed to write header: " << archive_error_string(extract) << "\n";
        } else if (archive_entry_size(entry) > 0) {
            const void* buff;
            size_t size;
            la_int64_t offset;

            while ((result = archive_read_data_block(archive, &buff, &size, &offset)) != ARCHIVE_EOF) {
                if (result != ARCHIVE_OK) {
                    std::cerr << "Error reading data block: " << archive_error_string(archive) << "\n";
                    break;
                }

                if ((result = archive_write_data_block(extract, buff, size, offset)) != ARCHIVE_OK) {
                    std::cerr << "Error writing data block: " << archive_error_string(extract) << "\n";
                    break;
                }

                // 更新进度
                processed_size += size;
                callback(static_cast<float>(processed_size) / static_cast<float>(total_size) * 100);
            }
        }

        // 关闭条目
        if ((result = archive_write_finish_entry(extract)) != ARCHIVE_OK) {
            std::cerr << "Error finishing entry: " << archive_error_string(extract) << "\n";
        }
    }

    if (result != ARCHIVE_EOF) {
        std::cerr << "Error reading archive: " << archive_error_string(archive) << "\n";
    }

    // 释放资源
    archive_read_free(archive);
    archive_write_free(extract);
}

void Extract7zResourceWithProgress(int resourcesId,const fs::path& outPath,const std::function<void(float)> & callback) {
    // 获取 AppData 缓存路径
    char* appDataPath = nullptr;
    size_t len = 0;
    if (_dupenv_s(&appDataPath, &len, "APPDATA") != 0 || appDataPath == nullptr) {
        std::cerr << "Failed to get AppData path." << std::endl;
        return;
    }
    fs::path tempFilePath = fs::path(appDataPath) / "Temp7zCache.7z";
    free(appDataPath);

    // 确保缓存路径所在目录存在
    if (!EnsureDirectoryExists(tempFilePath.parent_path())) {
        std::cerr << "Failed to ensure AppData cache directory exists." << std::endl;
        return;
    }

    // 找到资源
    HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(resourcesId), TEXT("7Z"));
    if (!hRes) {
        std::cerr << "Failed to find resource." << std::endl;
        return;
    }

    HGLOBAL hResData = LoadResource(NULL, hRes);
    if (!hResData) {
        std::cerr << "Failed to load resource." << std::endl;
        return;
    }

    DWORD resSize = SizeofResource(NULL, hRes);
    void* pResData = LockResource(hResData);
    if (!pResData) {
        std::cerr << "Failed to lock resource." << std::endl;
        return;
    }

    // 写入到缓存文件
    std::ofstream cacheFile(tempFilePath, std::ios::binary);
    if (!cacheFile) {
        std::cerr << "Failed to open cache file for writing: " << tempFilePath << std::endl;
        return;
    }

    const size_t chunkSize = 64*1024; // 每次写入的字节数
    const char* data = static_cast<const char*>(pResData);
    size_t writtenBytes = 0;

    while (writtenBytes < resSize) {
        size_t bytesToWrite = std::min(chunkSize, resSize - writtenBytes);
        cacheFile.write(data + writtenBytes, bytesToWrite);
        writtenBytes += bytesToWrite;

        // 更新进度
        int progress = static_cast<int>((static_cast<double>(writtenBytes) / resSize) * 100);
        std::cout << "Writing to cache: " << progress << "%\r" << std::flush;

        // 模拟延迟（仅用于演示进度）

    }

    cacheFile.close();
    std::cout << "Cache file written: " << tempFilePath << std::endl;

    extract_7z(tempFilePath, outPath,callback);

    // 删除缓存文件
    if (fs::exists(tempFilePath)) {
        fs::remove(tempFilePath);
        std::cout << "Cache file deleted: " << tempFilePath << std::endl;
    }
}

// 辅助函数：将宽字符（wchar_t）转换为 UTF-8
std::string wcharToUtf8(const wchar_t* wideString) {
    if (!wideString) return "";
    int sizeRequired = WideCharToMultiByte(CP_UTF8, 0, wideString, -1, nullptr, 0, nullptr, nullptr);
    if (sizeRequired <= 0) return "";
    std::string utf8String(sizeRequired - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wideString, -1, &utf8String[0], sizeRequired, nullptr, nullptr);
    return utf8String;
}


std::filesystem::path selectFolderUsingIFileDialog(const std::filesystem::path& defaultFolder) {
    HRESULT hr = CoInitialize(nullptr);  // 初始化 COM
    if (FAILED(hr)) return std::filesystem::path();

    IFileDialog* pFileDialog = nullptr;
    hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileDialog));
    if (SUCCEEDED(hr)) {
        DWORD options;
        pFileDialog->GetOptions(&options);
        pFileDialog->SetOptions(options | FOS_PICKFOLDERS);  // 设置为文件夹选择模式

        // 设置初始文件夹
        IShellItem* pDefaultFolder = nullptr;
        hr = SHCreateItemFromParsingName(defaultFolder.wstring().c_str(), nullptr, IID_PPV_ARGS(&pDefaultFolder));
        if (SUCCEEDED(hr)) {
            pFileDialog->SetFolder(pDefaultFolder);
            pDefaultFolder->Release();
        }

        hr = pFileDialog->Show(nullptr);  // 显示对话框
        if (SUCCEEDED(hr)) {
            IShellItem* pShellItem = nullptr;
            hr = pFileDialog->GetResult(&pShellItem);
            if (SUCCEEDED(hr)) {
                PWSTR folderPath = nullptr;
                hr = pShellItem->GetDisplayName(SIGDN_FILESYSPATH, &folderPath);
                if (SUCCEEDED(hr) && folderPath) {
                    std::filesystem::path result(folderPath);  // 使用宽字符直接构造路径
                    CoTaskMemFree(folderPath);  // 释放内存
                    pShellItem->Release();
                    pFileDialog->Release();
                    CoUninitialize();
                    return result;
                }
                pShellItem->Release();
            }
        }
        pFileDialog->Release();
    }
    CoUninitialize();
    return std::filesystem::path();
}

// 将字符串颜色转换为 glm::vec4
glm::vec4 colorStringToVec4(const std::string& color) {
    // 验证输入格式
    if (color.empty() || color[0] != '#' || (color.length() != 7 && color.length() != 9)) {
        throw std::invalid_argument("Color string must be in the format #RRGGBB or #RRGGBBAA");
    }

    auto hexToFloat = [](const std::string& hex) -> float {
        return std::stoi(hex, nullptr, 16) / 255.0f;
    };

    // 提取 R、G、B 分量
    float r = hexToFloat(color.substr(1, 2)); // Red
    float g = hexToFloat(color.substr(3, 2)); // Green
    float b = hexToFloat(color.substr(5, 2)); // Blue

    // 提取 A 分量（如果没有，默认为 1.0）
    float a = (color.length() == 9) ? hexToFloat(color.substr(7, 2)) : 1.0f;

    return glm::vec4(r, g, b, a);
}

// 加载纹理
void loadTexture(const std::string &path, GLuint &textureID)
{
    int width, height, channels;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &channels, 4);
    if (!data)
    {
        throw std::runtime_error("Failed to load texture: " + path);
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
}

void loadTextureFromResource(int resourceID, GLuint &textureID)
{
    // 声明宽度、高度和通道数
    int width, height, channels;

    // 获取当前模块的实例句柄
    HMODULE hModule = GetModuleHandle(nullptr);

    // 找到资源
    HRSRC hResource = FindResource(hModule, MAKEINTRESOURCE(resourceID), TEXT("IMAGE"));
    if (!hResource)
    {
        throw std::runtime_error("Failed to find resource with ID: " + std::to_string(resourceID));
    }

    // 加载资源到内存
    HGLOBAL hLoadedResource = LoadResource(hModule, hResource);
    if (!hLoadedResource)
    {
        throw std::runtime_error("Failed to load resource with ID: " + std::to_string(resourceID));
    }

    // 锁定资源获取数据指针
    LPVOID pLockedResource = LockResource(hLoadedResource);
    if (!pLockedResource)
    {
        throw std::runtime_error("Failed to lock resource with ID: " + std::to_string(resourceID));
    }

    // 获取资源大小
    DWORD resourceSize = SizeofResource(hModule, hResource);
    if (resourceSize == 0)
    {
        throw std::runtime_error("Resource size is 0 for ID: " + std::to_string(resourceID));
    }

    // 将资源数据拷贝到缓冲区
    std::vector<unsigned char> imageData(static_cast<unsigned char *>(pLockedResource),
                                         static_cast<unsigned char *>(pLockedResource) + resourceSize);

    // 使用 stb_image 从内存加载图像数据
    unsigned char *data = stbi_load_from_memory(imageData.data(), imageData.size(), &width, &height, &channels, 4);
    if (!data)
    {
        throw std::runtime_error("Failed to load texture from resource with ID: " + std::to_string(resourceID));
    }

    // 生成和绑定纹理
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 释放图像数据
    stbi_image_free(data);
}
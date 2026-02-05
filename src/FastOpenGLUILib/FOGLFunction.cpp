//
// Created by honoka on 2026/2/3.
//
#include "glad/glad.h"
#include <Windows.h>   // 可选，但显式更安全

#include <Shlwapi.h>
#include <ShlObj.h>
#include <TlHelp32.h>
#include <psapi.h>
#include <shellapi.h>

#include <algorithm>
#include "FOGLFunction.h"

#include <fstream>
#include <iostream>
#include "bit7z/bit7z.hpp"
// #include "resource.h"

namespace fs = std::filesystem;
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

void Extract7zResourceWithProgress(int rc7zId,int resourcesId,const fs::path& outPath,const std::function<void(float)> & callback) {
    // 获取 AppData 缓存路径
    const char* appDataPath = std::getenv("APPDATA");
    if (appDataPath == nullptr) {
        std::cerr << "Failed to get AppData path." << std::endl;
        return;
    }

    fs::path tempFilePath = fs::path(appDataPath) / "Temp7zCache.7z";

    // 确保缓存路径所在目录存在
    if (!EnsureDirectoryExists(tempFilePath.parent_path())) {
        std::cerr << "Failed to ensure AppData cache directory exists." << std::endl;
        return;
    }

    // 找到资源
    HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(resourcesId), TEXT("DATA"));
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
        size_t bytesToWrite = (chunkSize < (resSize - writtenBytes)) ? chunkSize : (resSize - writtenBytes);
        cacheFile.write(data + writtenBytes, bytesToWrite);
        writtenBytes += bytesToWrite;

        // 更新进度
        int progress = static_cast<int>((static_cast<double>(writtenBytes) / resSize) * 100);
        std::cout << "Writing to cache: " << progress << "%\r" << std::flush;
    }

    cacheFile.close();
    std::cout << "Cache file written: " << tempFilePath << std::endl;

    if(!exists(outPath))
        create_directories(outPath);
//    extract_7z(tempFilePath, outPath,callback);
    extract_7z_UseBit7z(rc7zId,tempFilePath, outPath,callback);
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

//ansi转u8
std::string ansiToUtf8(const char* ansi) {
    if (!ansi) return "";

    // ANSI -> UTF-16
    int wlen = MultiByteToWideChar(CP_ACP, 0, ansi, -1, nullptr, 0);
    if (wlen <= 0) return "";

    std::wstring wstr(wlen - 1, L'\0');
    MultiByteToWideChar(CP_ACP, 0, ansi, -1, &wstr[0], wlen);

    // UTF-16 -> UTF-8
    int u8len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (u8len <= 0) return "";

    std::string u8(u8len - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &u8[0], u8len, nullptr, nullptr);

    return u8;
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

// UTF-8 转 GB2312
std::string UTF8ToGB2312(const std::string& utf8Str) {
    // UTF-8 转 WideChar
    int wideCharSize = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, nullptr, 0);
    if (wideCharSize <= 0) {
        throw std::runtime_error("Failed to convert UTF-8 to WideChar");
    }
    std::wstring wideStr(wideCharSize, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, &wideStr[0], wideCharSize);

    // WideChar 转 GB2312
    int gb2312Size = WideCharToMultiByte(936, 0, wideStr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (gb2312Size <= 0) {
        throw std::runtime_error("Failed to convert WideChar to GB2312");
    }
    std::string gb2312Str(gb2312Size, 0);
    WideCharToMultiByte(936, 0, wideStr.c_str(), -1, &gb2312Str[0], gb2312Size, nullptr, nullptr);

    return gb2312Str;
}

// 删除快捷方式文件
bool DeleteShortcut(const std::wstring& shortcutPath) {
    if (DeleteFileW(shortcutPath.c_str())) {
        std::wcout << "Deleted shortcut: " << shortcutPath << std::endl;
        return true;
    } else {
        std::wcerr << "Failed to delete shortcut: " << shortcutPath << " Error: " << GetLastError() << std::endl;
        return false;
    }
}

// 遍历删除文件和目录，无法删除的跳过
void SafeRemoveAll(const fs::path& dirPath) {
    for (const auto& entry : fs::directory_iterator(dirPath)) {
        std::error_code ec; // 用于捕获删除时的错误

        if (fs::is_directory(entry)) {
            // 递归删除子目录
            SafeRemoveAll(entry.path());
            fs::remove(entry.path(), ec);
        } else {
            // 删除文件
            fs::remove(entry, ec);
        }

        if (ec) {
            // 如果发生错误，输出错误信息并跳过
            std::cerr << "Failed to delete: " << entry.path()
                      << ". Error: " << ec.message() << std::endl;
        }
    }

    // 最后删除自身目录
    std::error_code ec;
    fs::remove(dirPath, ec);
    if (ec) {
        std::cerr << "Failed to delete directory: " << dirPath
                  << ". Error: " << ec.message() << std::endl;
    }
}

// 根据进程名和路径查找并终止进程
bool KillProcessByPath(const std::string& exePath) {
    bool isKilled = false;

    // 创建快照
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to create process snapshot!" << std::endl;
        return false;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // 遍历进程快照
    if (Process32First(hSnapshot, &pe32)) {
        do {
            // 获取进程句柄
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
            if (hProcess) {
                char processPath[MAX_PATH];
                if (GetModuleFileNameExA(hProcess, NULL, processPath, MAX_PATH)) {
                    // 检查路径是否匹配
                    if (_stricmp(processPath, exePath.c_str()) == 0) {
                        // 终止进程
                        if (TerminateProcess(hProcess, 0)) {
                            std::cout << "Terminated process: " << processPath << std::endl;
                            isKilled = true;
                        } else {
                            std::cerr << "Failed to terminate process: " << processPath << std::endl;
                        }
                    }
                }
                CloseHandle(hProcess);
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    return isKilled;
}

void LaunchExe(const std::filesystem::path& targetProgramPath) {
    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    PROCESS_INFORMATION pi = { 0 };

    // 设置不显示控制台窗口
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    std::string path = "\"" + targetProgramPath.string() + "\"";
    if (CreateProcessA(
            nullptr,                        // 应用程序名称
            path.data(),                    // 命令行参数
            nullptr,                        // 进程安全属性
            nullptr,                        // 线程安全属性
            FALSE,                          // 是否继承句柄
            CREATE_NO_WINDOW,               // 创建时不显示窗口
            nullptr,                        // 环境变量
            nullptr,                        // 当前目录
            &si,                            // 启动信息
            &pi                             // 进程信息
    )) {
        // 等待进程结束
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        std::cerr << "Failed to launch program. Error: " << GetLastError() << std::endl;
    }
}


void deleteSelf() {
    char szBatchFile[MAX_PATH] = { 0 };
    char szCurrentExe[MAX_PATH] = { 0 };

    // 获取当前程序路径
    GetModuleFileNameA(NULL, szCurrentExe, MAX_PATH);

    // 创建一个临时批处理文件
    sprintf(szBatchFile, "%s_del.bat", szCurrentExe);
    FILE* pBatch = fopen(szBatchFile, "w");

    if (pBatch) {
        fprintf(pBatch,
                ":loop\n"
                "del /q \"%s\"\n"      // 尝试删除当前程序文件
                "if exist \"%s\" goto loop\n"  // 如果文件仍然存在，循环尝试
                "del /q \"%s\"\n",     // 删除批处理文件本身
                szCurrentExe, szCurrentExe, szBatchFile);

        fclose(pBatch);

        // 运行批处理文件
        ShellExecuteA(NULL, "open", szBatchFile, NULL, NULL, SW_HIDE);
    }

    ExitProcess(0); // 退出程序
}

// 获取 AppData 目录路径
std::string getAppDataPath() {
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, path))) {
        return std::string(path) + "\\Temp\\";  // 你的程序专用目录
    }
    return "";
}

// 从资源中提取文件
bool extractResourceToAppData(int resourceId, const std::string& outputFileName) {
    std::string appDataPath = getAppDataPath();
    if (appDataPath.empty()) {
        std::cerr << "无法获取 AppData 目录" << std::endl;
        return false;
    }

    // 确保目录存在
    CreateDirectoryA(appDataPath.c_str(), NULL);

    // 获取当前模块的实例句柄
    HMODULE hModule = GetModuleHandle(nullptr);

    // 资源处理
    HRSRC hRes = FindResource(hModule, MAKEINTRESOURCE(resourceId), TEXT("DATA"));
    if (!hRes) {
        std::cerr << "无法找到资源 ID: " << resourceId << std::endl;
        return false;
    }

    HGLOBAL hData = LoadResource(NULL, hRes);
    if (!hData) {
        std::cerr << "加载资源失败" << std::endl;
        return false;
    }

    DWORD dataSize = SizeofResource(NULL, hRes);
    void* pData = LockResource(hData);
    if (!pData || dataSize == 0) {
        std::cerr << "无法锁定资源" << std::endl;
        return false;
    }

    std::string outputFilePath = appDataPath + outputFileName;
    std::ofstream outFile(outputFilePath, std::ios::binary);
    if (!outFile) {
        std::cerr << "无法创建文件: " << outputFilePath << std::endl;
        return false;
    }

    outFile.write(static_cast<const char*>(pData), dataSize);
    outFile.close();

    std::cout << "资源提取成功: " << outputFilePath << std::endl;
    return true;
}

void extract_7z_UseBit7z(int rc7zId,const fs::path &archive_path, const fs::path &output_dir,
                         const std::function<void(float)> &callback) {
    const std::string fileName = "7z.dll";
    // 指定 7z.dll 的路径
    const std::string sevenZipPath = getAppDataPath()+"7z.dll";
    // if (extractResourceToAppData(RES_7Z_DLL, fileName)) {
    if (extractResourceToAppData(rc7zId, fileName)) {
        try {

            // 创建解压对象
            bit7z::Bit7zLibrary lib(sevenZipPath);
            bit7z::BitArchiveReader reader(lib,archive_path.string().c_str(), bit7z::BitFormat::SevenZip);

            bit7z::BitFileExtractor extractor(lib, bit7z::BitFormat::SevenZip);

            uint64_t totalSize = reader.size();

            extractor.setProgressCallback([&](uint64_t progress){
//                std::cout<<progress<<" "<<totalSize<<std::endl;
                callback(float(progress)/totalSize*100);
                return true;
            });
            // 执行解压
            extractor.extract(archive_path.string().c_str(), output_dir.string().c_str());

            std::cout << "解压完成！" << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "解压失败: " << e.what() << std::endl;
            return ;
        }
    } else {
        std::cerr << "解压失败" << std::endl;
    }

    if(fs::exists(sevenZipPath)){
        std::filesystem::remove(sevenZipPath);
    }
}

std::vector<uint32_t> utf8_to_codepoints(const std::u8string& utf8) {
    std::vector<uint32_t> codepoints;
    size_t i = 0;

    while (i < utf8.size()) {
        uint8_t c = static_cast<uint8_t>(utf8[i]);
        uint32_t cp = 0;
        size_t bytes = 0;

        if ((c & 0x80) == 0) {            // 1-byte
            cp = c;
            bytes = 1;
        } else if ((c & 0xE0) == 0xC0) {  // 2-byte
            cp = c & 0x1F;
            bytes = 2;
        } else if ((c & 0xF0) == 0xE0) {  // 3-byte
            cp = c & 0x0F;
            bytes = 3;
        } else if ((c & 0xF8) == 0xF0) {  // 4-byte
            cp = c & 0x07;
            bytes = 4;
        } else {
            // 非法 UTF-8 起始字节
            ++i;
            continue;
        }

        if (i + bytes > utf8.size()) break;

        for (size_t j = 1; j < bytes; ++j) {
            uint8_t cc = static_cast<uint8_t>(utf8[i + j]);
            cp = (cp << 6) | (cc & 0x3F);
        }

        codepoints.push_back(cp);
        i += bytes;
    }

    return codepoints;
}

// 简单 UTF-8 解码函数
std::vector<uint32_t> utf8_to_codepoints(const std::string& utf8) {
    std::vector<uint32_t> codepoints;
    size_t i = 0;
    while(i < utf8.size()) {
        uint8_t c = utf8[i];
        uint32_t cp = 0;
        size_t bytes = 0;
        if((c & 0x80) == 0) {            // 1-byte
            cp = c;
            bytes = 1;
        } else if((c & 0xE0) == 0xC0) { // 2-byte
            cp = c & 0x1F;
            bytes = 2;
        } else if((c & 0xF0) == 0xE0) { // 3-byte
            cp = c & 0x0F;
            bytes = 3;
        } else if((c & 0xF8) == 0xF0) { // 4-byte
            cp = c & 0x07;
            bytes = 4;
        } else {
            // 非法 UTF-8，跳过
            i++;
            continue;
        }

        if(i + bytes > utf8.size()) break; // 防溢出

        for(size_t j = 1; j < bytes; j++) {
            cp <<= 6;
            cp |= (utf8[i+j] & 0x3F);
        }

        codepoints.push_back(cp);
        i += bytes;
    }
    return codepoints;
}

std::vector<uint32_t> utf8_to_codepoints(std::span<const char8_t> utf8) {
    std::vector<uint32_t> codepoints;
    size_t i = 0;

    while (i < utf8.size()) {
        uint8_t c = utf8[i];
        uint32_t cp = 0;
        size_t bytes = 0;

        if ((c & 0x80) == 0) {           // 1-byte
            cp = c;
            bytes = 1;
        } else if ((c & 0xE0) == 0xC0) { // 2-byte
            cp = c & 0x1F;
            bytes = 2;
        } else if ((c & 0xF0) == 0xE0) { // 3-byte
            cp = c & 0x0F;
            bytes = 3;
        } else if ((c & 0xF8) == 0xF0) { // 4-byte
            cp = c & 0x07;
            bytes = 4;
        } else {
            // 非法 UTF-8，跳过
            i++;
            continue;
        }

        if (i + bytes > utf8.size()) break; // 防溢出

        for (size_t j = 1; j < bytes; j++) {
            cp <<= 6;
            cp |= (utf8[i + j] & 0x3F);
        }

        codepoints.push_back(cp);
        i += bytes;
    }

    return codepoints;
}

inline std::vector<TextFragment> parseShellColor(const std::string& s) {
    std::vector<TextFragment> out;
    glm::vec4 cur{1,1,1,1};
    std::string buf;

    for (size_t i=0;i<s.size();) {
        if (s[i]=='\x1b' && s[i+1]=='[') {
            if (!buf.empty()) out.push_back({buf,cur}), buf.clear();
            i+=2;
            int code = 0;
            while (isdigit(s[i])) code = code*10 + (s[i++]-'0');
            i++; // m
            if (code==31) cur={1,0,0,1};
            else if (code==32) cur={0,1,0,1};
            else if (code==0) cur={1,1,1,1};
        } else buf+=s[i++];
    }
    if (!buf.empty()) out.push_back({buf,cur});
    return out;
}
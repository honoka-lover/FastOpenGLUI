//
// Created by honoka on 2026/2/3.
//

#ifndef FASTOPENGLUI_FOGLFUNCTION_H
#define FASTOPENGLUI_FOGLFUNCTION_H

#include <string>
#include <vector>
#include <cstdint>
#include <filesystem>
#include <span>

#include "glm/vec4.hpp"

struct TextFragment {
    std::string text;
    glm::vec4 color;
};

// 简单 UTF-8 解码函数
std::vector<uint32_t> utf8_to_codepoints(const std::u8string& utf8);
std::vector<uint32_t> utf8_to_codepoints(const std::string& utf8);
std::vector<uint32_t> utf8_to_codepoints(std::span<const char8_t> utf8);
std::vector<TextFragment> parseShellColor(const std::string& s);

// 确保目标目录存在
bool EnsureDirectoryExists(const std::filesystem::path& directoryPath);

// 解压7z到指定目录
void extract_7z_UseBit7z(int rc7zId,const std::filesystem::path& archive_path, const std::filesystem::path& output_dir,const std::function<void(float)> & callback);

// 从资源文件解压7z到指定目录
void Extract7zResourceWithProgress(int rc7zId,int resourcesId,const std::filesystem::path& outPath, const std::function<void(float)> & callback);

// 辅助函数：将宽字符（wchar_t）转换为 UTF-8
std::string wcharToUtf8(const wchar_t* wideString);

//ansi转u8
std::string ansiToUtf8(const char* ansi);

// 将字符串颜色转换为 glm::vec4
glm::vec4 colorStringToVec4(const std::string& color);

// 打开指定文件夹
std::filesystem::path selectFolderUsingIFileDialog(const std::filesystem::path& defaultFolder);

void loadTexture(const std::string &path, unsigned int &textureID);
void loadTextureFromResource(int resourceID, unsigned int &textureID);

// UTF-8 转 GB2312
std::string UTF8ToGB2312(const std::string& utf8Str);

// 遍历删除文件和目录，无法删除的跳过
void SafeRemoveAll(const std::filesystem::path& dirPath);

// 删除快捷方式文件
bool DeleteShortcut(const std::wstring& shortcutPath);

// 根据进程名和路径查找并终止进程
bool KillProcessByPath(const std::string& exePath);

void LaunchExe(const std::filesystem::path& targetProgramPath);

void deleteSelf();

// 获取 AppData 目录路径
std::string getAppDataPath();

#endif //FASTOPENGLUI_FOGLFUNCTION_H
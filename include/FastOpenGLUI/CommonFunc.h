//
// Created by m1393 on 2025/1/11.
//

#ifndef FASTOPENGLUI_COMMONFUNC_H
#define FASTOPENGLUI_COMMONFUNC_H

#include "glad/glad.h"
#include "glm/gtc/matrix_transform.hpp"
#include "thread"
#include "filesystem"
#include "functional"
namespace fs = std::filesystem;

// 确保目标目录存在
bool EnsureDirectoryExists(const fs::path& directoryPath);

// 解压7z到指定目录
void extract_7z(const fs::path& archive_path, const fs::path& output_dir,const std::function<void(float)> & callback);

// 从资源文件解压7z到指定目录
void Extract7zResourceWithProgress(int resourcesId,const fs::path& outPath, const std::function<void(float)> & callback);

// 辅助函数：将宽字符（wchar_t）转换为 UTF-8
std::string wcharToUtf8(const wchar_t* wideString);

// 将字符串颜色转换为 glm::vec4
glm::vec4 colorStringToVec4(const std::string& color);

// 打开指定文件夹
std::filesystem::path selectFolderUsingIFileDialog(const std::filesystem::path& defaultFolder);

void loadTexture(const std::string &path, GLuint &textureID);
void loadTextureFromResource(int resourceID, GLuint &textureID);

// UTF-8 转 GB2312
std::string UTF8ToGB2312(const std::string& utf8Str);
#endif //FASTOPENGLUI_COMMONFUNC_H

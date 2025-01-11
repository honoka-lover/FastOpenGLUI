#ifndef REGEDITREADMODEL_H
#define REGEDITREADMODEL_H

#include <Windows.h>
#include <iostream>
#include <string>
#include <filesystem>

bool WriteToRegistry(HKEY hKeyRoot, const std::wstring &subKey, const std::wstring &valueName, const std::wstring &data);

bool ReadFromRegistry(HKEY hKeyRoot, const std::wstring &subKey, const std::wstring &valueName, std::wstring &data);

bool DeleteRegistryKey(HKEY hKeyRoot, const std::wstring &subKey);


void WriteInstallData(const std::filesystem::path installPath);

void DeleteInstallData();

// 提取文件名（去掉路径）
std::wstring GetProgramName();

#endif // REGEDITREADMODEL_H

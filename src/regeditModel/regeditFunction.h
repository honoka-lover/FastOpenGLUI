#ifndef REGEDITREADMODEL_H
#define REGEDITREADMODEL_H

#include <windows.h>
#include <iostream>
#include <string>

bool WriteToRegistry(HKEY hKeyRoot, const std::wstring &subKey, const std::wstring &valueName, const std::wstring &data);

bool ReadFromRegistry(HKEY hKeyRoot, const std::wstring &subKey, const std::wstring &valueName, std::wstring &data);

bool DeleteRegistryKey(HKEY hKeyRoot, const std::wstring &subKey);

#endif // REGEDITREADMODEL_H

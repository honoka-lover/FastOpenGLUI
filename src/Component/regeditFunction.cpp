#include "regeditFunction.h"

bool ReadFromRegistry(HKEY hKeyRoot, const std::wstring &subKey, const std::wstring &valueName, std::wstring &data)
{
    HKEY hKey;
    LONG result = RegOpenKeyExW(
        hKeyRoot,       // 根键
        subKey.c_str(), // 子键路径
        0,              // 选项
        KEY_READ,       // 读权限
        &hKey           // 返回的句柄
    );

    if (result != ERROR_SUCCESS)
    {
        std::wcerr << L"Failed to open key: " << result << std::endl;
        return false;
    }

    wchar_t buffer[256];
    DWORD bufferSize = sizeof(buffer);
    DWORD dataType;

    result = RegQueryValueExW(
        hKey,                             // 注册表句柄
        valueName.c_str(),                // 值名称
        nullptr,                          // 保留
        &dataType,                        // 数据类型
        reinterpret_cast<BYTE *>(buffer), // 数据缓冲区
        &bufferSize                       // 缓冲区大小
    );

    RegCloseKey(hKey);

    if (result != ERROR_SUCCESS)
    {
        std::wcerr << L"Failed to query value: " << result << std::endl;
        return false;
    }

    if (dataType != REG_SZ)
    {
        std::wcerr << L"Invalid data type." << std::endl;
        return false;
    }

    data = buffer;
    return true;
}

bool WriteToRegistry(HKEY hKeyRoot, const std::wstring &subKey, const std::wstring &valueName, const std::wstring &data)
{
    HKEY hKey;
    LONG result = RegCreateKeyExW(
        hKeyRoot,                // 根键
        subKey.c_str(),          // 子键路径
        0,                       // 保留
        nullptr,                 // 类类型
        REG_OPTION_NON_VOLATILE, // 非易失性
        KEY_WRITE,               // 写权限
        nullptr,                 // 默认安全属性
        &hKey,                   // 返回的句柄
        nullptr                  // 是否新创建的指针
    );

    if (result != ERROR_SUCCESS)
    {
        std::wcerr << L"Failed to open or create key: " << result << std::endl;
        return false;
    }

    result = RegSetValueExW(
        hKey,                                                   // 注册表句柄
        valueName.c_str(),                                      // 值名称
        0,                                                      // 保留
        REG_SZ,                                                 // 数据类型：字符串
        reinterpret_cast<const BYTE *>(data.c_str()),           // 数据
        static_cast<DWORD>((data.size() + 1) * sizeof(wchar_t)) // 数据大小
    );

    RegCloseKey(hKey);

    if (result != ERROR_SUCCESS)
    {
        std::wcerr << L"Failed to set value: " << result << std::endl;
        return false;
    }

    return true;
}

bool DeleteRegistryKey(HKEY hKeyRoot, const std::wstring &subKey)
{
    LONG result = RegDeleteKeyW(
        hKeyRoot,      // 根键
        subKey.c_str() // 要删除的子键路径
    );

    if (result == ERROR_SUCCESS)
    {
        std::wcout << L"Registry key deleted successfully!" << std::endl;
        return true;
    }
    else if (result == ERROR_ACCESS_DENIED)
    {
        std::wcerr << L"Access denied! Make sure you have permissions to delete this key." << std::endl;
    }
    else if (result == ERROR_CHILD_MUST_BE_VOLATILE)
    {
        std::wcerr << L"Key is not empty. Cannot delete non-empty key with this function." << std::endl;
    }
    else
    {
        std::wcerr << L"Failed to delete registry key. Error code: " << result << std::endl;
    }

    return false;
}

// 示例
//  int main() {
//      std::wstring subKey = L"SOFTWARE\\MyApp";
//      std::wstring valueName = L"MyValue";
//      std::wstring writeData = L"Hello, Registry!";
//      std::wstring readData;

//     if (WriteToRegistry(HKEY_CURRENT_USER, subKey, valueName, writeData)) {
//         std::wcout << L"Value written successfully!" << std::endl;
//     }

//     if (ReadFromRegistry(HKEY_CURRENT_USER, subKey, valueName, readData)) {
//         std::wcout << L"Value read successfully: " << readData << std::endl;
//     }

//     return 0;
// }

// int main() {
//     std::wstring subKey = L"SOFTWARE\\MyEmptyKey";

//     // 先创建一个空键以进行测试
//     HKEY hKey;
//     RegCreateKeyExW(HKEY_CURRENT_USER, subKey.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr);
//     RegCloseKey(hKey);

//     // 删除该空键
//     if (DeleteRegistryKey(HKEY_CURRENT_USER, subKey)) {
//         std::wcout << L"Empty registry key deleted successfully." << std::endl;
//     } else {
//         std::wcout << L"Failed to delete the empty registry key." << std::endl;
//     }

//     return 0;
// }

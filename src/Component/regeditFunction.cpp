#include "regeditFunction.h"
#include "soft_info.h"
#include <shlobj.h>
#include "CommonFunc.h"
#include <shlwapi.h>
#include <TlHelp32.h>
#include "psapi.h"
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

// 提取文件名（去掉路径）
std::wstring GetProgramName() {
    wchar_t path[MAX_PATH];
    if (GetModuleFileNameW(NULL, path, MAX_PATH) == 0) {
        return L""; // 获取失败
    }

    // 提取文件名部分
    std::wstring fullPath(path);
    size_t pos = fullPath.find_last_of(L"\\/");
    if (pos != std::string::npos) {
        return fullPath.substr(pos + 1); // 从最后一个斜杠之后提取
    }
    return fullPath; // 如果没有斜杠，直接返回完整路径
}



// 创建快捷方式函数
bool CreateShortcut(const std::filesystem::path& shortcutPath, const std::filesystem::path& targetPath, const std::string& description = "", const std::filesystem::path& iconPath = "") {
    // 初始化 COM 库
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        std::wcerr << L"Failed to initialize COM library." << std::endl;
        return false;
    }

    // 创建 IShellLink 对象
    IShellLink* pShellLink = nullptr;
    hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&pShellLink);
    if (FAILED(hr)) {
        std::wcerr << L"Failed to create IShellLink instance." << std::endl;
        CoUninitialize();
        return false;
    }

    // 设置快捷方式的目标路径
    pShellLink->SetPath(UTF8ToGB2312(targetPath.u8string()).c_str());
    // 设置快捷方式的描述
    if (!description.empty()) {
        pShellLink->SetDescription(description.c_str());
    }
    // 设置快捷方式的图标
    if (!iconPath.empty()) {
        pShellLink->SetIconLocation(iconPath.string().c_str(), 0);
    }

    // 查询 IPersistFile 接口
    IPersistFile* pPersistFile = nullptr;
    hr = pShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pPersistFile);
    if (FAILED(hr)) {
        std::wcerr << L"Failed to query IPersistFile interface." << std::endl;
        pShellLink->Release();
        CoUninitialize();
        return false;
    }

    // 保存快捷方式
    hr = pPersistFile->Save(shortcutPath.c_str(), TRUE);
    if (FAILED(hr)) {
        std::wcerr << L"Failed to save shortcut." << std::endl;
    }

    // 释放资源
    pPersistFile->Release();
    pShellLink->Release();
    CoUninitialize();

    return SUCCEEDED(hr);
}

void WriteInstallData(const std::filesystem::path installPath){
    const std::wstring REG_UNINST_NODE = L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"+ SOFT_TYPE;

    WriteToRegistry(HKEY_CURRENT_USER,L"SOFTWARE\\"+ SOFT_TYPE,L"installName",GetProgramName());

    WriteToRegistry(HKEY_LOCAL_MACHINE, REG_UNINST_NODE, L"UninstallString", installPath/"uninstall.exe");
    WriteToRegistry(HKEY_LOCAL_MACHINE, REG_UNINST_NODE, L"DisplayName", SOFT_NAME);
    WriteToRegistry(HKEY_LOCAL_MACHINE, REG_UNINST_NODE, L"DisplayVersion", SOFT_VERSION);
    WriteToRegistry(HKEY_LOCAL_MACHINE, REG_UNINST_NODE, L"DisplayIcon", installPath/"uninstall.exe");
    WriteToRegistry(HKEY_LOCAL_MACHINE, REG_UNINST_NODE, L"Publisher", SOFT_PUBLISHER);


    wchar_t desktopPath[MAX_PATH];
    wchar_t startMenuPath[MAX_PATH];
    // 获取桌面路径
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, desktopPath))) {
        std::wstring desktopShortcut = std::wstring(desktopPath) + L"\\" + SOFT_NAME+L".lnk";

        // 创建桌面快捷方式
        if(!CreateShortcut(desktopShortcut,(installPath/SOFT_EXENAME).string()))
            std::cout<<"创建桌面快捷方式失败";
    }

    // 获取开始菜单路径
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_STARTMENU, NULL, 0, startMenuPath))) {
        std::wstring startMenuShortcut = std::wstring(startMenuPath) + L"\\" + SOFT_NAME+L".lnk";

        // 创建开始菜单快捷方式
        if(!CreateShortcut(startMenuShortcut,installPath/SOFT_EXENAME))
            std::cout<<"创建开始菜单快捷方式失败";
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

// 去除字符串末尾的 "\uninstall.exe"
std::wstring RemoveUninstallSuffix(const std::wstring& input) {
    const std::wstring suffix = L"\\uninstall.exe";
    size_t pos = input.rfind(suffix); // 找到指定后缀的位置
    if (pos != std::string::npos && pos == input.size() - suffix.size()) {
        return input.substr(0, pos); // 去掉后缀
    }
    return input; // 如果后缀不存在，则返回原字符串
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

void DeleteInstallData(){
    const std::wstring REG_UNINST_NODE = L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"+ SOFT_TYPE;

    std::wstring uninstallProgramPath;
    bool needDelete = ReadFromRegistry(HKEY_LOCAL_MACHINE,REG_UNINST_NODE,L"UninstallString",uninstallProgramPath);

    if(!needDelete)
        return;

    std::wstring installPath = RemoveUninstallSuffix(uninstallProgramPath);
    fs::path installpt = installPath + L"\\" +SOFT_EXENAME;
    std::string exePath = UTF8ToGB2312(installpt.u8string());
    KillProcessByPath(exePath);

    DeleteRegistryKey(HKEY_CURRENT_USER,L"SOFTWARE\\"+ SOFT_TYPE);
    DeleteRegistryKey(HKEY_LOCAL_MACHINE,REG_UNINST_NODE);


    wchar_t desktopPath[MAX_PATH];
    wchar_t startMenuPath[MAX_PATH];
    // 获取桌面路径
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, desktopPath))) {
        std::wstring desktopShortcut = std::wstring(desktopPath) + L"\\" + SOFT_NAME+L".lnk";

        DeleteShortcut(desktopShortcut);
    }

    // 获取开始菜单路径
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_STARTMENU, NULL, 0, startMenuPath))) {
        std::wstring startMenuShortcut = std::wstring(startMenuPath) + L"\\" + SOFT_NAME+L".lnk";

        DeleteShortcut(startMenuShortcut);
    }

//    char* appDataPath = nullptr;
//    size_t len = 0;
//    if (_dupenv_s(&appDataPath, &len, "APPDATA") != 0 || appDataPath == nullptr) {
//        std::cerr << "Failed to get AppData path." << std::endl;
//        return;
//    }
//    SetCurrentDirectory(appDataPath);

    try {
        if (fs::exists(installPath)) {
            SafeRemoveAll(installPath);
        } else {
            std::wcout << "Directory does not exist: " << installPath << std::endl;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

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

#include "InstallWindow.h"
#include <iostream>

#include "archive_entry.h"
#include "archive.h"
#include <locale>

void attachToConsoleIfAvailable()
{
#ifdef _WIN32
    // 检查当前是否从命令行启动，并尝试附加控制台
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        FILE* outStream;
        freopen_s(&outStream, "CONOUT$", "w", stdout);  // 重定向 stdout
        freopen_s(&outStream, "CONOUT$", "w", stderr);  // 重定向 stderr
    }
#endif
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

#ifdef _WIN32
    attachToConsoleIfAvailable();  // 尝试附加到现有控制台
#endif
    std::cout<<"hell";
    // 设置全局区域为 UTF-8
    std::locale::global(std::locale("en_US.UTF-8"));
    try
    {
        InstallWindow window(880, 580, "Custom Window");
        window.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}

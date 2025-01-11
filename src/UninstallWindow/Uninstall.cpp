#include "UninstallWindow.h"
#include <iostream>

#include "archive_entry.h"
#include "archive.h"
#include <locale>
// 主函数
int main()
{

    // 设置全局区域为 UTF-8
    std::locale::global(std::locale("en_US.UTF-8"));
    try
    {
        UninstallWindow window(880, 580, "Custom Window");
        window.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}

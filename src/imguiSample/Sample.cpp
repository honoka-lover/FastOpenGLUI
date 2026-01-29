// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp


#include <stdio.h>
#include "FoglSample.h"
#define GL_SILENCE_DEPRECATION

#include <GLFW/glfw3.h> // Will drag system OpenGL headers

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif




#include <iostream>

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
    // 设置全局区域为 UTF-8
    //    std::locale::global(std::locale("en_US.UTF-8"));
    try
    {
        FoglSample window(880, 580, "Custom Window");
        window.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}




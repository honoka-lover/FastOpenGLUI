//
// Created by honoka on 2026/1/31.
//
#include <iostream>
#include <memory>
#include <glm/glm.hpp>
#include "FOGLApplication.h"
#include "FOGLRenderContext.h"
#include "FOGLSubWindow.h"
#include "FOGLButton.h"
#include "FOGLContainer.h"
#include "FOGLVBoxLayout.h"

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
    // 创建应用
    FOGLApplication app;


    // 创建根窗口
    auto root = std::make_shared<FOGLWidget>("Root Window");
    root->becomeTopLevelWindow();
    app.addTopLevelWidget(root);

    // 添加子 Widget
    auto child = std::make_shared<FOGLWidget>("Child Widget");
    root->addChild(child);

    // 子 Widget 创建新的顶层窗口
    child->createNewTopLevelWindow();

    // 创建子窗口
    auto subWin = std::make_shared<FOGLSubWindow>("Demo SubWindow");
    subWin->setGeometry(50, 50, 300, 200);

    // 创建容器 + 布局
    auto container = std::make_shared<FOGLContainer>();
    container->setGeometry(0, 0, 300, 200);

    auto layout = std::make_unique<FOGLVBoxLayout>();
    layout->spacing = 8.0f;
    container->setLayout(std::move(layout));

    // 创建按钮
    auto button = std::make_shared<FOGLButton>("Click Me");
    button->setGeometry(10, 10, 120, 40);
    button->setClickEvent([]() {
        std::cout << "Button clicked!\n";
    });

    // 挂到容器
    container->addChild(button);

    // 把容器挂到子窗口
    subWin->addChild(container);

    // 把子窗口挂到 UI 根节点
    root->addChild(subWin);

    // 运行应用
    app.run();

    return 0;
}

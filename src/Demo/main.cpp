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
#include "FOGLTextWidget.h"
#include "FOGLVBoxLayout.h"
#include "FOGLFunction.h"
#include "FOGLTextEditWidget.h"

void attachToConsoleIfAvailable()
{
#ifdef _WIN32
    //gdb模式不执行
    if (IsDebuggerPresent()) {
        return;
    }
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
    FOGLApplication::instance().init();

    // 创建根窗口
    auto root = std::make_shared<FOGLWidget>("Root Window");
    // root->becomeTopLevelWindow();

    // FOGLWidget widget("test");
    // widget.becomeTopLevelWindow();

    // // 添加子 Widget
    // auto child = std::make_shared<FOGLWidget>("Child Widget");
    // root->addChild(child);

    // 子 Widget 创建新的顶层窗口
    // child->becomeTopLevelWindow();

    // 创建子窗口
    auto subWin = std::make_shared<FOGLSubWindow>("Demo SubWindow");
    subWin->setGeometry(50, 50, 300, 200);

    // 创建容器 + 布局
    auto container = std::make_shared<FOGLContainer>();
    container->setGeometry(0, 0, 300, 200);

    auto layout = std::make_unique<FOGLVBoxLayout>();
    layout->setSpacing( 8.0f);
    container->setLayout(std::move(layout));

    // 创建按钮
    auto button = std::make_shared<FOGLButton>("Click Me");
    button->setGeometry(10, 10, 120, 40);
    button->setRadius(200);
    button->setClickEvent([]() {
        std::cout << "Button clicked!\n";
    });
    button->setHoverTexture("./wallhaven-p9gmlj.jpg");


    // textWidget->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));
    // auto button1 = std::make_shared<FOGLButton>("Click Me");
    // button1->becomeTopLevelWindow();
    // 挂到容器
    container->addChild(button);
    // container->becomeTopLevelWindow();

    // 把容器挂到子窗口
    subWin->setContainer(container);

    subWin->setGeometry(0,0,600,400);
    // 把子窗口挂到 UI 根节点
    root->addChild(subWin);

    // 3️⃣ 创建编辑器
    auto editor = std::make_shared<FOGLTextEditWidget>("My Editor");
    // editor->resize(300,200);
    editor->setGeometry(0,0,600,400);
    // 4️⃣ 设置为顶层窗口
    editor->becomeTopLevelWindow(800, 600);

    // 5️⃣ 配置编辑器
    editor->setFontSize(30.0f);        // 字体大小
    // editor->setWordWrap(true);         // 自动换行

    // 6️⃣ 添加一些文本
    editor->setText(u8"Hello, World!\nStart editing...");

    // 7️⃣ 或者添加富文本
    std::vector<TextFragment> fragments = {
        {"Welcome ", {1.0f, 1.0f, 1.0f, 1.0f}},      // 白色
        {"to ", {0.5f, 0.5f, 1.0f, 1.0f}},           // 蓝色
        {"FOGLTextEditWidget!", {1.0f, 0.5f, 0.0f, 1.0f}} // 橙色
    };
    // editor->setRichTextFragments(fragments);
    button->addChild(editor);

    glfwSwapInterval(0);   // 关闭垂直同步

    // 运行应用
    FOGLApplication::instance().exec();

    return 0;
}
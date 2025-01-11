#include "UninstallWindow.h"

#include <vector>
#include <chrono>
#include <thread>
#include <fstream>
#include <ShlObj_core.h>
#include "UninstallResources.h"
#include "filesystem"
#include "FastOpenGLUI/soft_info.h"
#include "UninstallResources.h"
#include "regeditFunction.h"
#include "shellapi.h"
namespace fs = std::filesystem;

// 顶点着色器源码
static const char *vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
)";

// 片段着色器源码
static const char *fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D backgroundTexture;

void main() {
    vec4 texColor = texture(backgroundTexture, TexCoord);
    if (texColor.a < 0.1) {
        discard; // 丢弃透明区域
    }
    FragColor = texColor;
}
)";

std::atomic<float> progress{0.0f}; // 提取进度 (0-100)

// 构造函数
UninstallWindow::UninstallWindow(int width, int height, const std::string &title)
    : FOGLWindow(width,height,title)
    , centerText(nullptr)
    , isThreadRunning(false) // 初始化线程状态为未运行
{


    centerText = new TextRenderer(this->windowWidth, this->windowHeight);
    if (fs::exists("C:/Windows/Fonts/msyh.ttc"))
        centerText->LoadFont("C:/Windows/Fonts/msyh.ttc");
    else if (fs::exists("C:/Windows/Fonts/msyh.ttf"))
        centerText->LoadFont("C:/Windows/Fonts/msyh.ttf");

    installPath = L"C:\\Program Files\\" + SOFT_TYPE;

    initButton();
}

// 析构函数
UninstallWindow::~UninstallWindow()
{
    if (extractionThread.joinable())
    {
        extractionThread.join();
    }


}

// 鼠标事件处理
void UninstallWindow::handleMouse()
{
    bool keepDragging = true;

    // 获取鼠标位置并检测 hover 和点击
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    minimizeButton->isHovered = false;
    closeButton->isHovered = false;

    if (insideFOGLRectangle(minimizeButton))
    {
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            minimizeButton->isHovered = false;
            glfwIconifyWindow(window);
            keepDragging = false;
        }
        else
        {
            minimizeButton->isHovered = true;
        }
    }
    else if (insideFOGLRectangle(closeButton))
    {
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            closeButton->isHovered = false;
            close();
            keepDragging = false;
        }
        else
        {
            closeButton->isHovered = true;
        }
    }else if(insideFOGLRectangle(uninstallButton)){
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            static int i=0;
            if(i == 0) {
                i++;
                DeleteInstallData();
            }
        }
        else
        {

        }
    }


    if(keepDragging)
        FOGLWindow::handleMouse();
}


// 渲染
void UninstallWindow::render()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // 设置清除颜色为透明
    glClear(GL_COLOR_BUFFER_BIT);

    backgroundRect->draw();

    // 绘制按钮
    minimizeButton->draw();
    closeButton->draw();


    // 正交投影矩阵，将窗口桌标投影到opengl坐标系中
    glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    uninstallButton->draw(projection);

    centerText->RenderText(L"卸载", uninstallButton->x + 50, uninstallButton->y + 20, 1.0f, 14, glm::vec3(1.0f, 1.0f, 1.0f));


}

void UninstallWindow::close()
{
    if (isThreadRunning)
    {
        if (extractionThread.joinable())
        {
            extractionThread.join();
        }
    }
    FOGLWindow::close();
//    deleteSelf();
}

void UninstallWindow::initButton()
{
    minimizeButton = createFOGLRectangle(800.0f, 18.0f, 24.0f, 24.0f, 0.0f, "#51CCFB");
    minimizeButton->setBackgroundSource(IDR_MINIMIZEPNG);
    minimizeButton->setHoverBackgroundSource(IDR_MINIMIZEHOVERPNG);
    closeButton = createFOGLRectangle(840.0f, 18.0f, 24.0f, 24.0f, 0.0f, "#51CCFB");
    closeButton->setBackgroundSource(IDR_CLOSEPNG);
    closeButton->setHoverBackgroundSource(IDR_CLOSEHOVERPNG);
    uninstallButton = createFOGLRectangle(300, 256.0f, 202.0f, 56.0f, 26.0f, "#51CCFB");

    backgroundRect = createFOGLRectangle(0.0f, 0.0f, (float)windowWidth, (float)windowHeight, 4.0f, "#000000");

    backgroundRect->setBackgroundSource(IDR_BACKGROUND);
}
void deleteSelf() {
    char szBatchFile[MAX_PATH] = { 0 };
    char szCurrentExe[MAX_PATH] = { 0 };

    // 获取当前程序路径
    GetModuleFileNameA(NULL, szCurrentExe, MAX_PATH);

    // 创建一个临时批处理文件
    sprintf(szBatchFile, "%s_del.bat", szCurrentExe);
    FILE* pBatch = fopen(szBatchFile, "w");

    if (pBatch) {
        fprintf(pBatch,
                ":loop\n"
                "del /q \"%s\"\n"      // 尝试删除当前程序文件
                "if exist \"%s\" goto loop\n"  // 如果文件仍然存在，循环尝试
                "del /q \"%s\"\n",     // 删除批处理文件本身
                szCurrentExe, szCurrentExe, szBatchFile);

        fclose(pBatch);

        // 运行批处理文件
        ShellExecuteA(NULL, "open", szBatchFile, NULL, NULL, SW_HIDE);
    }

    ExitProcess(0); // 退出程序
}
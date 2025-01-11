#include "InstallWindow.h"

#include <vector>
#include <chrono>
#include <thread>
#include <fstream>
#include "InstallResources.h"
#include "filesystem"
#include "FastOpenGLUI/soft_info.h"
#include "InstallResources.h"
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
InstallWindow::InstallWindow(int width, int height, const std::string &title)
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
InstallWindow::~InstallWindow()
{
    if (extractionThread.joinable())
    {
        extractionThread.join();
    }


}

// 鼠标事件处理
void InstallWindow::handleMouse()
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
    }
    else if (insideFOGLRectangle(folderButton))
    {
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            if (fs::exists(installPath))
            {
                auto result = selectFolderUsingIFileDialog(installPath);
                if (!result.empty())
                    installPath = result / SOFT_TYPE;
            }
            else
            {
                auto result = selectFolderUsingIFileDialog(L"C:\\Program Files");
                if (!result.empty())
                    installPath = result / SOFT_TYPE;
            }
            std::cout << installPath.string() << std::endl;
            keepDragging = false;
        }
        else
        {
        }
    }
    else if (insideFOGLRectangle(installButton))
    {
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            if (!isThreadRunning)
                extractRes(installPath);
            keepDragging = false;
        }
        else
        {
        }
    }

    if(keepDragging)
        FOGLWindow::handleMouse();
}


// 渲染
void InstallWindow::render()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // 设置清除颜色为透明
    glClear(GL_COLOR_BUFFER_BIT);

    backgroundRect->draw();

    // 绘制按钮
    minimizeButton->draw();
    closeButton->draw();

    centerText->RenderText(L"你好", 300.0f, 200.0f, 1.0f, 40, glm::vec3(0.0f, 0.0f, 0.0f));

    // 正交投影矩阵，将窗口桌标投影到opengl坐标系中
    glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    installButton->draw(projection);

    centerText->RenderText(L"快速安装", installButton->x + 50, installButton->y + 20, 1.0f, 14, glm::vec3(1.0f, 1.0f, 1.0f));

    folderButton->draw();
    centerText->RenderText(L"浏览", folderButton->x + 30, folderButton->y + 10, 1.0f, 14, glm::vec3(1.0f, 1.0f, 1.0f));

    installBack->draw();

    centerText->RenderText(installPath, 200, 500, 1.0f, 14, glm::vec3(0.0f, 0.0f, 0.0f));
}

void InstallWindow::extractRes(std::filesystem::path extractPath)
{
    extractionThread = std::thread([this, extractPath]()
                                   {
        isThreadRunning = true;
        try {
            // 使用普通函数
            auto MyCallback = [](float value) {
                std::cout << "Callback called with value: " << value << std::endl;
            };
            Extract7zResourceWithProgress(RES_DATA,extractPath, MyCallback);
        } catch (...) {
            // 捕获所有异常，避免未处理的崩溃
        }
        isThreadRunning = false; });
}

void InstallWindow::close()
{
    if (isThreadRunning)
    {
        if (extractionThread.joinable())
        {
            extractionThread.join();
        }
    }
    FOGLWindow::close();
}

void InstallWindow::initButton()
{
    minimizeButton = createFOGLRectangle(800.0f, 18.0f, 24.0f, 24.0f, 0.0f, "#51CCFB");
    minimizeButton->setBackgroundSource(IDR_MINIMIZEPNG);
    minimizeButton->setHoverBackgroundSource(IDR_MINIMIZEHOVERPNG);
    closeButton = createFOGLRectangle(840.0f, 18.0f, 24.0f, 24.0f, 0.0f, "#51CCFB");
    closeButton->setBackgroundSource(IDR_CLOSEPNG);
    closeButton->setHoverBackgroundSource(IDR_CLOSEHOVERPNG);
    installButton = createFOGLRectangle(300, 256.0f, 202.0f, 56.0f, 26.0f, "#51CCFB");

    folderButton = createFOGLRectangle(528, 320, 98, 40, 4, glm::vec4(0x56 / 255.0f, 0x57 / 255.0f, 0x5B / 255.0f, 0.9f));

    installBack = createFOGLRectangle(200, 500, 400, 200, 4, "#ffffff");

    backgroundRect = createFOGLRectangle(0.0f, 0.0f, (float)windowWidth, (float)windowHeight, 4.0f, "#000000");

    backgroundRect->setBackgroundSource(IDR_BACKGROUND);
}

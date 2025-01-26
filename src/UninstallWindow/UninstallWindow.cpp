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

// 渲染
void UninstallWindow::render()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // 设置清除颜色为透明
    glClear(GL_COLOR_BUFFER_BIT);

    backgroundRect->draw();

    // 绘制按钮
    minimizeButton->draw();
    closeButton->draw();

    centerText->RenderText(SOFT_NAME + L"卸载程序", 200.0f, 180.0f, 1.0f, 40, glm::vec3(0.0f, 0.0f, 0.0f));

    // 正交投影矩阵，将窗口桌标投影到opengl坐标系中
    glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    uninstallButton->draw(projection);
    if(uninstallButton->visible())
        centerText->RenderText(L"立即卸载", uninstallButton->x + 65, uninstallButton->y + 16, 1.0f, 20,glm::vec3(1.0f, 1.0f, 1.0f));

    progressBar->draw();
    if(progressBar->visible()){
        if(currentValue < 98.0f)
            currentValue.store(currentValue.load() + 0.7f);
        progressBar->setValue(currentValue.load()/100);
        centerText->RenderText(L"卸载中...", uninstallButton->x + 65, uninstallButton->y + 140, 1.0f, 18, glm::vec3(0.0f, 0.0f, 0.0f));
    }

    welComeButton->draw();
    if(welComeButton->visible())
        centerText->RenderText(L"卸载完成", uninstallButton->x + 65, uninstallButton->y + 16, 1.0f, 20,glm::vec3(1.0f, 1.0f, 1.0f));
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
    minimizeButton = createRectangle(800.0f, 18.0f, 24.0f, 24.0f, 0.0f, "#51CCFB");
    minimizeButton->setBackgroundSource(IDR_MINIMIZEPNG);
    minimizeButton->setHoverBackgroundSource(IDR_MINIMIZEHOVERPNG);
    minimizeButton->setEventClickFunc([this](){
        minimazal();
//            minimizeButton->resize(200,200);
//        minimizeButton->move(-20,0);
        return true;
    });

    closeButton = createRectangle(840.0f, 18.0f, 24.0f, 24.0f, 0.0f, "#51CCFB");
    closeButton->setBackgroundSource(IDR_CLOSEPNG);
    closeButton->setHoverBackgroundSource(IDR_CLOSEHOVERPNG);
    closeButton->setEventClickFunc([this](){
        close();
        if(welComeButton->visible())
            deleteSelf();
        return true;
    });

    uninstallButton = createRectangle(320, 280.0f, 240.0f, 80.0f, 40.0f, "#51CCFB");
    uninstallButton->setEventClickFunc([this](){
        uninstallButton->setVisible(false);
        progressBar->setVisible(true);
        DeleteInstallData();

        consoleProgress.start(4000,[this](){
            welComeButton->setVisible(true);
            progressBar->setVisible(false);
        });
        return true;
    });

    backgroundRect = createRectangle(0.0f, 0.0f, (float)windowWidth, (float)windowHeight, 0.0f, "#000000");

    backgroundRect->setBackgroundSource(IDR_BACKGROUND);

    welComeButton = createRectangle(320, 280.0f, 240.0f, 80.0f, 40.0f, "#51CCFB");

    welComeButton->setEventClickFunc([this](){
        close();
        deleteSelf();
        return true;
    });
    welComeButton->setVisible(false);

    progressBar = createProgressBar(100,480,700,16,8,"#ffffff");
    progressBar->setInnerProgressColor("#51CCFB");
    progressBar->setVisible(false);
}

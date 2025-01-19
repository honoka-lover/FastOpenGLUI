#include "InstallWindow.h"

#include <vector>
#include <chrono>
#include <thread>
#include <fstream>
#include "InstallResources.h"
#include "filesystem"
#include "soft_info.h"
#include "InstallResources.h"
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


// 渲染
void InstallWindow::render() {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // 设置清除颜色为透明
    glClear(GL_COLOR_BUFFER_BIT);

    backgroundRect->draw();

    // 绘制按钮
    minimizeButton->draw();
    closeButton->draw();

    centerText->RenderText(SOFT_NAME + L"安装程序", 200.0f, 180.0f, 1.0f, 40, glm::vec3(0.0f, 0.0f, 0.0f));

    // 正交投影矩阵，将窗口桌标投影到opengl坐标系中
    glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

    installButton->draw(projection);
    if (installButton->visible()) {
        centerText->RenderText(L"快速安装", installButton->x + 65, installButton->y + 16, 1.0f, 20,glm::vec3(1.0f, 1.0f, 1.0f));
    }

    installBack->draw();
    if (installBack->visible())
        centerText->RenderText(installPath, 60, 460, 1.0f, 16, glm::vec3(0.0f, 0.0f, 0.0f));

    folderButton->draw();

    if (folderButton->visible())
        centerText->RenderText(L"浏览", folderButton->x + 40, folderButton->y + 10, 1.0f, 16,glm::vec3(1.0f, 1.0f, 1.0f));


    if(progressBar->visible()){
        progressBar->setValue(currentValue.load()/100);
        centerText->RenderText(L"安装中...", installButton->x + 65, installButton->y + 140, 1.0f, 18, glm::vec3(0.0f, 0.0f, 0.0f));
    }

    progressBar->draw();

    welComeButton->draw();
    if(welComeButton->visible())
        centerText->RenderText(L"立即体验", installButton->x + 65, installButton->y + 16, 1.0f, 20,glm::vec3(1.0f, 1.0f, 1.0f));
}

void InstallWindow::extractRes(std::filesystem::path extractPath) {
    extractionThread = std::thread([this, extractPath]() {
        isThreadRunning = true;
        try {
            // 使用普通函数
            auto MyCallback = [this](float value) {
                std::cout << "Callback called with value: " << value << std::endl;
                currentValue.store(value);
                progressBar->setVisible(true);
                installButton->setVisible(false);
                folderButton->setVisible(false);
                installBack->setVisible(false);
            };
            Extract7zResourceWithProgress(RES_DATA, extractPath, MyCallback);
            WriteInstallData(installPath);
        } catch (...) {
            // 捕获所有异常，避免未处理的崩溃
        }
        progressBar->setVisible(false);
        isThreadRunning = false;
        welComeButton->setVisible(true);
    });
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
    minimizeButton = createRectangle(800.0f, 18.0f, 24.0f, 24.0f, 0.0f, "#51CCFB");
    minimizeButton->setBackgroundSource(IDR_MINIMIZEPNG);
    minimizeButton->setHoverBackgroundSource(IDR_MINIMIZEHOVERPNG);

    minimizeButton->setEventClickFunc([this](){
        minimazal();
        return true;
    });

    closeButton = createRectangle(840.0f, 18.0f, 24.0f, 24.0f, 0.0f, "#51CCFB");
    closeButton->setBackgroundSource(IDR_CLOSEPNG);
    closeButton->setHoverBackgroundSource(IDR_CLOSEHOVERPNG);

    closeButton->setEventClickFunc([this](){
        close();
        return true;
    });

    installButton = createRectangle(320, 280.0f, 240.0f, 80.0f, 40.0f, "#51CCFB");
    welComeButton = createRectangle(320, 280.0f, 240.0f, 80.0f, 40.0f, "#51CCFB");

    welComeButton->setEventClickFunc([this](){
        close();
        LaunchExe(installPath/SOFT_EXENAME);
        return true;
    });
    welComeButton->setVisible(false);

    installButton->setEventClickFunc([this](){
        if (!isThreadRunning)
            extractRes(installPath);
        return true;
    });

    folderButton = createRectangle(720, 450, 120, 60, 4, glm::vec4(0x56 / 255.0f, 0x57 / 255.0f, 0x5B / 255.0f, 0.9f));
    folderButton->setEventClickFunc([this](){
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
        return true;
    });

    installBack = createRectangle(40, 450, 750, 60, 4, "#ffffff");

    backgroundRect = createRectangle(0.0f, 0.0f, (float)windowWidth, (float)windowHeight, 4.0f, "#000000");

    backgroundRect->setBackgroundSource(IDR_BACKGROUND);

    progressBar = createProgressBar(100,480,700,16,8,"#ffffff");
    progressBar->setInnerProgressColor("#51CCFB");
    progressBar->setVisible(false);
}

#include "MainWindow.h"

#include <vector>
#include <chrono>
#include <thread>
#include <fstream>
#include "resources.h"
#include "filesystem"
#include "soft_info.h"
#include "resources.h"
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
MainWindow::MainWindow(int width, int height, const std::string &title)
    : windowWidth(width)
    , windowHeight(height)
    , isDragging(false)
    , centerText(nullptr)
    , isThreadRunning(false) // 初始化线程状态为未运行
{
    if (!glfwInit())
    {
        throw std::runtime_error("Failed to initialize GLFW");
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);              // 去除窗口装饰
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE); // 设置窗口透明
    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        throw std::runtime_error("Failed to initialize GLAD");
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shader.Compile(vertexShaderSource,fragmentShaderSource);
    setupQuad();


    centerText = new TextRenderer(this->windowWidth, this->windowHeight);
    if (fs::exists("C:/Windows/Fonts/msyh.ttc"))
        centerText->LoadFont("C:/Windows/Fonts/msyh.ttc");
    else if (fs::exists("C:/Windows/Fonts/msyh.ttf"))
        centerText->LoadFont("C:/Windows/Fonts/msyh.ttf");

    installPath = L"C:\\Program Files\\"+SOFT_TYPE;

    initButton();
}

// 析构函数
MainWindow::~MainWindow()
{
    if (extractionThread.joinable()) {
        extractionThread.join();
    }

    glDeleteBuffers(1, &backVbo);
    glDeleteVertexArrays(1, &backVao);
    glfwDestroyWindow(window);
    glfwTerminate();
}


// 设置绘制四边形的顶点数据
void MainWindow::setupQuad()
{
    float vertices[] = {
        // positions    // texture coords
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 0.0f, 1.0f};
    unsigned int indices[] = {0, 1, 2, 2, 3, 0};

    glGenVertexArrays(1, &backVao);
    glGenBuffers(1, &backVbo);

    glBindVertexArray(backVao);

    glBindBuffer(GL_ARRAY_BUFFER, backVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}


// 鼠标事件处理
void MainWindow::handleMouse()
{
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    if (handleButtons())
        return;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        if (!isDragging)
        {
            isDragging = true;
            lastMouseX = mouseX;
            lastMouseY = mouseY;
        }
        else
        {
            int dx = static_cast<int>(mouseX - lastMouseX);
            int dy = static_cast<int>(mouseY - lastMouseY);
            int x, y;
            glfwGetWindowPos(window, &x, &y);
            glfwSetWindowPos(window, x + dx, y + dy);
            // std::cout << "dx:" << dx << " dy:" << dy << std::endl;
            // std::cout << "x:" << x << " y:" << y << std::endl;
            // 更新鼠标位置
            // lastMouseX = mouseX;
            // lastMouseY = mouseY;
        }
    }
    else
    {
        isDragging = false;
    }
}

// 按钮事件处理
bool MainWindow::handleButtons()
{
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
            return true;
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
            return true;
        }
        else
        {
            closeButton->isHovered = true;
        }
    }else if(insideFOGLRectangle(folderButton)){
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            if(fs::exists(installPath)){
                auto result = selectFolderUsingIFileDialog(installPath);
                if(!result.empty())
                    installPath = result / SOFT_TYPE;
            }else{
                auto result = selectFolderUsingIFileDialog(L"C:\\Program Files");
                if(!result.empty())
                    installPath = result / SOFT_TYPE;
            }
            std::cout<<installPath.string()<<std::endl;
            return true;
        }
        else
        {

        }
    }else if(insideFOGLRectangle(installButton)){
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            if(!isThreadRunning)
                extractRes(installPath);
            return true;
        }
        else
        {

        }
    }

    return false;
}



// 渲染
void MainWindow::render()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // 设置清除颜色为透明
    glClear(GL_COLOR_BUFFER_BIT);

    shader.Use();
    glBindVertexArray(backVao);

    // 绘制背景
//    glBindTexture(GL_TEXTURE_2D, backgroundTexture);
//    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    backgroundRect->draw();

    // 绘制按钮
    minimizeButton->draw();
    closeButton->draw();

    centerText->RenderText(L"你好", 300.0f, 200.0f, 1.0f, 40, glm::vec3(0.0f, 0.0f, 0.0f));

    // 正交投影矩阵，将窗口桌标投影到opengl坐标系中
    glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    installButton->draw(projection);

    centerText->RenderText(L"快速安装", installButton->x+50, installButton->y+20, 1.0f, 14, glm::vec3(1.0f, 1.0f, 1.0f));

    folderButton->draw();
    centerText->RenderText(L"浏览", folderButton->x+30, folderButton->y+10, 1.0f, 14, glm::vec3(1.0f, 1.0f, 1.0f));

    installBack->draw();

    centerText->RenderText(installPath, 200, 500, 1.0f, 14, glm::vec3(0.0f, 0.0f, 0.0f));
}

// 主循环
void MainWindow::run()
{
    const double targetFPS = 60.0;                // 目标帧率
    const double frameDuration = 1.0 / targetFPS; // 每帧持续时间（秒）

    while (!glfwWindowShouldClose(window))
    {
        // 记录帧开始时间
        auto frameStart = std::chrono::high_resolution_clock::now();

        // 执行更新和渲染
        handleMouse();

        render();

        // 显示缓冲区
        glfwSwapBuffers(window);
        glfwPollEvents();

        // 记录帧结束时间并计算渲染时间
        auto frameEnd = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = frameEnd - frameStart;

        // 如果渲染时间小于目标帧时间，进行延迟
        if (elapsed.count() < frameDuration)
        {
            std::this_thread::sleep_for(
                std::chrono::duration<double>(frameDuration - elapsed.count()));
        }
    }
}



void MainWindow::extractRes(std::filesystem::path extractPath)
{
    extractionThread = std::thread([this, extractPath]() {
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
        isThreadRunning = false;
    });
}

void MainWindow::close() {
    if (isThreadRunning) {
        if (extractionThread.joinable()) {
            extractionThread.join();
        }
    }
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

bool MainWindow::insideFOGLRectangle(const FOGLRectangle *button) {
    // 获取鼠标位置并检测 hover 和点击
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);


    if (mouseX >= button->x && mouseX <= button->x + button->width && mouseY >= button->y && mouseY <= button->y + button->height)
    {
        return true;
    }

    return false;
}

void MainWindow::initButton() {
    minimizeButton = createFOGLRectangle(800.0f, 18.0f, 24.0f, 24.0f,0.0f, "#51CCFB");
    minimizeButton->setBackgroundSource(IDR_MINIMIZEPNG);
    minimizeButton->setHoverBackgroundSource(IDR_MINIMIZEHOVERPNG);
    closeButton = createFOGLRectangle(840.0f, 18.0f, 24.0f, 24.0f,0.0f, "#51CCFB");
    closeButton->setBackgroundSource(IDR_CLOSEPNG);
    closeButton->setHoverBackgroundSource(IDR_CLOSEHOVERPNG);
    installButton = createFOGLRectangle(300, 256.0f, 202.0f, 56.0f, 26.0f, "#51CCFB");

    folderButton = createFOGLRectangle(528,320,98,40,4,glm::vec4(0x56/255.0f,0x57/255.0f,0x5B/255.0f,0.9f));

    installBack = createFOGLRectangle(200, 500,400,200,4,"#ffffff");

    backgroundRect = createFOGLRectangle(0.0f,0.0f,(float)windowWidth,(float)windowHeight, 4.0f,"#000000");

    backgroundRect->setBackgroundSource(IDR_BACKGROUND);
}



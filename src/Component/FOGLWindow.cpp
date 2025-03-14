﻿//
// Created by m1393 on 2025/1/11.
//
#include "FOGLWindow.h"
#include "algorithm"
FOGLWindow::FOGLWindow(int width, int height, const std::string &title):
        windowWidth(width)
        , windowHeight(height)
        , isDragging(false)
        , lastMouseX(0.0f)
        , lastMouseY(0.0f)
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

    // 获取主显示器和视频模式
    GLFWmonitor *primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *videoMode = glfwGetVideoMode(primaryMonitor);
    if (!primaryMonitor || !videoMode)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to get monitor or video mode");
    }

    // 计算居中位置
    int screenWidth = videoMode->width;
    int screenHeight = videoMode->height;
    int posX = (screenWidth - width) / 2;
    int posY = (screenHeight - height) / 2;

    // 设置窗口位置
    glfwSetWindowPos(window, posX, posY);


    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        throw std::runtime_error("Failed to initialize GLAD");
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

FOGLWindow::~FOGLWindow(){
    p_FOGLRectangleViewList.clear();

    glfwDestroyWindow(window);
    glfwTerminate();
}

void FOGLWindow::run() {
    const double targetFPS = m_frameRate;                // 目标帧率
    const double frameDuration = 1.0 / targetFPS; // 每帧持续时间（秒）

    while (!glfwWindowShouldClose(window) && window)
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

void FOGLWindow::close() {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
    glfwDestroyWindow(window);
    window = nullptr;
    glfwTerminate();
}

void FOGLWindow::setFrameRate(double time) {
    m_frameRate = time;
}

void FOGLWindow::render() {

}

void FOGLWindow::handleMouse() {
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    if(notify())
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

bool FOGLWindow::insideFOGLRectangle(const FOGLRectangle *button)
{
    // 获取鼠标位置并检测 hover 和点击
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    if (mouseX >= button->x && mouseX <= button->x + button->width && mouseY >= button->y && mouseY <= button->y + button->height)
    {
        return true;
    }

    return false;
}

FOGLRectangle *
FOGLWindow::createRectangle(float x, float y, float width, float height, float radius, glm::vec4 color){
    auto *view = FOGLRectangle::createFOGLRectangle(x, y, width, height, radius, color, this);
    addView(view);
    return view;
}

FOGLRectangle *
FOGLWindow::createRectangle(float x, float y, float width, float height, float radius, std::string color) {
    auto *view =  FOGLRectangle::createFOGLRectangle(x, y, width, height, radius, colorStringToVec4(color), this);
    addView(view);
    return view;
}

void FOGLWindow::addView(FOGLRectangle *view) {
    std::shared_ptr<FOGLRectangle> temp(view);
    auto iter = find(p_FOGLRectangleViewList.begin(), p_FOGLRectangleViewList.end(), temp);
    if (iter == p_FOGLRectangleViewList.end()) {
        p_FOGLRectangleViewList.push_front(temp);
    } else {
        std::cout << "View already exists" << std::endl;
    }
}

bool FOGLWindow::notify() {
    auto iter = p_FOGLRectangleViewList.begin();
    for(; iter != p_FOGLRectangleViewList.end(); iter++)
    {
        if((*iter).get()->processMouseEvent())
            return true;
    }
    return false;
}

void FOGLWindow::minimazal() {
    glfwIconifyWindow(window);
}

FOGLProgressBar *
FOGLWindow::createProgressBar(float x, float y, float width, float height, float radius, glm::vec4 color) {
    auto *view = FOGLProgressBar::createFOGLProgressBar(x, y, width, height, radius, color, this);
    addView(view);
    return view;
}

FOGLProgressBar *
FOGLWindow::createProgressBar(float x, float y, float width, float height, float radius, std::string color) {
    auto *view =  FOGLProgressBar::createFOGLProgressBar(x, y, width, height, radius, colorStringToVec4(color), this);
    addView(view);
    return view;
}

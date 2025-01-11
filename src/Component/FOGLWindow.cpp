//
// Created by m1393 on 2025/1/11.
//
#include "FOGLWindow.h"

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
    glfwDestroyWindow(window);
    glfwTerminate();
}

void FOGLWindow::run() {
    const double targetFPS = m_frameRate;                // 目标帧率
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

void FOGLWindow::close() {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void FOGLWindow::setFrameRate(double time) {
    m_frameRate = time;
}

void FOGLWindow::render() {

}

void FOGLWindow::handleMouse() {
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

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
FOGLWindow::createFOGLRectangle(float x, float y, float width, float height, float radius, glm::vec4 color){
    return new FOGLRectangle(x, y, width, height, radius, color, windowWidth, windowHeight);
}

FOGLRectangle *
FOGLWindow::createFOGLRectangle(float x, float y, float width, float height, float radius, std::string color) {
    return new FOGLRectangle(x, y, width, height, radius, colorStringToVec4(color), windowWidth, windowHeight);
}

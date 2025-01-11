//
// Created by m1393 on 2025/1/11.
//

#ifndef FASTOPENGLUI_FOGLWINDOW_H
#define FASTOPENGLUI_FOGLWINDOW_H


#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "string"
#include "iostream"
#include "thread"
#include "FOGLRectangle.h"

class FOGLWindow{
public:
    FOGLWindow(int width, int height, const std::string &title = "");

    virtual ~FOGLWindow();

    virtual void run();

    virtual void close();

    [[nodiscard]] int getWidth() const { return windowWidth; }
    [[nodiscard]] int getHeight() const { return windowHeight; }

    void setFrameRate(double time);
protected:
    GLFWwindow *window;
    int windowWidth, windowHeight;
    bool isDragging{};               // 记录是否点击
    double lastMouseX{}, lastMouseY{}; // 记录移动

    //帧率，每秒刷新多少次页面
    double m_frameRate = 60;

    //设置要渲染的图像
    virtual void render() = 0;

    //处理鼠标事件 (包含全局窗口拖动功能)
    virtual void handleMouse();

    FOGLRectangle *createFOGLRectangle(float x, float y, float width, float height, float radius = 0.0f, glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

    FOGLRectangle *createFOGLRectangle(float x, float y, float width, float height, float radius, std::string color = "000000FF");

    bool insideFOGLRectangle(const FOGLRectangle *button);
};


#endif //FASTOPENGLUI_FOGLWINDOW_H

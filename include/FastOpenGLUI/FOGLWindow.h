//
// Created by m1393 on 2025/1/11.
//

#ifndef FASTOPENGLUI_FOGLWINDOW_H
#define FASTOPENGLUI_FOGLWINDOW_H


#include <memory>
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

    virtual void minimizal();

    [[nodiscard]] float getWidth() const { return (float)windowWidth; }
    [[nodiscard]] float getHeight() const { return (float)windowHeight; }

    void setFrameRate(double time);

    GLFWwindow* getGLFWwindowPointer(){ return window;}

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

    FOGLRectangle *createRectangle(float x, float y, float width, float height, float radius = 0.0f, glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

    FOGLRectangle *createRectangle(float x, float y, float width, float height, float radius, std::string color = "000000FF");

    bool insideFOGLRectangle(const FOGLRectangle *button);

private:
    std::list<std::shared_ptr<FOGLRectangle>> p_FOGLRectangleViewList;

    void addView(FOGLRectangle* view);

    bool notify();
};


#endif //FASTOPENGLUI_FOGLWINDOW_H

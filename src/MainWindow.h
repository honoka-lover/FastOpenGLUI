#ifndef GLFWINSTALL_MAINWINDOW_H
#define GLFWINSTALL_MAINWINDOW_H

#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include "text_renderer.h"
#include "FOGLRectangle.h"
#include "thread"

#include "CommonFunc.h"

class MainWindow
{
public:
    MainWindow(int width, int height, const std::string &title);
    ~MainWindow();

    void run();

    void extractRes(std::filesystem::path extractPath);

    void close();

    float getWidth() const { return windowWidth; }
    float getHeight() const { return windowHeight; }
private:
    GLFWwindow *window;
    Shader shader;
    int windowWidth, windowHeight;
    GLuint backVao, backVbo;
    bool isDragging;//记录是否点击
    double lastMouseX, lastMouseY;//记录移动
    TextRenderer *centerText;

    FOGLRectangle *minimizeButton, *closeButton;
    FOGLRectangle *installButton, *folderButton,*installBack, *backgroundRect;

    std::thread extractionThread;    // 提取线程
    std::atomic<bool> isThreadRunning; // 标记线程是否正在运行

    fs::path installPath;

    void render();
    void handleMouse();
    bool handleButtons();

    void setupQuad();
    FOGLRectangle *createFOGLRectangle(float x, float y, float width, float height,float radius = 0.0f,glm::vec4 color = glm::vec4(0.0f,0.0f,0.0f,1.0f)) {
        return new FOGLRectangle(x, y, width, height,radius,color, this);
    }

    FOGLRectangle *createFOGLRectangle(float x, float y, float width, float height,float radius,std::string color = "000000FF") {
        return new FOGLRectangle(x, y, width, height,radius,colorStringToVec4(color), this);
    }

    bool insideFOGLRectangle(const FOGLRectangle *button);

    void initButton();

};


#endif
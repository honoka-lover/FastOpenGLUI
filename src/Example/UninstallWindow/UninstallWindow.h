#ifndef GLFWINSTALL_InstallWindow_H
#define GLFWINSTALL_InstallWindow_H

#include <iostream>
#include <string>
#include "text_renderer.h"
#include "FOGLRectangle.h"
#include "thread"
#include "FOGLWindow.h"
#include "CommonFunc.h"
#include "FOGLProgressBar.h"
#include "Timer.h"

class UninstallWindow:public FOGLWindow
{
public:
    UninstallWindow(int width, int height, const std::string &title);
    ~UninstallWindow() override;

    void close() override;
private:
    TextRenderer *centerText;

    FOGLRectangle *minimizeButton, *closeButton;
    FOGLRectangle *uninstallButton, *backgroundRect;
    FOGLProgressBar *progressBar;
    FOGLRectangle *welComeButton;

    std::thread extractionThread;      // 提取线程
    std::atomic<bool> isThreadRunning; // 标记线程是否正在运行

    fs::path installPath;

    std::atomic<float> currentValue = 0.0f;

    Timer consoleProgress;

    void render() override;

    void initButton();
};


#endif
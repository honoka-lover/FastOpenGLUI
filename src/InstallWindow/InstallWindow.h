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
class InstallWindow:public FOGLWindow
{
public:
    InstallWindow(int width, int height, const std::string &title);
    ~InstallWindow() override;

    void extractRes(std::filesystem::path extractPath);

    void close() override;

private:
    TextRenderer *centerText;

    FOGLRectangle *minimizeButton, *closeButton;
    FOGLRectangle *installButton, *folderButton, *installBack, *backgroundRect;

    FOGLRectangle *welComeButton;

    FOGLProgressBar *progressBar;

    std::thread extractionThread;      // 提取线程
    std::atomic<bool> isThreadRunning; // 标记线程是否正在运行

    fs::path installPath;

    std::atomic<float> currentValue = 0.0f;

    void render() override;

    void initButton();
};

#endif
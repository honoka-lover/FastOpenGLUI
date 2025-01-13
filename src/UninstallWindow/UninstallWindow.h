#ifndef GLFWINSTALL_InstallWindow_H
#define GLFWINSTALL_InstallWindow_H

#include <iostream>
#include <string>
#include "FastOpenGLUI/text_renderer.h"
#include "FastOpenGLUI/FOGLRectangle.h"
#include "thread"
#include "FastOpenGLUI/FOGLWindow.h"
#include "FastOpenGLUI/CommonFunc.h"
#include "FastOpenGLUI/FOGLProgressBar.h"
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


    std::thread extractionThread;      // 提取线程
    std::atomic<bool> isThreadRunning; // 标记线程是否正在运行

    fs::path installPath;

    void render() override;

    void initButton();
};

void deleteSelf();
#endif
//
// Created by honoka on 2026/1/31.
//

#include "FOGLApplication.h"

#include <iostream>

FOGLApplication::FOGLApplication() {
    if (!glfwInit())
    {
        throw std::runtime_error("Failed to initialize GLFW");
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

}

FOGLApplication::~FOGLApplication() {
    glfwTerminate();
}

// 全局 map（或者 Application 内部管理）
std::unordered_map<GLFWwindow*,FOGLWidget*> g_windowMap;

// 回调
void mouseButtonCallback(GLFWwindow* handle, int button, int action, int mods) {
    auto it = g_windowMap.find(handle);
    if (it == g_windowMap.end()) return;

    auto w = it->second;

    MouseEvent e{};
    double x,y;
    glfwGetCursorPos(handle,&x,&y);
    e.x = x; e.y = y;
    e.button = static_cast<MouseButton>(button);
    // e.action = static_cast<MouseAction>(action);
    e.mods = mods;
    e.button = GLFW_MOUSE_BUTTON_LEFT == button ? MouseButton::Left : MouseButton::Right;
    e.action = action == GLFW_PRESS ? MouseAction::Press : MouseAction::Release;
    w->onMouseEvent(e);
}

void mouseButtonMoveCallback(GLFWwindow* handle, double x,double y) {
    auto it = g_windowMap.find(handle);
    if (it == g_windowMap.end()) return;

    auto w = it->second;

    MouseEvent e{};
    e.x = x; e.y = y;
    e.action = MouseAction::Move;
    w->onMouseEvent(e);
}

void mouseButtonScrollCallback(GLFWwindow* handle, double x,double y) {
    auto it = g_windowMap.find(handle);
    if (it == g_windowMap.end()) return;

    auto w = it->second;

    MouseEvent e{};
    e.x = x; e.y = y;
    e.action = MouseAction::Scroll;
    w->onMouseEvent(e);
}


void FOGLApplication::addTopLevelWidget(std::shared_ptr<FOGLWidget> w) {
    // 自动创建 GLFWwindow 如果还没创建
    w->setTopLevelRegisterFunc([this](std::shared_ptr<FOGLWidget> w2){
        addTopLevelWidget(w2); // 递归注册
    });
    m_roots.push_back(w);
    g_windowMap[w->getGLFWwindowPointer()]=w.get();
    glfwSetMouseButtonCallback(w->getGLFWwindowPointer(), mouseButtonCallback);
    glfwSetCursorPosCallback(w->getGLFWwindowPointer(), mouseButtonMoveCallback);
    glfwSetScrollCallback(w->getGLFWwindowPointer(), mouseButtonMoveCallback);
}

void FOGLApplication::run() {
   while (!m_roots.empty()) {
        // 遍历所有窗口
       // 渲染所有顶层窗口
       for (auto it = m_roots.begin(); it != m_roots.end();) {
           auto& w = *it;
           GLFWwindow* win = w->getGLFWwindowPointer();
           if (glfwWindowShouldClose(win)) {
               // 销毁窗口并移出列表
               w->close();
               it = m_roots.erase(it);
           } else {
               // 切换上下文渲染
               glfwMakeContextCurrent(win);
               w->render();
               glfwSwapBuffers(win);
               ++it;
           }
       }

       // for (auto it = m_roots.begin(); it != m_roots.end(); ) {
       //     auto& w = *it;
       //     GLFWwindow* win = w->getGLFWwindowPointer();
       //
       //     if (glfwWindowShouldClose(win)) {
       //         // 销毁窗口并移出列表
       //         glfwDestroyWindow(win);
       //         it = m_windows.erase(it);
       //     } else {
       //         // 切换上下文渲染
       //         glfwMakeContextCurrent(win);
       //         w->render();
       //         glfwSwapBuffers(win);
       //         ++it;
       //     }
       // }



        glfwPollEvents(); // 全局事件轮询
    }

    // 所有窗口关闭后退出
    glfwTerminate();
}

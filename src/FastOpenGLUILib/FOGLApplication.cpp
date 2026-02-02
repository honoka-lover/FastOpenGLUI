//
// Created by honoka on 2026/1/31.
//

#include "FOGLApplication.h"

#include <iostream>

FOGLApplication& FOGLApplication::instance() {
    static FOGLApplication app;
    return app;
}

void FOGLApplication::init() {
}

void FOGLApplication::registerRoot(FOGLWidget * root) {
    if (!root) return;
    if (std::find(m_pendingAdd.begin(), m_pendingAdd.end(), root) == m_pendingAdd.end())
        m_pendingAdd.push_back(root);
}

void FOGLApplication::unregisterRoot(FOGLWidget *root) {
    if (!root)
        return;
    if (std::find(m_pendingRemove.begin(), m_pendingRemove.end(), root) == m_pendingRemove.end())
        m_pendingRemove.push_back(root);
}

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
    w->onMoveEvent(e);
}

void mouseButtonScrollCallback(GLFWwindow* handle, double x,double y) {
    auto it = g_windowMap.find(handle);
    if (it == g_windowMap.end()) return;

    auto w = it->second;

    MouseEvent e{};
    e.x = x; e.y = y;
    e.action = MouseAction::Scroll;
    w->onScrollEvent(e);
}

void keyCallback(GLFWwindow* window,
                 int key,
                 int scancode,
                 int action,
                 int mods)
{
    auto it = g_windowMap.find(window);
    if (it == g_windowMap.end()) return;
    auto w = it->second;

    KeyEvent e{};
    e.key = key;
    e.scancode = scancode;
    e.mods = static_cast<KeyMod>(mods);

    switch (action) {
        case GLFW_PRESS:   e.action = KeyAction::Press;   break;
        case GLFW_RELEASE: e.action = KeyAction::Release; break;
        case GLFW_REPEAT:  e.action = KeyAction::Repeat;  break;
        default: ;
    }

    w->onKeyEvent(e);
}

void charCallback(GLFWwindow* window, unsigned int codepoint)
{
    auto it = g_windowMap.find(window);
    if (it == g_windowMap.end()) return;
    auto w = it->second;

    TextEvent e{};
    e.codepoint = codepoint; // Unicode
    w->onTextInput(e);
}


void registerEvents(FOGLWidget *w) {
    g_windowMap[w->getGLFWwindowPointer()]=w;
    glfwSetMouseButtonCallback(w->getGLFWwindowPointer(), mouseButtonCallback);
    glfwSetCursorPosCallback(w->getGLFWwindowPointer(), mouseButtonMoveCallback);
    glfwSetScrollCallback(w->getGLFWwindowPointer(), mouseButtonScrollCallback);
    glfwSetKeyCallback(w->getGLFWwindowPointer(), keyCallback);
    glfwSetCharCallback(w->getGLFWwindowPointer(), charCallback);

}

void FOGLApplication::updateRoot() {
    // ---------- remove ----------
    if (!m_pendingRemove.empty()) {
        std::vector<FOGLWidget*> pendingRemove;
        pendingRemove.swap(m_pendingRemove);

        auto it = std::remove_if(
            m_roots.begin(),
            m_roots.end(),
            [&pendingRemove](FOGLWidget* w) {
                return std::find(pendingRemove.begin(), pendingRemove.end(), w) != pendingRemove.end();
            }
        );
        m_roots.erase(it, m_roots.end());
    }

    // ---------- add ----------
    if (!m_pendingAdd.empty()) {
        std::vector<FOGLWidget*> pendingAdd;
        pendingAdd.swap(m_pendingAdd);

        for (auto* w : pendingAdd) {
            if (!w) continue;

            if (std::find(m_roots.begin(), m_roots.end(), w) != m_roots.end())
                continue;
            registerEvents(w);
            m_roots.push_back(w); // 仅存指针，不增加引用
        }
    }
}



void FOGLApplication::run() {
   do {
       updateRoot();
       // 遍历所有窗口
       // 渲染所有顶层窗口
       for (auto it = m_roots.begin(); it != m_roots.end();) {
           auto w = *it;
           GLFWwindow* win = w->getGLFWwindowPointer();
           if (!win) {
               it = m_roots.erase(it);
               continue;
           }
           if (glfwWindowShouldClose(win)) {
               // 销毁窗口并移出列表
               it = m_roots.erase(it);
               w->close();
           } else {
               // 切换上下文渲染
               glfwMakeContextCurrent(win);
               w->render();
               glfwSwapBuffers(win);
               ++it;
           }
       }

       glfwPollEvents(); // 全局事件轮询
   }while (!m_roots.empty() || !m_pendingAdd.empty());


    // 所有窗口关闭后退出
    glfwTerminate();
}

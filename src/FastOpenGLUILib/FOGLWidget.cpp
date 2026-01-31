//
// Created by honoka on 2026/1/31.
//
#include "FOGLWidget.h"
#include <iostream>
#include <memory>

#include "FOGLRenderContext.h"

static FOGLResourceManager resMgr;
FOGLRenderContext FOGLWidget::m_ctx(resMgr);

FOGLWidget::FOGLWidget(const std::string& name): m_windowName(name) {


}

FOGLWidget::~FOGLWidget() {

}

void FOGLWidget::resignTopLevelWindow()  {
    if (m_windowRole == FOGLWindowRole::TopLevel)
        return;

    // 1️⃣ 关闭 GLFWwindow
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }

    // 2️⃣ 修改状态
    m_windowRole = FOGLWindowRole::None;

    // 3️⃣ 子 Widget 不做处理，它们仍然保留 parent 指针
    // 如果需要，把 m_registerTopLevel 置空
    m_registerTopLevel = nullptr;
}

void FOGLWidget::close() {
    if (m_windowRole == FOGLWindowRole::TopLevel) {
        glfwSetWindowShouldClose(m_window, GLFW_TRUE);
        if (m_window)
            glfwDestroyWindow(m_window);
        m_window = nullptr;
    }else {

    }
    m_eventListen = false;
}

void FOGLWidget::becomeTopLevelWindow(int width, int height,bool transparentFrameBuffer,bool decorated) {
    if (m_window)
        return;
    m_rect.width = width;
    m_rect.height = height;
    if (transparentFrameBuffer)
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE); // 设置窗口透明

    if (decorated)
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); // 去除窗口装饰
    // glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);              // 去除窗口装饰
    // glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE); // 设置窗口透明
    m_window = glfwCreateWindow(width, height, m_windowName.c_str(), nullptr, nullptr);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    if (!m_window)
        throw std::runtime_error("Failed to create GLFW window");

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
    glfwSetWindowPos(m_window, posX, posY);

    glfwMakeContextCurrent(m_window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        throw std::runtime_error("Failed to initialize GLAD");
    }

    // ✅ 现在 OpenGL 上下文有效，可以初始化 ctx
    m_ctx.init(width,height);  // 这里安全调用 glEnable/glBlendFunc 等

    m_windowRole = FOGLWindowRole::TopLevel;
}

void FOGLWidget::setGeometry(float x, float y, float w, float h) {


     m_rect = {x, y, w, h};
    updateOffset();
}

void FOGLWidget::updateOffset() {
    if (m_windowRole != FOGLWindowRole::TopLevel && m_parent) {
        m_xOffset = m_parent->m_rect.x + m_parent->m_xOffset;
        m_yOffset = m_parent->m_rect.y + m_parent->m_yOffset;
    }
    for (auto &child:m_children) {
        child->updateOffset();
    }
}

void FOGLWidget::addChild(std::shared_ptr<FOGLWidget> child) {
    child->m_parent = this;
    // 继承注册回调
    child->m_registerTopLevel = m_registerTopLevel;

    m_children.push_back(child);
    updateOffset();
}

void FOGLWidget::setTopLevelRegisterFunc(WindowRegisterFunc func) {
    m_registerTopLevel = std::move(func);
}

void FOGLWidget::createNewTopLevelWindow(std::string title)  {
    this->m_parent = nullptr;
    if (title.empty())
        title = m_windowName;
    auto w = std::make_shared<FOGLWidget>(title);
    w->becomeTopLevelWindow();

    if (m_registerTopLevel) {
        m_registerTopLevel(w); // 通知 Application
    }
}

void FOGLWidget::attachChildren(std::vector<std::shared_ptr<FOGLWidget>> children,bool asTopLevel) {
    for (auto& c : children) {
        c->m_parent = this;

        if (asTopLevel) {
            c->becomeTopLevelWindow();
        } else {
            c->resignTopLevelWindow();
        }
    }
    m_children = std::move(children);
}

std::vector<std::shared_ptr<FOGLWidget>> FOGLWidget::detachChildren() {
    for (auto& c : m_children)
        c->m_parent = nullptr;

    return std::move(m_children);
}

bool FOGLWidget::contains(float px, float py) const {
    return px >= m_rect.x && px <= m_rect.x + m_rect.width &&
           py >= m_rect.y && py <= m_rect.y + m_rect.height;
}

void FOGLWidget::render() {
    if (!m_eventListen)
        return;
    auto it = m_children.begin();
    for (int i=0; i<m_children.size(); i++) {
        auto & w = *it;
        if (!(*it)->m_parent) {
            m_children.erase(it);
        }else
            ++it;
    }

    // 先处理待提升的 Widget
    for (auto& w : m_pendingBringToFront) {
        auto it = std::find(m_children.begin(), m_children.end(), w);
        if (it != m_children.end()) {
            m_children.erase(it);
            m_children.push_back(w); // Z-order 最前
        }
    }
    m_pendingBringToFront.clear();

    // double mouseX, mouseY;
    // glfwGetCursorPos(m_window, &mouseX, &mouseY);
    // int leftAction = glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT);
    onPaint(m_ctx);
}

bool FOGLWidget::processMouseEvent(double mouseX, double mouseY, int button, int action) {

    return false;
}

void FOGLWidget::onPaint(FOGLRenderContext &ctx) {
    // 先统一提升 Z-order
    for (auto& w : m_pendingBringToFront) {
        auto it = std::find(m_children.begin(), m_children.end(), w);
        if (it != m_children.end()) {
            m_children.erase(it);
            m_children.push_back(w); // Z-order 最前
        }
    }
    m_pendingBringToFront.clear();

    for (auto child : m_children) {
        if (child->isVisible()) {
            child->onPaint(m_ctx);
        }
    }
}

bool FOGLWidget::handleSelfMouseEvent(const MouseEvent &e) {
    if (e.action == MouseAction::Press)
        std::cout << "handleSelfMouseEvent"  << " " << e.x << " " << e.y << std::endl;

    return true;
}

bool FOGLWidget::onMouseEvent(const MouseEvent &e) {
    bool handleMouseEventFlag = true;
    if (!this->m_visible || !m_eventListen) {
        return handleMouseEventFlag;
    }
    std::shared_ptr<FOGLWidget> childToBringFront = nullptr;

    if (!contains(e.x, e.y))
        return handleMouseEventFlag;

    if (m_children.empty()) {
        return handleSelfMouseEvent(e);
    }

    // 倒序遍历子 Widget（Z-order 最前先处理）
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        auto child = *it;
        if (!child->m_visible) continue;

        if (child->contains(e.x, e.y)) {
            MouseEvent me = e;
            me.x -= m_rect.x;
            me.y -= m_rect.y;
            if (child->onMouseEvent(me)) {  // 递归调用
                handleMouseEventFlag = handleSelfMouseEvent(me);
                if (!handleMouseEventFlag) {
                    break;
                }
            }
            childToBringFront = child;
            break;
        }
    }

    if (childToBringFront) {
        setFocusChild(childToBringFront);
    }

    return handleMouseEventFlag;
}

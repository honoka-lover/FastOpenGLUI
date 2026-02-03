//
// Created by honoka on 2026/1/31.
//
#include "FOGLWidget.h"
#include <iostream>
#include <memory>

#include "FOGLApplication.h"
#include "FOGLContainer.h"
#include "FOGLRenderContext.h"

static FOGLResourceManager resMgr;

FOGLWidget::FOGLWidget(const std::string& name): m_windowName(name)
{
    setGeometry(0,0,0,0);
}

FOGLWidget::~FOGLWidget() {
    FOGLApplication::instance().unregisterRoot(this);
    if (m_ctx) {
        delete m_ctx;
        m_ctx = nullptr;
    }
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

}

void FOGLWidget::close() {
    if (m_windowRole == FOGLWindowRole::TopLevel) {
        FOGLApplication::instance().unregisterRoot(this);
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

    if (m_ctx)
        delete m_ctx;
    m_ctx = new FOGLRenderContext(resMgr);
    // ✅ 现在 OpenGL 上下文有效，可以初始化 ctx
    m_ctx->init(width,height);  // 这里安全调用 glEnable/glBlendFunc 等

    m_parent = nullptr;
    m_windowRole = FOGLWindowRole::TopLevel;

    FOGLApplication::instance().registerRoot(this);
}

void FOGLWidget::setGeometry(float x, float y, float w, float h) {
     m_rect = {x, y, w, h};
    updateOffset();
    if (m_ctx &&m_windowRole != FOGLWindowRole::TopLevel)
        m_ctx->init(w,h);
}

void FOGLWidget::setGeometry(FOGLRect rect) {
    m_rect = rect;
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

    m_children.push_back(child);
    updateOffset();
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

void FOGLWidget::setContainer(const std::shared_ptr<FOGLContainer>& container) {
    if (container) {
        container->setGeometry(0,0,m_rect.width,m_rect.height);
        addChild(std::shared_ptr<FOGLWidget>(container));
    }
}

bool FOGLWidget::contains(float px, float py) const {
    return px >= m_rect.x && px <= m_rect.x + m_rect.width &&
           py >= m_rect.y && py <= m_rect.y + m_rect.height;
}

void FOGLWidget::hide() {
    m_visible = false;
}

void FOGLWidget::show() {
    m_visible = true;
}

void FOGLWidget::render() {
    if (!m_eventListen)
        return;
    auto it = m_children.begin();
    for (int i=0; i<m_children.size(); i++) {
        auto & w = *it;
        if ((*it)->m_windowRole == FOGLWindowRole::TopLevel ||!(*it)->m_parent) {
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
    onLayout();
    onPaint(*m_ctx);
}

void FOGLWidget::onLayout() {
     for (auto& c : m_children)
         c->onLayout();
}

void FOGLWidget::onPaint(FOGLRenderContext &ctx) {

    for (auto child : m_children) {
        if (child->visible()) {
            child->onPaint(ctx);
        }
    }
}

void FOGLWidget::handleSelfMouseEvent(const MouseEvent &e) {

}

void  FOGLWidget::handleScrollEvent(const MouseEvent &e) {

}

void FOGLWidget::onTextInput(const TextEvent &e) {
}

void FOGLWidget::onKeyEvent(const KeyEvent &e) {
}

bool FOGLWidget::onScrollEvent(const MouseEvent& e) {
    bool handleScrollEventFlag = true;
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        auto child = *it;
        if (!child->m_visible) continue;
        if (!child->onScrollEvent(e)) {
            handleScrollEventFlag = false;
        }
    }
    if (handleScrollEventFlag) {
        handleScrollEvent(e);
    }
    return handleScrollEventFlag;
}

bool FOGLWidget::onMoveEvent(const MouseEvent &e) {
    bool handleMoveEventFlag = true;
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        const auto& child = *it;
        if (!child->m_visible) continue;
        if (!child->onMoveEvent(e)) {
            handleMoveEventFlag = false;
        }
    }
    if (handleMoveEventFlag) {
        m_hovered = contains(e.x,e.y);
    }
    return handleMoveEventFlag;
}

bool FOGLWidget::onMouseEvent(const MouseEvent &e) {
    bool handleMouseEventFlag = true;
    if (!this->m_visible || !m_eventListen) {
        return handleMouseEventFlag;
    }

    // 倒序遍历子 Widget（Z-order 最前先处理）
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        auto child = *it;
        if (!child->m_visible) continue;

        MouseEvent me = e;
        me.x -= m_rect.x;
        me.y -= m_rect.y;
        if (child->contains(me.x, me.y)) {
            if (!child->onMouseEvent(me)) {  // 递归调用
                handleMouseEventFlag = false;
            }
        }
    }

    if (handleMouseEventFlag) {
        handleScrollEvent(e);
    }
    return handleMouseEventFlag;
}

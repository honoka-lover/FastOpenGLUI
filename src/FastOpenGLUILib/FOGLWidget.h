//
// Created by honoka on 2026/1/31.
//

#ifndef FASTOPENGLUI_FOGLWIDGET_H
#define FASTOPENGLUI_FOGLWIDGET_H

#include <functional>
#include <vector>
#include <memory>
#include <stdexcept>
#include <string>

#include "FOGLResourceManager.h"
#include <GLFW/glfw3.h>
struct FOGLRect { float x, y, width, height; };
enum class MouseButton { Left, Right, Middle, Other };
enum class MouseAction { Press, Release, Move, Scroll };

struct MouseEvent {
    // 鼠标相对于窗口的坐标
    double x = 0.0;
    double y = 0.0;

    // 鼠标相对于屏幕/绝对坐标（可选）
    double screenX = 0.0;
    double screenY = 0.0;

    MouseButton button = MouseButton::Left;
    MouseAction action = MouseAction::Move;

    double scrollX = 0.0;
    double scrollY = 0.0;

    int mods = 0;          // Ctrl / Shift / Alt
    GLFWwindow* window = nullptr; // 事件来源窗口
};

enum class FOGLWindowRole {
    None,        // 普通 widget
    TopLevel     // 需要 GLFWwindow
};

class FOGLRenderContext; // forward
class FOGLResourceManager;

class FOGLWidget :public std::enable_shared_from_this<FOGLWidget>{//shared_from_this()可以共享自己的this指针
public:
    using WindowRegisterFunc = std::function<void(std::shared_ptr<FOGLWidget>)>;

    FOGLWidget(const std::string& name = "");
    virtual ~FOGLWidget();

    bool isTopLevel() const { return m_windowRole == FOGLWindowRole::TopLevel; }

     // ✅ 添加 getter
    GLFWwindow* getGLFWwindowPointer() { return m_window; }

    void resignTopLevelWindow();

    void close();

    void becomeTopLevelWindow(int width = 600, int height = 400,bool transparentFrameBuffer = true, bool decorated = false);

    void setGeometry(float x, float y, float w, float h);

    void updateOffset();

    [[nodiscard]] const FOGLRect& geometry() const { return m_rect; }

    void addChild(std::shared_ptr<FOGLWidget> child);

    void setTopLevelRegisterFunc(WindowRegisterFunc func);

    void createNewTopLevelWindow(std::string title = "");

    //TODO: 待功能完成后补全attachChildren detachChildren
    void attachChildren(std::vector<std::shared_ptr<FOGLWidget>> children,bool asTopLevel = false);

    std::vector<std::shared_ptr<FOGLWidget>> detachChildren();   // 取走，但不破坏结构

    [[nodiscard]] FOGLWidget* parent() const { return m_parent; }

    void setFocus(bool focus) { m_focused = focus; }
    bool hasFocus() const { return m_focused; }

    void setDirty() {
        m_dirty = true;
        if (m_parent)
            m_parent->setDirty();
    }
    [[nodiscard]] bool isVisible() const {return m_visible;}

    [[nodiscard]] const std::vector<std::shared_ptr<FOGLWidget>>& children() const { return m_children; }

    bool contains(float px, float py) const;

    void raise() {
        if (!m_parent) return;

        auto& siblings = m_parent->m_children;
        auto it = std::find_if(
            siblings.begin(), siblings.end(),
            [&](auto& p) { return p.get() == this; }
        );
        if (it != siblings.end()) {
            auto self = *it;
            siblings.erase(it);
            siblings.push_back(self);
        }
    }

    virtual void onFocusIn() {}
    virtual void onFocusOut() {}

    void render();

    // Z-order 提升接口
    bool wantsBringToFront() const { return m_bringToFront; }
    void setBringToFrontFlag(bool flag) { m_bringToFront = flag; }

    // 鼠标事件接口（子类实现）
    virtual bool processMouseEvent(double mouseX, double mouseY, int button, int action);

    virtual void onInit() {}
    virtual void onLayout() { for (auto& c : m_children) c->onLayout(); }
    virtual void onPaint(FOGLRenderContext& ctx);
    virtual bool handleSelfMouseEvent(const MouseEvent& e);

    bool onMouseEvent(const MouseEvent& e);
protected:
    FOGLRect m_rect{};
    float m_xOffset = 0.0;
    float m_yOffset = 0.0;
    FOGLWidget* m_parent = nullptr;
    std::vector<std::shared_ptr<FOGLWidget>> m_children;
    bool m_visible = true;
    bool m_dirty = true;
    bool m_focused = false;
    bool m_bringToFront = false;  // 点击后是否希望提升到最上层

    FOGLWindowRole m_windowRole = FOGLWindowRole::None;

    GLFWwindow* m_nativeWindow = nullptr; // 仅 TopLevel 有效

    WindowRegisterFunc m_registerTopLevel = nullptr;

    static FOGLRenderContext m_ctx; // 延迟初始化

    std::shared_ptr<FOGLWidget> m_focusChild;
private:
    bool m_eventListen =true;

    std::string m_windowName;
    GLFWwindow* m_window = nullptr;
    FOGLResourceManager m_resourceManager;

    // 待提升的子 Widget（bringToFront 队列）
    std::vector<std::shared_ptr<FOGLWidget>> m_pendingBringToFront;

    // 仅在渲染或事件递归结束后调用
    void setFocusChild(std::shared_ptr<FOGLWidget> child) {
        if (m_focusChild == child) return;

        // 之前的失去焦点
        if (m_focusChild) m_focusChild->onFocusOut();

        m_focusChild = child;

        // 新的获得焦点
        if (m_focusChild) m_focusChild->onFocusIn();

        // 标记 Z-order 提升
        if (child) m_pendingBringToFront.push_back(child);
    }
};


#endif //FASTOPENGLUI_FOGLWIDGET_H
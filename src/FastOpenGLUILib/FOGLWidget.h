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
class FOGLContainer;

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
    FOGLWidget(const std::string& name = "");
    virtual ~FOGLWidget();

    // 判断是否为真实glfwwindow窗口，None为窗口内嵌的组件
    bool isTopLevel() const { return m_windowRole == FOGLWindowRole::TopLevel; }

     // 获取glfwwindow窗口指针，如果是窗口组件，该值为父窗口值
    GLFWwindow* getGLFWwindowPointer() { return m_window; }

    void resignTopLevelWindow();

    // 关闭窗口
    void close();

    // 组件提升为glfw窗口，转移原chlid数据， render函数调用后整理旧的”空窗口“
    void becomeTopLevelWindow(int width = 600, int height = 400,bool transparentFrameBuffer = true, bool decorated = false);

    // 设置窗口位置，宽高
    void setGeometry(float x, float y, float w, float h);

    void setGeometry(FOGLRect rect);
    // 更新偏移值，供子窗口渲染判断位置
    void updateOffset();

    // 获取窗口位置宽高
    [[nodiscard]] const FOGLRect& geometry() const { return m_rect; }

    // 将子窗口挂着到当前窗口下（ 暂时为处理FOGLWidget为顶层窗口时挂在情况 ）
    void addChild(std::shared_ptr<FOGLWidget> child);

    //TODO: 待功能完成后补全attachChildren detachChildren
    void attachChildren(std::vector<std::shared_ptr<FOGLWidget>> children,bool asTopLevel = false);

    std::vector<std::shared_ptr<FOGLWidget>> detachChildren();   // 取走，但不破坏结构

    [[nodiscard]] FOGLWidget* parent() const { return m_parent; }

    //设置填充控件，不需要在调用addChild
    void setContainer(const std::shared_ptr<FOGLContainer>& container);

    void setFocus(bool focus) { m_focused = focus; }
    bool hasFocus() const { return m_focused; }

    void setDirty() {
        m_dirty = true;
        if (m_parent)
            m_parent->setDirty();
    }
    [[nodiscard]] bool visible() const {return m_visible;}

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

    virtual void hide();

    virtual void show();

    virtual void onFocusIn() {}
    virtual void onFocusOut() {}

    void render();

    // Z-order 提升接口
    bool wantsBringToFront() const { return m_bringToFront; }
    void setBringToFrontFlag(bool flag) { m_bringToFront = flag; }

    // 鼠标事件接口（子类实现）
    virtual bool processMouseEvent(double mouseX, double mouseY, int button, int action);

    virtual void onInit() {}
    virtual void onLayout();
    virtual void onPaint(FOGLRenderContext& ctx);

    //返回值为true,事件继续传递，否则外层不再传递
    virtual bool handleSelfMouseEvent(const MouseEvent& e);

    //返回值为true,事件继续传递，否则外层不再传递
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

    static FOGLRenderContext m_ctx; // 延迟初始化

    std::shared_ptr<FOGLWidget> m_focusChild;
private:
    // 用来判断是否还需要处理窗口事件
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
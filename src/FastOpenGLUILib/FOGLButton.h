//
// Created by honoka on 2026/1/31.
//

#ifndef FASTOPENGLUI_FOGLBUTTON_H
#define FASTOPENGLUI_FOGLBUTTON_H


#include "FOGLWidget.h"
#include "FOGLRenderContext.h"
#include <string>
#include <functional>
#include <glad/glad.h>
#include <glm/glm.hpp>

class FOGLButton : public FOGLWidget {
public:
    FOGLButton(const std::string& text = "") : m_text(text) {}

    void setColor(const glm::vec4& color) { m_color = color; }
    void setHoverColor(const glm::vec4& color) { m_hoverColor = color; }
    void setClickEvent(std::function<void()> cb) { m_clickEvent = std::move(cb); }

    bool processMouseEvent(double x, double y, int button, int action) override {
        for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
            if ((*it)->processMouseEvent(x, y, button, action)) return true;
        }
        return false;
    }

    void setTexture(const std::string& name) {
        // auto data = FOGLResourceSystem::loadBinary(name);
        // if (data.empty()) return;
        // // TODO: 可以用 stbi_load_from_memory 上传纹理
        // // 这里给个占位
    }

    void onPaint(FOGLRenderContext& ctx) override {
        glm::vec4 color = m_hovered ? m_hoverColor : m_color;
        ctx.drawRect(m_rect.x+m_xOffset, m_rect.y+m_yOffset, m_rect.width, m_rect.height, color);
        FOGLWidget::onPaint(ctx);
    }

    bool handleSelfMouseEvent(const MouseEvent& e) override {
        if (e.button == MouseButton::Left && e.action == MouseAction::Press) {
            if (m_clickEvent)
                m_clickEvent();
            return false; // 事件被消耗
        }
        return true;
    }

private:
    std::function<void()> m_clickEvent;
    bool hitTest(float x, float y) const {
        return x >= m_rect.x && x <= m_rect.x + m_rect.width &&
               y >= m_rect.y && y <= m_rect.y + m_rect.height;
    }

    std::string m_text;
    glm::vec4 m_color = glm::vec4(0.2f, 0.2f, 0.8f, 1.0f);
    glm::vec4 m_hoverColor = glm::vec4(0.4f, 0.4f, 1.0f, 1.0f);

    bool m_hovered = false;
    bool m_pressed = false;
};


#endif //FASTOPENGLUI_FOGLBUTTON_H
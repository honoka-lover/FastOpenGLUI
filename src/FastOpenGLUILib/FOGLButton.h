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

    void setHoverTexture(const std::string& texture);

    void setTexture(const std::string& name);

    void setHoverTextureResource(int textureResourceId){ m_textureResourceIdHover = textureResourceId; }

    void setTextureResource(int textureResourceId) { m_textureResourceId = textureResourceId; }

    void onPaint(FOGLRenderContext& ctx) override;

    void handleSelfMouseEvent(const MouseEvent& e) override;

    bool onMouseEvent(const MouseEvent &e) override;
private:
    std::function<void()> m_clickEvent;
    bool hitTest(float x, float y) const {
        return x >= m_rect.x && x <= m_rect.x + m_rect.width &&
               y >= m_rect.y && y <= m_rect.y + m_rect.height;
    }

    std::string m_text;
    glm::vec4 m_color = glm::vec4(0.2f, 0.2f, 0.8f, 1.0f);
    glm::vec4 m_hoverColor = glm::vec4(0.4f, 0.4f, 1.0f, 1.0f);

    bool m_pressed = false;

    std::string m_texture{};
    std::string m_textureHover{};
    int m_textureResourceId = 0;
    int m_textureResourceIdHover = 0;
};


#endif //FASTOPENGLUI_FOGLBUTTON_H
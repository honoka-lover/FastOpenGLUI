//
// Created by honoka on 2026/1/31.
//

#include "FOGLButton.h"
#include <filesystem>

void FOGLButton::setHoverTexture(const std::string &texture) {
    if (std::filesystem::exists(texture)) {
        m_textureHover = texture;
    }
}

void FOGLButton::setTexture(const std::string &name) {
    if (std::filesystem::exists(name)) {
        m_texture = name;
    }
}

void FOGLButton::onPaint(FOGLRenderContext &ctx) {
    if (m_hovered) {
        if (m_textureHover.empty()) {
            if (m_textureResourceIdHover) {
                ctx.drawTexture(m_textureResourceIdHover,m_rect.x+m_xOffset, m_rect.y+m_yOffset, m_rect.width, m_rect.height,m_radius);
                return FOGLWidget::onPaint(ctx);
            }
        }else {
            ctx.drawTexture(m_textureHover,m_rect.x+m_xOffset, m_rect.y+m_yOffset, m_rect.width, m_rect.height,m_radius);
            return FOGLWidget::onPaint(ctx);
        }
        ctx.drawRect(m_rect.x+m_xOffset, m_rect.y+m_yOffset, m_rect.width, m_rect.height,m_hovered?m_hoverColor: m_color,m_radius);
        return FOGLWidget::onPaint(ctx);
    }

    if (!m_texture.empty()) {
        ctx.drawTexture(m_texture,m_rect.x+m_xOffset, m_rect.y+m_yOffset, m_rect.width, m_rect.height,m_radius);
    }else if (m_textureResourceId) {
        ctx.drawTexture(m_textureResourceId,m_rect.x+m_xOffset, m_rect.y+m_yOffset, m_rect.width, m_rect.height,m_radius);
    }else {
        ctx.drawRect(m_rect.x+m_xOffset, m_rect.y+m_yOffset, m_rect.width, m_rect.height,m_hovered?m_hoverColor: m_color,m_radius);
    }

    FOGLWidget::onPaint(ctx);
}

bool FOGLButton::onMouseEvent(const MouseEvent &e) {
    FOGLWidget::onMouseEvent(e);
    if (e.button == MouseButton::Left && e.action == MouseAction::Press) {
        if (m_clickEvent)
            m_clickEvent();
    }
    return false;
}

//
// Created by honoka on 2026/1/31.
//

#ifndef FASTOPENGLUI_FOGLSUBWINDOW_H
#define FASTOPENGLUI_FOGLSUBWINDOW_H
#include "FOGLWidget.h"
#include <string>

#include "FOGLRenderContext.h"
#include "glm/vec4.hpp"

class FOGLSubWindow : public FOGLWidget {
public:
    FOGLSubWindow(const std::string& title = "") : m_title(title) {}

    void onPaint(FOGLRenderContext& ctx) override { // 背景
        ctx.drawRect(m_rect.x+m_xOffset, m_rect.y+m_yOffset, m_rect.width, m_rect.height, glm::vec4(0.2f, 0.2f, 0.2f, 1.f));

        // 绘制子控件
        FOGLWidget::onPaint(ctx); }

    void handleSelfMouseEvent(const MouseEvent& e) override;
private:
    bool hitTest(float x, float y) const { return x >= m_rect.x && x <= m_rect.x+m_rect.width && y >= m_rect.y && y <= m_rect.y+m_rect.height; }
    std::string m_title;
    bool m_dragging = false;
    float m_dragOffsetX = 0, m_dragOffsetY = 0;
};


#endif //FASTOPENGLUI_FOGLSUBWINDOW_H
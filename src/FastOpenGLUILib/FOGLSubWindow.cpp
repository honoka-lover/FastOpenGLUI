//
// Created by honoka on 2026/1/31.
//

#include "FOGLSubWindow.h"

#include "GLFW/glfw3.h"

// bool FOGLSubWindow::processMouseEvent(double mouseX, double mouseY, int button, int action) {
//     if (!isVisible() || !contains(mouseX, mouseY)) return false;
//
//     if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
//         // 设置 Z-order 提升标志
//         setBringToFrontFlag(true);
//
//         m_dragging = true;
//         m_dragOffsetX = mouseX - m_rect.x;
//         m_dragOffsetY = mouseY - m_rect.y;
//
//         return true; // 事件被消耗
//     }
//
//     if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
//         m_dragging = false;
//         return true;
//     }
//
//     // 拖动逻辑
//     if (m_dragging) {
//         m_rect.x = mouseX - m_dragOffsetX;
//         m_rect.y = mouseY - m_dragOffsetY;
//         return true;
//     }
//
//     return false;
// }

bool FOGLSubWindow::handleSelfMouseEvent(const MouseEvent &e) {
    if (e.action == MouseAction::Press) {
        std::cout<<"test";
    }

    return FOGLWidget::handleSelfMouseEvent(e);
}

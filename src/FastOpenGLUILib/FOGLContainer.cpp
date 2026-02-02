//
// Created by honoka on 2026/1/31.
//

#include "FOGLContainer.h"

void FOGLContainer::setLayout(std::unique_ptr<FOGLLayout> layout) {
    m_layout = std::move(layout);
}


void FOGLContainer::onPaint(FOGLRenderContext &ctx) {
    if (!this->isTopLevel() && this->parent())
        this->m_rect = this->parent()->geometry();
    FOGLWidget::onPaint(ctx);
}



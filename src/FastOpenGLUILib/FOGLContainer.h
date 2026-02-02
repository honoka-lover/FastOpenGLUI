//
// Created by honoka on 2026/1/31.
//

#ifndef FASTOPENGLUI_FOGLCONTAINER_H
#define FASTOPENGLUI_FOGLCONTAINER_H
#include "FOGLWidget.h"
#include "FOGLLayout.h"
#include <memory>

class FOGLContainer : public FOGLWidget {
public:
    void setLayout(std::unique_ptr<FOGLLayout> layout);

    void onLayout() override {
        if (m_layout) m_layout->apply(this); // 应用布局
        FOGLWidget::onLayout();              // 递归布局子控件
    }

    void onPaint(FOGLRenderContext &ctx) override;

private:
    std::unique_ptr<FOGLLayout> m_layout;
};


#endif //FASTOPENGLUI_FOGLCONTAINER_H
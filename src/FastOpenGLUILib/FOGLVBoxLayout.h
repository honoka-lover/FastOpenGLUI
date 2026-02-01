//
// Created by honoka on 2026/1/31.
//

#ifndef FASTOPENGLUI_FOGLVBOXLAYOUT_H
#define FASTOPENGLUI_FOGLVBOXLAYOUT_H


#include "FOGLLayout.h"
#include "FOGLWidget.h"

class FOGLVBoxLayout : public FOGLLayout {
public:

    void setSpacing(float spacing);

    void apply(FOGLWidget* parent) override;

private:
    float m_spacing = 4.0f; // 子控件间距
};

#endif //FASTOPENGLUI_FOGLVBOXLAYOUT_H
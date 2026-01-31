//
// Created by honoka on 2026/1/31.
//

#ifndef FASTOPENGLUI_FOGLVBOXLAYOUT_H
#define FASTOPENGLUI_FOGLVBOXLAYOUT_H


#include "FOGLLayout.h"
#include "FOGLWidget.h"

class FOGLVBoxLayout : public FOGLLayout {
public:
    float spacing = 4.0f; // 子控件间距

    void apply(FOGLWidget* parent) override {
        float yOffset = 0.0f;

        for (auto& child : parent->children()) {
            if (!child->isVisible()) continue;
            FOGLRect geo = child->geometry();
            child->setGeometry(geo.x, yOffset, geo.width, geo.height);
            yOffset += geo.height + spacing;
        }
    }
};

#endif //FASTOPENGLUI_FOGLVBOXLAYOUT_H
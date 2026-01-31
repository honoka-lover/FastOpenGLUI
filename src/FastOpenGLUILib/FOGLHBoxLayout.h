//
// Created by honoka on 2026/1/31.
//

#ifndef FASTOPENGLUI_FOGLHBOXLAYOUT_H
#define FASTOPENGLUI_FOGLHBOXLAYOUT_H


#include "FOGLLayout.h"
#include "FOGLWidget.h"

class FOGLHBoxLayout : public FOGLLayout {
public:
    float spacing = 4.0f;

    void apply(FOGLWidget* parent) override {
        float xOffset = 0.0f;

        for (auto& child :  parent->children()) {
            if (!child->isVisible()) continue;

            FOGLRect geo = child->geometry();
            child->setGeometry(xOffset, geo.y, geo.width, geo.height);
            xOffset += geo.width + spacing;
        }
    }
};


#endif //FASTOPENGLUI_FOGLHBOXLAYOUT_H
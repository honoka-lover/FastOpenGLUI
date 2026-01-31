//
// Created by honoka on 2026/1/31.
//

#ifndef FASTOPENGLUI_FOGLGRIDLAYOUT_H
#define FASTOPENGLUI_FOGLGRIDLAYOUT_H
#include "FOGLLayout.h"
#include "FOGLWidget.h"
class FOGLGridLayout : public FOGLLayout {
public:
    int columns = 2;
    float spacing = 4.0f;

    void apply(FOGLWidget* parent) override {
        float xOffset = 0, yOffset = 0;
        int col = 0;
        for (auto& child :  parent->children()) {
            if (!child->isVisible()) continue;

            FOGLRect geo = child->geometry();
            child->setGeometry(xOffset, yOffset, geo.width, geo.height);

            col++;
            xOffset += geo.width + spacing;
            if (col >= columns) {
                col = 0;
                xOffset = 0;
                yOffset += geo.height + spacing;
            }
        }
    }
};

#endif //FASTOPENGLUI_FOGLGRIDLAYOUT_H
//
// Created by honoka on 2026/1/31.
//

#ifndef FASTOPENGLUI_FOGLHBOXLAYOUT_H
#define FASTOPENGLUI_FOGLHBOXLAYOUT_H


#include "FOGLLayout.h"
#include "FOGLWidget.h"

class FOGLHBoxLayout : public FOGLLayout {
public:


    void setSpacing(float spacing);

    void apply(FOGLWidget* parent) override;

private:
    float m_spacing = 4.0f;
};


#endif //FASTOPENGLUI_FOGLHBOXLAYOUT_H
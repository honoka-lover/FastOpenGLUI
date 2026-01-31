//
// Created by honoka on 2026/1/31.
//

#ifndef FASTOPENGLUI_FOGLLAYOUT_H
#define FASTOPENGLUI_FOGLLAYOUT_H
#include <memory>

class FOGLWidget;

class FOGLLayout {
public:
    virtual ~FOGLLayout() = default;

    // 将布局应用到父 Widget 的所有子 Widget 上
    virtual void apply(FOGLWidget* parent) = 0;
};

#endif //FASTOPENGLUI_FOGLLAYOUT_H
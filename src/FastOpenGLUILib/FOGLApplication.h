//
// Created by honoka on 2026/1/31.
//

#ifndef FASTOPENGLUI_FOGLAPPLICATION_H
#define FASTOPENGLUI_FOGLAPPLICATION_H
#include "vector"
#include "memory"
#include "FOGLWidget.h"
class FOGLApplication {
public:
    FOGLApplication();

    ~FOGLApplication();

    void addTopLevelWidget(std::shared_ptr<FOGLWidget> w);

    void run();

private:
    std::vector<std::shared_ptr<FOGLWidget>> m_roots;
};

#endif //FASTOPENGLUI_FOGLAPPLICATION_H
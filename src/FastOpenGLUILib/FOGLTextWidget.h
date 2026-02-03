//
// Created by honoka on 2026/2/3.
//

#ifndef FASTOPENGLUI_FOGLTEXTWIDGET_H
#define FASTOPENGLUI_FOGLTEXTWIDGET_H

#include "FOGLWidget.h"
#include "FOGLFont.h"
class FOGLTextWidget : public FOGLWidget {
public:
    FOGLTextWidget(const std::string& name = "");

    void setText(const std::string& text);
    void setColor(const glm::vec4& col) { color = col; }
    void setFont(const std::string& ttfFile,int pixelHeight);

    void onPaint(FOGLRenderContext& ctx) override;

protected:

    static FOGLFont font;
    std::string text;
    glm::vec4 color{1,1,1,1};

private:
};

#endif //FASTOPENGLUI_FOGLTEXTWIDGET_H
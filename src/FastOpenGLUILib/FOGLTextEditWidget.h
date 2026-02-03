//
// Created by honoka on 2026/2/3.
//

#ifndef FASTOPENGLUI_FOGLTEXTEDITWIDGET_H
#define FASTOPENGLUI_FOGLTEXTEDITWIDGET_H
#include "FOGLFunction.h"
#include "FOGLTextWidget.h"

class FOGLTextEditWidget : public FOGLTextWidget {
public:
    void setReadOnly(bool v){ readOnly=v; }

    size_t cursorIndex = 0;      // 光标位置
    size_t selectionStart = SIZE_MAX;
    size_t selectionEnd = SIZE_MAX;

    std::vector<TextFragment> fragments; // 富文本渲染

    FOGLTextEditWidget(const std::string& name = "")
        : FOGLTextWidget(name) {}

    // 解析 shell 颜色标签成 fragments
    void setRichTextFromShell(const std::string& shellText);

    // void onTextInput(const TextEvent& e) override;
    void onKeyEvent(const KeyEvent& e) override;

    void onPaint(FOGLRenderContext& ctx) override;
protected:
    // void drawFragments(FOGLRenderContext& ctx, float x, float y);
    // void drawCursor(FOGLRenderContext& ctx);
    bool readOnly = false;       // 是否只读
};




#endif //FASTOPENGLUI_FOGLTEXTEDITWIDGET_H

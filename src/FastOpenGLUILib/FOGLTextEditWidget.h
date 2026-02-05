//
// Created by honoka on 2026/2/3.
//

#ifndef FASTOPENGLUI_FOGLTEXTEDITWIDGET_H
#define FASTOPENGLUI_FOGLTEXTEDITWIDGET_H
#include "FOGLFunction.h"
#include "FOGLTextWidget.h"

class FOGLTextEditWidget : public FOGLTextWidget {
public:
    void setReadOnly(bool v){ m_readOnly=v; }

    FOGLTextEditWidget(const std::string& name = "")
        : FOGLTextWidget(name) {}

    // 解析 shell 颜色标签成 fragments
    void setRichTextFromShell(const std::string& shellText);

    // ===== 事件 =====
    bool onMouseEvent(const MouseEvent& e) override;
    void onTextInput(const TextEvent& e) override;
    void onKeyEvent(const KeyEvent& e) override;

    void onFocusIn() override;
    void onFocusOut() override;

    void onPaint(FOGLRenderContext& ctx) override;
protected:
    /// ===== 编辑状态 =====
    int m_caret = 0;              // UTF-32 index
    int m_selStart = -1;
    int m_selEnd   = -1;
    size_t cursor = 0;
    glm::vec4 cursorColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    bool cursorVisible = true;
    bool m_selecting = false;
    bool m_readOnly  = false;

    // ===== 光标闪烁 =====
    float m_blinkTimer = 0.f;
    bool  m_caretVisible = true;

    // ===== 布局缓存（每帧重建）=====
    struct GlyphBox {
        FOGLRect rect;
        int index;
    };
    std::vector<GlyphBox> m_hitBoxes;

    // ===== 缓存布局结果 =====
    struct GlyphLayout {
        uint32_t cp;
        glm::vec2 pos;
        glm::vec2 size;
        const FOGLGlyph* glyph;
        int index; // 在 codepoints 中的位置
    };

    std::vector<GlyphLayout> layout;  // 一次 paint 构建

    void copySelection();

    void pasteFromClipboard();

    int hitTest(float x, float y);

    void updateCodepoints() override;
};




#endif //FASTOPENGLUI_FOGLTEXTEDITWIDGET_H

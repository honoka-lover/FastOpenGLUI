//
// Created by honoka on 2026/2/3.
//

#include "FOGLTextEditWidget.h"

#include "FOGLFunction.h"

inline std::vector<TextFragment> parseShellColor(const std::string& s) {
    std::vector<TextFragment> out;
    glm::vec4 cur{1,1,1,1};
    std::string buf;

    for (size_t i=0;i<s.size();) {
        if (s[i]=='\x1b' && s[i+1]=='[') {
            if (!buf.empty()) out.push_back({buf,cur}), buf.clear();
            i+=2;
            int code = 0;
            while (isdigit(s[i])) code = code*10 + (s[i++]-'0');
            i++; // m
            if (code==31) cur={1,0,0,1};
            else if (code==32) cur={0,1,0,1};
            else if (code==0) cur={1,1,1,1};
        } else buf+=s[i++];
    }
    if (!buf.empty()) out.push_back({buf,cur});
    return out;
}


void FOGLTextEditWidget::setRichTextFromShell(const std::string &shellText) {
    // fragments.clear();
    // glm::vec4 currentColor = {1,1,1,1}; // 默认白色
    // size_t i = 0;
    // while(i < shellText.size()) {
    //     if(shellText[i] == '\x1b' && shellText[i+1] == '[') {
    //         size_t m = shellText.find('m', i);
    //         if(m != std::string::npos) {
    //             std::string code = shellText.substr(i+2, m-i-2);
    //             int c = std::stoi(code);
    //             // 简单映射常用颜色 30~37
    //             switch(c){
    //                 case 30: currentColor = {0,0,0,1}; break; // 黑
    //                 case 31: currentColor = {1,0,0,1}; break; // 红
    //                 case 32: currentColor = {0,1,0,1}; break; // 绿
    //                 case 33: currentColor = {1,1,0,1}; break; // 黄
    //                 case 34: currentColor = {0,0,1,1}; break; // 蓝
    //                 case 35: currentColor = {1,0,1,1}; break; // 紫
    //                 case 36: currentColor = {0,1,1,1}; break; // 青
    //                 case 37: currentColor = {1,1,1,1}; break; // 白
    //                 case 0:  currentColor = {1,1,1,1}; break; // 重置
    //             }
    //             i = m+1;
    //             continue;
    //         }
    //     }
    //     // 普通文字
    //     size_t j = i;
    //     while(j < shellText.size() && shellText[j] != '\x1b') j++;
    //     fragments.push_back({shellText.substr(i, j-i), currentColor});
    //     i = j;
    // }
}

int FOGLTextEditWidget::hitTest(float x, float y) {
    for (auto& g : layout) {
        if (x >= g.pos.x && x <= g.pos.x + g.size.x &&
            y >= g.pos.y && y <= g.pos.y + g.size.y)
            return g.index;
    }
    return m_codepoints.size();
}

void FOGLTextEditWidget::updateCodepoints() {
    // FOGLTextWidget::updateCodepoints();

}


bool FOGLTextEditWidget::onMouseEvent(const MouseEvent &e) {
    if (!visible()) return true;

    if (e.action == MouseAction::Press && contains(e.x, e.y)) {
        setFocus(true);
        m_selecting = true;

        m_caret = hitTest(e.x, e.y);
        m_selStart = m_selEnd = m_caret;

        setDirty();
        return false; // 吃掉
    }

    if (e.action == MouseAction::Move && m_selecting) {
        m_selEnd = hitTest(e.x, e.y);
        setDirty();
        return false;
    }

    if (e.action == MouseAction::Release) {
        m_selecting = false;
    }

    return true;
}

void FOGLTextEditWidget::onTextInput(const TextEvent &e) {
    if (m_readOnly || !hasFocus()) return;

    uint32_t cp = e.codepoint;

    // 删除选中
    if (m_selStart != m_selEnd) {
        int a = std::min(m_selStart, m_selEnd);
        int b = std::max(m_selStart, m_selEnd);
        m_codepoints.erase(m_codepoints.begin() + a,
                          m_codepoints.begin() + b);
        m_caret = a;
        m_selStart = m_selEnd = -1;
    }

    m_codepoints.insert(m_codepoints.begin() + m_caret, cp);
    m_caret++;

    setDirty();
}

void FOGLTextEditWidget::onPaint(FOGLRenderContext& ctx) {
    if (m_codepoints.empty()) return;

    layout.clear();

    float scale = fontSize / font.getBasePixelSize();
    float x = m_rect.x + 4.0f;
    float y = m_rect.y + font.getAscent() * scale + 4.0f;

    float lineHeight = font.getLineHeight() * scale;

    for (int i = 0; i < (int)m_codepoints.size(); ++i) {
        uint32_t cp = m_codepoints[i];

        if (cp == '\n') {
            x = m_rect.x + 4.0f;
            y += lineHeight;
            continue;
        }

        const auto& g = font.getGlyph(cp);

        GlyphLayout gl;
        gl.cp = cp;
        gl.glyph = &g;
        gl.index = i;

        gl.pos = {
            x + g.bearing.x * scale,
            y - g.bearing.y * scale
        };
        gl.size = {
            g.size.x * scale,
            g.size.y * scale
        };

        layout.push_back(gl);
        x += g.advance * scale;
    }

    // if (hasFocus() && cursorVisible) {
    //     float cx = calcCursorX(cursor);
    //
    //     ctx.drawRect(
    //         cx, m_rect.y + 4,
    //         1.5f, m_rect.height - 8,
    //         cursorColor
    //     );
    // }
}

void FOGLTextEditWidget::copySelection() {
}

void FOGLTextEditWidget::pasteFromClipboard() {
}


void FOGLTextEditWidget::onKeyEvent(const KeyEvent& e) {
    if (!hasFocus() || !e.pressed()) return;

    bool shift = e.mods & Mod_Shift;
    bool ctrl  = e.mods & Mod_Ctrl;

    switch (e.key) {

        case GLFW_KEY_LEFT:
            if (cursor > 0) cursor--;
            break;

        case GLFW_KEY_RIGHT:
            if (cursor < m_codepoints.size()) cursor++;
            break;

        case GLFW_KEY_BACKSPACE:
            if (m_selStart != m_selEnd) {
                // m_codepoints.erase(m_selStart, m_selEnd - m_selStart);
                cursor = m_selStart;
                m_selEnd = m_selStart;
            } else if (cursor > 0) {
                // m_codepoints.erase(cursor - 1, 1);
                cursor--;
            }
            break;

        case GLFW_KEY_DELETE:
            if (cursor < m_codepoints.size())
                // m_text.erase(cursor, 1);
            break;

        case GLFW_KEY_C:
            if (ctrl) copySelection();
            break;

        case GLFW_KEY_V:
            if (ctrl) pasteFromClipboard();
            break;

        case GLFW_KEY_A:
            if (ctrl) {
                m_selStart = 0;
                m_selEnd   = m_codepoints.size();
                cursor   = m_selEnd;
            }
            break;
    }

    setDirty();
}

void FOGLTextEditWidget::onFocusIn() {
    m_caretVisible = true;
    m_blinkTimer = 0.f;
}

void FOGLTextEditWidget::onFocusOut() {
    m_selStart = m_selEnd = -1;
    setDirty();
}

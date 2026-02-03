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
    fragments.clear();
    glm::vec4 currentColor = {1,1,1,1}; // 默认白色
    size_t i = 0;
    while(i < shellText.size()) {
        if(shellText[i] == '\x1b' && shellText[i+1] == '[') {
            size_t m = shellText.find('m', i);
            if(m != std::string::npos) {
                std::string code = shellText.substr(i+2, m-i-2);
                int c = std::stoi(code);
                // 简单映射常用颜色 30~37
                switch(c){
                    case 30: currentColor = {0,0,0,1}; break; // 黑
                    case 31: currentColor = {1,0,0,1}; break; // 红
                    case 32: currentColor = {0,1,0,1}; break; // 绿
                    case 33: currentColor = {1,1,0,1}; break; // 黄
                    case 34: currentColor = {0,0,1,1}; break; // 蓝
                    case 35: currentColor = {1,0,1,1}; break; // 紫
                    case 36: currentColor = {0,1,1,1}; break; // 青
                    case 37: currentColor = {1,1,1,1}; break; // 白
                    case 0:  currentColor = {1,1,1,1}; break; // 重置
                }
                i = m+1;
                continue;
            }
        }
        // 普通文字
        size_t j = i;
        while(j < shellText.size() && shellText[j] != '\x1b') j++;
        fragments.push_back({shellText.substr(i, j-i), currentColor});
        i = j;
    }
}

void FOGLTextEditWidget::onPaint(FOGLRenderContext& ctx) {
    // auto frags = parseShellColor(text);
    // std::vector<GlyphInstance> inst;
    //
    // float x = m_rect.x + 4;
    // float y = m_rect.y + font.getLineHeight();
    // // float y = m_rect.y + font->getLineHeight() - scrollY;
    //
    //
    // for (auto& f : frags) {
    //     auto cps = utf8_to_codepoints(f.text);
    //     for (auto cp : cps) {
    //         if (cp=='\n') {
    //             x = m_rect.x + 4;
    //             y += font.getLineHeight();
    //             continue;
    //         }
    //         auto& g = font.getGlyph(cp);
    //         inst.push_back({
    //             {x + g.bearing.x, y - g.bearing.y},
    //             g.size,
    //             g.uv0, g.uv1,
    //             f.color
    //         });
    //         x += g.advance;
    //     }
    // }
    // ctx.drawGlyphInstances(font.getAtlasTexture(), inst);
}

void FOGLTextEditWidget::onKeyEvent(const KeyEvent& e) {
    // if(readOnly) {
    //     // 只允许复制
    //     if(ctrlPressed && e.key == GLFW_KEY_C){
    //         glfwSetClipboardString(getGLFWwindowPointer(), utf8_text.c_str());
    //     }
    //     return;
    // }

    // 正常可编辑逻辑（光标移动 /删除 /插入）
}
//
// Created by honoka on 2026/2/3.
//

#ifndef FASTOPENGLUI_FOGLTEXTWIDGET_H
#define FASTOPENGLUI_FOGLTEXTWIDGET_H

#include <span>

#include "FOGLWidget.h"
#include "FOGLFont.h"

struct StyledGlyph { uint32_t codepoint; glm::vec4 color; };

struct TextColorRun {
    size_t start;   // glyph index
    size_t length;
    glm::vec4 color;
};


class FOGLTextWidget : public FOGLWidget {
public:
    enum class Encoding { UTF8, UTF16, UTF32, GB2312, ANSI };

    FOGLTextWidget(const std::string& name = "",Encoding enc= Encoding::UTF8);

    void setText(const char* str);
    void setText(const wchar_t* str);
    void setText(std::string_view str);               // 字符串字面量 / std::string
    void setText(std::wstring_view str);
    void setText(std::span<const char> str);
    void setText(std::span<const wchar_t> str);
    void setText(std::span<const char16_t> data);
    void setText(std::span<const char32_t> data);
    void setText(std::span<const char8_t> str);
    // void setText(std::u32string_view text,
    //          std::span<const TextColorRun> runs,
    //          const glm::vec4& defaultColor);

    void setStyledGlyphs(std::span<const StyledGlyph> glyphs);

    void setColor(const glm::vec4& col);
    // 获取文本，按指定编码输出
    std::string getText(Encoding enc) const;

    void onPaint(FOGLRenderContext& ctx) override;

    void setFontSize(float px) { fontSize = px; }

    static void setFont(const std::string& fontFile){ font.loadFromFile(fontFile,48);}
protected:
    static FOGLFont font;
    // std::string m_text;
    glm::vec4 m_color{1,1,1,1};
    std::vector<uint32_t> m_codepoints;     // UTF-32 缓存
    float fontSize = 16.0f; // 想要的显示字号（像素）
    std::vector<StyledGlyph> m_glyphs;

    virtual void updateCodepoints();

private:
    // 辅助函数
    std::u16string codepointsToUTF16() const;
    std::string codepointsToUTF8() const;
};

#endif //FASTOPENGLUI_FOGLTEXTWIDGET_H
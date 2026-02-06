//
// Created by honoka on 2026/2/3.
//

#include "FOGLTextWidget.h"

#include <filesystem>

#include "FOGLFunction.h"

FOGLFont FOGLTextWidget::font;

void FOGLTextWidget::updateCodepoints() {
    // 去掉 m_codepoints 尾部空字符
    while (!m_codepoints.empty() && m_codepoints.back() == 0) {
        m_codepoints.pop_back();
    }

    // 更新 glyphs
    m_glyphs.clear();
    m_glyphs.reserve(m_codepoints.size());
    for (auto cp : m_codepoints) {
        m_glyphs.push_back({ cp, m_color });
    }
}

FOGLTextWidget::FOGLTextWidget(const std::string &name,Encoding enc) : FOGLWidget(name) {

}

void FOGLTextWidget::setText(std::span<const char> str) {
    m_codepoints.assign(str.begin(), str.end());
    updateCodepoints();
}

void FOGLTextWidget::setText(std::span<const char16_t> data) {
    m_codepoints.assign(data.begin(), data.end());
    updateCodepoints();
}

void FOGLTextWidget::setText(std::span<const char32_t> data) {
    m_codepoints.assign(data.begin(), data.end());
    updateCodepoints();
}

void FOGLTextWidget::setText(std::span<const char8_t> str) {
    m_codepoints = utf8_to_codepoints(str);
    updateCodepoints();
}

// void FOGLTextWidget::setText(std::u32string_view text, std::span<const TextColorRun> runs,
//                              const glm::vec4 &defaultColor) {
// }

void FOGLTextWidget::setStyledGlyphs(std::span<const StyledGlyph> glyphs) {
    m_glyphs.clear();
    m_glyphs.reserve(glyphs.size());
    for (auto glyph : glyphs) {
        m_glyphs.push_back(glyph);
    }
}

void FOGLTextWidget::setText(const char* str) {
    setText(std::span<const char>{str, strlen(str)});
}

void FOGLTextWidget::setText(const wchar_t *str) {
    if (!str) return;

    // 先求长度
    size_t len = 0;
    while (str[len] != 0) ++len;

    // 用 span 包装
    setText(std::span<const wchar_t>{str, len});
}

// void FOGLTextWidget::setText(std::string_view str) {
//     // 转为 span<const char>
//     setText(std::span<const char>{str.data(), str.size()});
// }

// void FOGLTextWidget::setText(std::wstring_view str) {
//     setText(std::span<const wchar_t>{str.data(), str.size()});
// }

void FOGLTextWidget::setText(std::span<const wchar_t> str) {
    m_codepoints.clear();
    m_codepoints.reserve(str.size());

#if defined(_WIN32) && WCHAR_MAX == 0xFFFF
    // Windows 下 wchar_t 是 UTF-16
    for (size_t i = 0; i < str.size(); ++i) {
        wchar_t wc = str[i];
        if (wc >= 0xD800 && wc <= 0xDBFF && i + 1 < str.size()) {
            // 高代理 + 低代理 -> 1 个 code point
            wchar_t low = str[++i];
            uint32_t cp = ((wc - 0xD800) << 10) + (low - 0xDC00) + 0x10000;
            m_codepoints.push_back(cp);
        } else {
            m_codepoints.push_back(wc);
        }
    }
#else
    // Linux 下 wchar_t 通常是 UTF-32
    for (auto wc : str) {
        m_codepoints.push_back(wc);
    }
#endif

    // 去掉尾部空字符
    while (!m_codepoints.empty() && m_codepoints.back() == 0)
        m_codepoints.pop_back();

    // 更新 glyphs
    updateCodepoints();
}

void FOGLTextWidget::setColor(const glm::vec4 &col) {
    m_color = col;
    updateCodepoints();
}

// ---------------------- 辅助函数 ----------------------

std::u16string FOGLTextWidget::codepointsToUTF16() const {
    std::u16string u16;
    for (uint32_t cp : m_codepoints) {
        if (cp <= 0xFFFF) {
            u16.push_back(static_cast<char16_t>(cp));
        } else if (cp <= 0x10FFFF) {
            // 转 surrogate pair
            cp -= 0x10000;
            char16_t high = static_cast<char16_t>((cp >> 10) + 0xD800);
            char16_t low  = static_cast<char16_t>((cp & 0x3FF) + 0xDC00);
            u16.push_back(high);
            u16.push_back(low);
        }
        // 超过 Unicode 范围忽略
    }
    return u16;
}

std::string FOGLTextWidget::codepointsToUTF8() const {
    std::string utf8;
    for (uint32_t cp : m_codepoints) {
        if (cp <= 0x7F) {
            utf8.push_back(static_cast<char>(cp));
        } else if (cp <= 0x7FF) {
            utf8.push_back(static_cast<char>(0xC0 | ((cp >> 6) & 0x1F)));
            utf8.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else if (cp <= 0xFFFF) {
            utf8.push_back(static_cast<char>(0xE0 | ((cp >> 12) & 0x0F)));
            utf8.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            utf8.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else if (cp <= 0x10FFFF) {
            utf8.push_back(static_cast<char>(0xF0 | ((cp >> 18) & 0x07)));
            utf8.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
            utf8.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            utf8.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        }
    }
    return utf8;
}


std::string FOGLTextWidget::getText(Encoding enc) const {
    switch(enc) {
        case Encoding::UTF8:
            return codepointsToUTF8();

        case Encoding::UTF16: {
            auto u16 = codepointsToUTF16();
            return std::string(reinterpret_cast<const char*>(u16.data()), u16.size() * 2);
        }

        case Encoding::UTF32: {
            return std::string(reinterpret_cast<const char*>(m_codepoints.data()), m_codepoints.size() * 4);
        }

        case Encoding::GB2312:
        case Encoding::ANSI: {
            // Windows 下 WideCharToMultiByte
            auto u16 = codepointsToUTF16();
            if (u16.empty()) return {};

            UINT codePage = (enc == Encoding::GB2312) ? 936 : CP_ACP;

            int len = WideCharToMultiByte(codePage, 0, reinterpret_cast<LPCWCH>(u16.data()),
                                          (int)u16.size(), nullptr, 0, nullptr, nullptr);
            if (len <= 0) return {};

            std::string out(len, 0);
            WideCharToMultiByte(codePage, 0, reinterpret_cast<LPCWCH>(u16.data()),
                                (int)u16.size(), out.data(), len, nullptr, nullptr);
            return out;
        }
    }

    return {};
}

void FOGLTextWidget::onPaint(FOGLRenderContext &ctx) {
    if (m_glyphs.empty()) return;

    // === 字号缩放因子 ===
    float scale = fontSize / static_cast<float>(font.getBasePixelSize());

    // 按纹理页分组
    std::unordered_map<int, std::vector<GlyphInstance>> pageInstances;

    // 计算总宽度
    float totalWidth = 0;
    for (auto glyph : m_glyphs) {
        totalWidth += font.getGlyph(glyph.codepoint).advance*scale;
    }

    // 居中计算
    float penX = m_rect.x + (m_rect.width - totalWidth) * 0.5f;
    float fontHeight = font.getAscent() + font.getDescent()* scale;
    float baseline = m_rect.y + (m_rect.height - fontHeight) * 0.5f + font.getAscent()* scale;

    // === 生成 GlyphInstance ===
    for (auto glyph : m_glyphs) {
        const auto& g = font.getGlyph(glyph.codepoint);

        GlyphInstance inst{};
        inst.position = {
            penX + g.bearing.x * scale,
            baseline - g.bearing.y * scale
        };

        inst.size = {
            g.size.x * scale,
            g.size.y * scale
        };

        inst.uv0 = g.uv0;
        inst.uv1 = g.uv1;
        inst.color = glyph.color;

        pageInstances[g.texturePage].push_back(inst);

        penX += g.advance * scale;
    }

    // === 分页绘制 ===
    for (auto& [pageIdx, instances] : pageInstances) {
        ctx.drawGlyphInstances(font.getTexturePage(pageIdx), instances);
    }
}



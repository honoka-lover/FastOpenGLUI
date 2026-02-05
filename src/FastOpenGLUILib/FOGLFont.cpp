//
// Created by honoka on 2026/2/3.
//

#include "FOGLFont.h"

#include <filesystem>
#include <fstream>

#include <ft2build.h>
#include <thread>

#include FT_FREETYPE_H

FT_Library ft{nullptr};
FT_Face face{nullptr};

FOGLFont::~FOGLFont() {

}

// 在 loadFromFile 后预加载
void FOGLFont::preloadCommonChars() {
    // 中文常用 3500 字
    const char* commonChinese = "的一是在不了有和人这中大为上个国我以要他时来用们生到作地于出就分对成会可主发年动同工也能下过子说产种面而方后多定行学法所民得经十三之进着等部度家电力里如水化高自二理起小物现实加量都两体制机当使点从业本去把性好应开它合还因由其些然前外天政四日那社义事平形相全表间样与关各重新线内数正心反你明看原又么利比或但质气第向道命此变条只没结解问意建月公无系军很情者最立代想已通并提直题党程展五果料象员革位入常文总次品式活设及管特件长求老头基资边流路级少图山统接知较将组见计别她手角期根论运农指几九区强放决西被干做必战先回则任取据处队南给色光门即保治北造百规热领七海口东导器压志世金增争济阶油思术极交受联什认六共权收证改清己美再采转更单风切打白教速花带安场身车例真务具万每目至达走积示议声报斗完类八离华名确才科张信马节话米整空元况今集温传土许步群广石记需段研界拉林律叫且究观越织装影算低持音众书布复容儿须际商非验连断深难近矿千周委素技备半办青省列习响约支般史感劳便团往酸历市克何除消构府称太准精值号率族维划选标写存候毛亲快效斯院查江型眼王按格养易置派层片始却专状育厂京识适属圆包火住调满县局照参红细引听该铁价严";

    for (const char* p = commonChinese; *p; ) {
        uint32_t cp = 0;
        int len = 0;

        // 简单 UTF-8 解码
        if ((*p & 0x80) == 0) { cp = *p; len = 1; }
        else if ((*p & 0xE0) == 0xC0) {
            cp = ((*p & 0x1F) << 6) | (*(p+1) & 0x3F); len = 2;
        }
        else if ((*p & 0xF0) == 0xE0) {
            cp = ((*p & 0x0F) << 12) | ((*(p+1) & 0x3F) << 6) | (*(p+2) & 0x3F);
            len = 3;
        }

        addGlyphToAtlas(cp);
        p += len;
    }
}

bool FOGLFont::loadFromFile(const std::string& ttfPath, int pixelHeight) {
    cleanup();

    if (FT_Init_FreeType(&ft)) return false;

    if (FT_New_Face(ft, ttfPath.c_str(), 0, &face)) {
        FT_Done_FreeType(ft);
        ft = nullptr;
        return false;
    }

    FT_Set_Pixel_Sizes(face, 0, pixelHeight);
    m_pixelHeight = pixelHeight;
    m_basePixelSize = pixelHeight;
    // 存储字体度量
    m_lineHeight = face->size->metrics.height >> 6;
    m_ascent = face->size->metrics.ascender >> 6;
    m_descent = -(face->size->metrics.descender >> 6);

    // 创建第一个纹理页
    m_pages.push_back(std::make_unique<TexturePage>(m_atlasW, m_atlasH));

    // 预加载常用 ASCII 字符（可选，加快首次渲染）
    for (uint32_t c = 32; c <= 126; ++c) {
        addGlyphToAtlas(c);
    }

    // 创建默认字形（空白方框）
    m_defaultGlyph.size = {float(pixelHeight/2), float(pixelHeight)};
    m_defaultGlyph.bearing = {0, float(pixelHeight)};
    m_defaultGlyph.advance = float(pixelHeight/2);
    m_defaultGlyph.uv0 = {0, 0};
    m_defaultGlyph.uv1 = {0, 0};
    m_defaultGlyph.texturePage = 0;

    initialized = true;
    return true;
}

const FOGLGlyph & FOGLFont::getGlyph(uint32_t codepoint) {
    if (!initialized)
        initFont();
    // O(1) 查找
    auto it = m_glyphs.find(codepoint);
    if (it != m_glyphs.end()) {
        return it->second;
    }

    // 首次遇到，动态添加
    if (addGlyphToAtlas(codepoint)) {
        return m_glyphs[codepoint];
    }

    // 添加失败，返回默认字形
    return m_defaultGlyph;
}

void FOGLFont::initFont() {
    if (std::filesystem::exists("C:/Windows/Fonts/msyh.ttc"))
        loadFromFile("C:/Windows/Fonts/msyh.ttc",48);
    else if (std::filesystem::exists("C:/Windows/Fonts/msyh.ttf"))
        loadFromFile("C:/Windows/Fonts/msyh.ttf",48);
    initialized = true;
//     std::thread preloadThread([this]() {
//         preloadCommonChars();
//     });
//     preloadThread.detach();
}

bool FOGLFont::addGlyphToAtlas(uint32_t codepoint) {
    if (!face) return false;

    // ===== 加载字形（强制使用抗锯齿渲染）=====
    if (FT_Load_Char(face, codepoint, FT_LOAD_RENDER)) {
        printf("Failed to load char U+%04X\n", codepoint);
        return false;
    }

    FT_GlyphSlot g = face->glyph;
    FT_Bitmap& bitmap = g->bitmap;

    int glyphW = bitmap.width;
    int glyphH = bitmap.rows;

    // 跳过空白字形（如空格）
    if (glyphW == 0 || glyphH == 0) {
        FOGLGlyph glyph;
        glyph.size = {0, 0};
        glyph.bearing = {float(g->bitmap_left), float(g->bitmap_top)};
        glyph.advance = float(g->advance.x >> 6);
        glyph.uv0 = {0, 0};
        glyph.uv1 = {0, 0};
        glyph.texturePage = 0;
        m_glyphs[codepoint] = glyph;
        return true;
    }

    // ===== 检查 bitmap 格式 =====
    if (bitmap.pixel_mode != FT_PIXEL_MODE_GRAY) {
        printf("Warning: Unsupported pixel mode %d for char U+%04X\n",
               bitmap.pixel_mode, codepoint);
        // 尝试转换或跳过
        return false;
    }

    // 查找可用纹理页
    int pageIndex = -1;
    TexturePage* targetPage = nullptr;

    for (int i = 0; i < m_pages.size(); ++i) {
        auto& page = m_pages[i];

        // 检查当前行
        if (page->penX + glyphW + 1 < m_atlasW) {
            if (page->penY + glyphH < m_atlasH) {
                targetPage = page.get();
                pageIndex = i;
                break;
            }
        }

        // 尝试换行
        if (page->penY + page->rowH + 1 + glyphH < m_atlasH) {
            page->penX = 1;
            page->penY += page->rowH + 1;
            page->rowH = 0;
            targetPage = page.get();
            pageIndex = i;
            break;
        }
    }

    // 创建新页
    if (!targetPage) {
        m_pages.push_back(std::make_unique<TexturePage>(m_atlasW, m_atlasH));
        targetPage = m_pages.back().get();
        pageIndex = m_pages.size() - 1;
        printf("Created texture page %d for char U+%04X\n", pageIndex, codepoint);
    }

    int penX = targetPage->penX;
    int penY = targetPage->penY;

    // ===== 正确处理 FreeType bitmap =====
    std::vector<uint8_t> glyphPixels(glyphW * glyphH * 4);

    for (int y = 0; y < glyphH; ++y) {
        for (int x = 0; x < glyphW; ++x) {
            // 关键：使用 bitmap.pitch（可能 != width）
            int srcIdx = y * bitmap.pitch + x;

            // 检查边界
            if (srcIdx >= bitmap.pitch * bitmap.rows) {
                printf("ERROR: srcIdx out of bounds: %d >= %d\n",
                       srcIdx, bitmap.pitch * bitmap.rows);
                continue;
            }

            uint8_t alpha = bitmap.buffer[srcIdx];

            int dstIdx = (y * glyphW + x) * 4;
            glyphPixels[dstIdx + 0] = 255;  // R
            glyphPixels[dstIdx + 1] = 255;  // G
            glyphPixels[dstIdx + 2] = 255;  // B
            glyphPixels[dstIdx + 3] = alpha; // A
        }
    }

    // 更新 CPU 缓冲
    for (int y = 0; y < glyphH; ++y) {
        for (int x = 0; x < glyphW; ++x) {
            int atlasIdx = ((penY + y) * m_atlasW + (penX + x)) * 4;
            int glyphIdx = (y * glyphW + x) * 4;

            targetPage->pixels[atlasIdx + 0] = glyphPixels[glyphIdx + 0];
            targetPage->pixels[atlasIdx + 1] = glyphPixels[glyphIdx + 1];
            targetPage->pixels[atlasIdx + 2] = glyphPixels[glyphIdx + 2];
            targetPage->pixels[atlasIdx + 3] = glyphPixels[glyphIdx + 3];
        }
    }

    // 更新 GPU 纹理
    glBindTexture(GL_TEXTURE_2D, targetPage->texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);  // 重要：重置行长度

    glTexSubImage2D(GL_TEXTURE_2D, 0,
                    penX, penY,
                    glyphW, glyphH,
                    GL_RGBA, GL_UNSIGNED_BYTE,
                    glyphPixels.data());

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        printf("OpenGL error after glTexSubImage2D: 0x%x\n", err);
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    // 存储字形信息
    FOGLGlyph glyph;
    glyph.size = {float(glyphW), float(glyphH)};
    glyph.bearing = {float(g->bitmap_left), float(g->bitmap_top)};
    glyph.advance = float(g->advance.x >> 6);

    // UV 坐标（添加半像素偏移避免采样问题）
    float uvOffsetX = 0.5f / m_atlasW;
    float uvOffsetY = 0.5f / m_atlasH;
    glyph.uv0 = {
        (float(penX) + uvOffsetX) / m_atlasW,
        (float(penY) + uvOffsetY) / m_atlasH
    };
    glyph.uv1 = {
        (float(penX + glyphW) - uvOffsetX) / m_atlasW,
        (float(penY + glyphH) - uvOffsetY) / m_atlasH
    };
    glyph.texturePage = pageIndex;

    m_glyphs[codepoint] = glyph;

    // 更新打包位置
    targetPage->penX += glyphW + 1;
    targetPage->rowH = std::max(targetPage->rowH, glyphH);

    return true;
}

GLuint FOGLFont::getTexturePage(int index) const {
    if (index >= 0 && index < m_pages.size()) {
        return m_pages[index]->texture;
    }
    return 0;
}

void FOGLFont::cleanup() {
    m_glyphs.clear();
    m_pages.clear();

    if (face) {
        FT_Done_Face(face);
        face = nullptr;
    }
    if (ft) {
        FT_Done_FreeType(ft);
        ft = nullptr;
    }
}

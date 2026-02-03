//
// Created by honoka on 2026/2/3.
//

#ifndef FASTOPENGLUI_FOGLFONT_H
#define FASTOPENGLUI_FOGLFONT_H
#include <glad/glad.h>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include <vector>
#include <cstdint>
#include <memory>

// 字形信息（添加纹理页索引）
struct FOGLGlyph {
    glm::vec2 uv0;
    glm::vec2 uv1;
    glm::vec2 size;
    glm::vec2 bearing;
    float advance;
    int texturePage;  // 所在纹理页索引
};

// 纹理页
struct TexturePage {
    GLuint texture;
    std::vector<uint8_t> pixels;  // CPU 端备份
    int penX, penY, rowH;         // 打包位置

    TexturePage(int w, int h)
        : texture(0), pixels(w * h * 4, 0),
          penX(1), penY(1), rowH(0) {
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    ~TexturePage() {
        if (texture) glDeleteTextures(1, &texture);
    }
};

// 单张字体纹理（Glyph Atlas）
class FOGLFont {
public:
    FOGLFont() : m_atlasW(1024), m_atlasH(1024) {}
    ~FOGLFont();

    // 加载 TTF 字体，生成 glyph atlas
    bool loadFromFile(const std::string& ttfPath, int pixelHeight);

    // 获取单个字形
    const FOGLGlyph& getGlyph(uint32_t codepoint);
    GLuint getTexturePage(int index) const;

    float getAscent() const { return m_ascent; }
    float getDescent() const { return m_descent; }
    float getLineHeight() const { return m_lineHeight; }

    void preloadCommonChars() ;
private:
    void initFont();
    void cleanup();
    bool addGlyphToAtlas(uint32_t codepoint);

    std::unordered_map<uint32_t, FOGLGlyph> m_glyphs;  // 快速查找
    std::vector<std::unique_ptr<TexturePage>> m_pages; // 纹理页

    bool initialized = false;
    int m_atlasW, m_atlasH;
    float m_ascent{0}, m_descent{0};
    int m_lineHeight{0};
    int m_pixelHeight{0};
    // 默认字形（用于缺失字符）
    FOGLGlyph m_defaultGlyph;
};


#endif //FASTOPENGLUI_FOGLFONT_H
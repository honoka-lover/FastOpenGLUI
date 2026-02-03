//
// Created by honoka on 2026/2/3.
//

#include "FOGLTextWidget.h"

#include <filesystem>

#include "FOGLFunction.h"

FOGLFont FOGLTextWidget::font;

FOGLTextWidget::FOGLTextWidget(const std::string &name) : FOGLWidget(name) {

}

void FOGLTextWidget::setText(const std::string &t) {
    this->text = t;
}

void FOGLTextWidget::setFont(const std::string &ttfFile,int pixelHeight) {
    font.loadFromFile(ttfFile,pixelHeight);
}

void FOGLTextWidget::onPaint(FOGLRenderContext &ctx) {
    if (text.empty()) return;

    auto cps = utf8_to_codepoints(text);

    // 按纹理页分组
    std::unordered_map<int, std::vector<GlyphInstance>> pageInstances;

    // 计算总宽度
    float totalWidth = 0;
    for (auto cp : cps) {
        totalWidth += font.getGlyph(cp).advance;
    }

    // 居中计算
    float penX = m_rect.x + (m_rect.width - totalWidth) * 0.5f;
    float fontHeight = font.getAscent() + font.getDescent();
    float baseline = m_rect.y + (m_rect.height - fontHeight) * 0.5f + font.getAscent();

    // 生成实例（按纹理页分组）
    for (auto cp : cps) {
        const auto& g = font.getGlyph(cp);  // 自动按需加载

        GlyphInstance inst;
        inst.position = {penX + g.bearing.x, baseline - g.bearing.y};
        inst.size = g.size;
        inst.uv0 = g.uv0;
        inst.uv1 = g.uv1;
        inst.color = color;

        pageInstances[g.texturePage].push_back(inst);
        penX += g.advance;
    }

    // 分批渲染（每个纹理页一次 draw call）
    for (auto& [pageIdx, instances] : pageInstances) {
        GLuint texture = font.getTexturePage(pageIdx);
        ctx.drawGlyphInstances(texture, instances);
    }
}



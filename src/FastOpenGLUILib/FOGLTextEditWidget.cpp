//
// Created by honoka on 2026/2/3.
// Improved implementation
//

#include "FOGLTextEditWidget.h"
#include "FOGLFunction.h"
#include <algorithm>
#include <GLFW/glfw3.h>

// ===== 富文本解析 =====

void FOGLTextEditWidget::setText(const char *str) {
    FOGLTextWidget::setText(str);
    rebuildLines();
}

void FOGLTextEditWidget::setText(const wchar_t *str) {
    FOGLTextWidget::setText(str);
    rebuildLines();
}

void FOGLTextEditWidget::setText(std::span<const char> str) {
    FOGLTextWidget::setText(str);
    rebuildLines();
}

void FOGLTextEditWidget::setText(std::span<const wchar_t> str) {
    FOGLTextWidget::setText(str);
    rebuildLines();
}

void FOGLTextEditWidget::setText(std::span<const char8_t> str) {
    FOGLTextWidget::setText(str);
    rebuildLines();
}

void FOGLTextEditWidget::setText(std::span<const char16_t> str) {
    FOGLTextWidget::setText(str);
    rebuildLines();
}

void FOGLTextEditWidget::setText(std::span<const char32_t> str) {
    FOGLTextWidget::setText(str);
    rebuildLines();
}

void FOGLTextEditWidget::setRichTextFromShell(const std::string& shellText) {
    std::vector<StyledGlyph> glyphs;
    glyphs.reserve(shellText.size());

    glm::vec4 currentColor = {1, 1, 1, 1}; // 默认白色

    for (size_t i = 0; i < shellText.size(); ) {
        // 检测 ANSI 转义序列 \x1b[...m
        if (i + 1 < shellText.size() && shellText[i] == '\x1b' && shellText[i + 1] == '[') {
            size_t m = shellText.find('m', i + 2);
            if (m != std::string::npos) {
                std::string code = shellText.substr(i + 2, m - i - 2);

                // 解析颜色代码
                int colorCode = std::stoi(code);
                switch (colorCode) {
                    case 0:  currentColor = {1, 1, 1, 1}; break; // 重置
                    case 30: currentColor = {0, 0, 0, 1}; break; // 黑
                    case 31: currentColor = {1, 0, 0, 1}; break; // 红
                    case 32: currentColor = {0, 1, 0, 1}; break; // 绿
                    case 33: currentColor = {1, 1, 0, 1}; break; // 黄
                    case 34: currentColor = {0, 0, 1, 1}; break; // 蓝
                    case 35: currentColor = {1, 0, 1, 1}; break; // 紫
                    case 36: currentColor = {0, 1, 1, 1}; break; // 青
                    case 37: currentColor = {1, 1, 1, 1}; break; // 白
                }

                i = m + 1;
                continue;
            }
        }

        // 普通字符
        uint32_t codepoint = static_cast<uint8_t>(shellText[i]);
        glyphs.push_back({codepoint, currentColor});
        i++;
    }

    setStyledGlyphs(glyphs);
    rebuildLines();
}

void FOGLTextEditWidget::setRichTextFragments(const std::vector<TextFragment>& fragments) {
    m_glyphs.clear();
    m_codepoints.clear();

    for (const auto& frag : fragments) {
        auto codepoints = utf8_to_codepoints(
            std::span<const char8_t>(
                reinterpret_cast<const char8_t*>(frag.text.data()),
                frag.text.size()
            )
        );

        for (uint32_t cp : codepoints) {
            m_codepoints.push_back(cp);
            m_glyphs.push_back({cp, frag.color});
        }
    }

    rebuildLines();
}

void FOGLTextEditWidget::insertRichText(size_t position, const std::string& text, const glm::vec4& color) {
    auto codepoints = utf8_to_codepoints(
        std::span<const char8_t>(
            reinterpret_cast<const char8_t*>(text.data()),
            text.size()
        )
    );

    position = std::min(position, m_codepoints.size());

    for (size_t i = 0; i < codepoints.size(); i++) {
        m_codepoints.insert(m_codepoints.begin() + position + i, codepoints[i]);
        m_glyphs.insert(m_glyphs.begin() + position + i, {codepoints[i], color});
    }

    rebuildLines();
}

void FOGLTextEditWidget::setColorRange(size_t start, size_t end, const glm::vec4& color) {
    start = std::min(start, m_glyphs.size());
    end = std::min(end, m_glyphs.size());

    for (size_t i = start; i < end; i++) {
        m_glyphs[i].color = color;
    }
}

// ===== 光标和选择 =====

void FOGLTextEditWidget::setCursorPosition(size_t pos) {
    m_caret = std::min(pos, m_codepoints.size());
    scrollToCursor();
}

void FOGLTextEditWidget::setSelection(size_t start, size_t end) {
    m_selStart = std::min(start, m_codepoints.size());
    m_selEnd = std::min(end, m_codepoints.size());
    setDirty();
}

void FOGLTextEditWidget::clearSelection() {
    m_selStart = m_caret;
    m_selEnd = m_caret;
    setDirty();
}

// ===== 行管理 =====

void FOGLTextEditWidget::rebuildLines() {
    m_lines.clear();

    float scale = fontSize / static_cast<float>(font.getBasePixelSize());
    float lineHeight = font.getLineHeight() * scale;
    float maxWidth = m_wordWrap ? (m_rect.width - 8.0f) : 1e6f;

    // 处理空文本的情况
    if (m_codepoints.empty()) {
        // 创建一个空行，确保有地方放光标
        m_lines.push_back({0, 0, 0.0f, lineHeight});
        updateVisibleRange();
        return;
    }

    size_t lineStart = 0;
    float x = 0.0f;
    float y = 0.0f;

    for (size_t i = 0; i < m_codepoints.size(); i++) {
        uint32_t cp = m_codepoints[i];

        // 换行符
        if (cp == '\n') {
            m_lines.push_back({lineStart, i - lineStart, y, lineHeight});
            lineStart = i + 1;
            x = 0.0f;
            y += lineHeight;
            continue;
        }

        const auto& glyph = font.getGlyph(cp);
        float advance = glyph.advance * scale;

        // 自动换行
        if (m_wordWrap && x + advance > maxWidth && i > lineStart) {
            m_lines.push_back({lineStart, i - lineStart, y, lineHeight});
            lineStart = i;
            x = 0.0f;
            y += lineHeight;
        }

        x += advance;
    }

    // 最后一行
    if (lineStart <= m_codepoints.size()) {
        m_lines.push_back({lineStart, m_codepoints.size() - lineStart, y, lineHeight});
    }

    // 确保至少有一行
    if (m_lines.empty()) {
        m_lines.push_back({0, 0, 0.0f, lineHeight});
    }

    updateVisibleRange();
}

void FOGLTextEditWidget::updateVisibleRange() {
    if (m_lines.empty()) {
        m_visibleLineStart = m_visibleLineEnd = 0;
        return;
    }

    float viewTop = m_scrollY;
    float viewBottom = m_scrollY + m_rect.height;

    m_visibleLineStart = 0;
    m_visibleLineEnd = m_lines.size();

    // 二分查找可见范围
    for (size_t i = 0; i < m_lines.size(); i++) {
        if (m_lines[i].yPosition + m_lines[i].height >= viewTop) {
            m_visibleLineStart = i;
            break;
        }
    }

    for (size_t i = m_visibleLineStart; i < m_lines.size(); i++) {
        if (m_lines[i].yPosition > viewBottom) {
            m_visibleLineEnd = i;
            break;
        }
    }

    // 确保至少显示一行
    if (m_visibleLineStart >= m_visibleLineEnd && !m_lines.empty()) {
        m_visibleLineEnd = m_visibleLineStart + 1;
    }
}

size_t FOGLTextEditWidget::getLineForIndex(size_t index) const {
    if (m_lines.empty()) return 0;

    for (size_t i = 0; i < m_lines.size(); i++) {
        const auto& line = m_lines[i];
        if (index >= line.startIndex && index <= line.startIndex + line.length) {
            return i;
        }
    }

    // 如果索引超出范围，返回最后一行
    return m_lines.size() > 0 ? m_lines.size() - 1 : 0;
}

void FOGLTextEditWidget::scrollToLine(size_t lineIndex) {
    if (lineIndex >= m_lines.size()) return;

    const auto& line = m_lines[lineIndex];
    float targetY = line.yPosition;

    // 确保行在可见区域内
    if (targetY < m_scrollY) {
        m_scrollY = targetY;
    } else if (targetY + line.height > m_scrollY + m_rect.height) {
        m_scrollY = targetY + line.height - m_rect.height;
    }

    m_scrollY = std::max(0.0f, m_scrollY);
    updateVisibleRange();
    setDirty();
}

void FOGLTextEditWidget::scrollToCursor() {
    size_t lineIndex = getLineForIndex(m_caret);
    scrollToLine(lineIndex);
}

// ===== 复制粘贴 =====

std::string FOGLTextEditWidget::getSelectedText() const {
    if (!hasSelection()) return "";

    auto [start, end] = getSelection();

    std::string result;
    for (size_t i = start; i < end && i < m_codepoints.size(); i++) {
        uint32_t cp = m_codepoints[i];

        // UTF-8 编码
        if (cp <= 0x7F) {
            result.push_back(static_cast<char>(cp));
        } else if (cp <= 0x7FF) {
            result.push_back(static_cast<char>(0xC0 | ((cp >> 6) & 0x1F)));
            result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else if (cp <= 0xFFFF) {
            result.push_back(static_cast<char>(0xE0 | ((cp >> 12) & 0x0F)));
            result.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else if (cp <= 0x10FFFF) {
            result.push_back(static_cast<char>(0xF0 | ((cp >> 18) & 0x07)));
            result.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        }
    }

    return result;
}

void FOGLTextEditWidget::copySelection() {
    if (!hasSelection()) return;

    std::string text = getSelectedText();
    glfwSetClipboardString(nullptr, text.c_str());
}

void FOGLTextEditWidget::cutSelection() {
    if (m_readOnly || !hasSelection()) return;

    copySelection();
    deleteSelection();
}

void FOGLTextEditWidget::pasteFromClipboard() {
    if (m_readOnly) return;

    const char* clipText = glfwGetClipboardString(nullptr);
    if (!clipText) return;

    insertText(clipText);
}

// ===== 编辑操作 =====

void FOGLTextEditWidget::insertText(const std::string& text) {
    if (m_readOnly) return;

    // 删除选中内容
    if (hasSelection()) {
        deleteSelection();
    }

    // 插入新文本
    auto codepoints = utf8_to_codepoints(
        std::span<const char8_t>(
            reinterpret_cast<const char8_t*>(text.data()),
            text.size()
        )
    );

    for (uint32_t cp : codepoints) {
        m_codepoints.insert(m_codepoints.begin() + m_caret, cp);
        m_glyphs.insert(m_glyphs.begin() + m_caret, {cp, m_color});
        m_caret++;
    }

    // 确保光标后选择状态正确
    m_selStart = m_selEnd = m_caret;

    rebuildLines();
    scrollToCursor();
    setDirty();
}

void FOGLTextEditWidget::deleteSelection() {
    if (!hasSelection()) return;

    auto [start, end] = getSelection();

    m_codepoints.erase(m_codepoints.begin() + start, m_codepoints.begin() + end);
    m_glyphs.erase(m_glyphs.begin() + start, m_glyphs.begin() + end);

    m_caret = start;
    m_selStart = m_selEnd = start;  // 确保选择状态清除

    rebuildLines();
    scrollToCursor();
    setDirty();
}

void FOGLTextEditWidget::deleteCharAt(size_t pos) {
    if (pos >= m_codepoints.size()) return;

    m_codepoints.erase(m_codepoints.begin() + pos);
    m_glyphs.erase(m_glyphs.begin() + pos);

    // 确保光标不会越界
    if (m_caret > m_codepoints.size()) {
        m_caret = m_codepoints.size();
    }

    // 确保选择状态正确
    m_selStart = m_selEnd = m_caret;

    rebuildLines();
    scrollToCursor();
    setDirty();
}

// ===== 光标移动 =====

void FOGLTextEditWidget::moveCursor(int delta, bool selecting) {
    size_t newPos = m_caret;

    if (delta > 0) {
        newPos = std::min(m_caret + delta, m_codepoints.size());
    } else if (delta < 0) {
        size_t absDelta = static_cast<size_t>(-delta);
        newPos = m_caret > absDelta ? m_caret - absDelta : 0;
    }

    m_caret = newPos;

    if (selecting) {
        m_selEnd = m_caret;
    } else {
        clearSelection();
    }

    scrollToCursor();
    setDirty();
}

void FOGLTextEditWidget::moveCursorToLineStart(bool selecting) {
    size_t lineIndex = getLineForIndex(m_caret);
    if (lineIndex >= m_lines.size()) return;

    m_caret = m_lines[lineIndex].startIndex;

    if (selecting) {
        m_selEnd = m_caret;
    } else {
        clearSelection();
    }

    setDirty();
}

void FOGLTextEditWidget::moveCursorToLineEnd(bool selecting) {
    size_t lineIndex = getLineForIndex(m_caret);
    if (lineIndex >= m_lines.size()) return;

    const auto& line = m_lines[lineIndex];
    m_caret = line.startIndex + line.length;

    if (selecting) {
        m_selEnd = m_caret;
    } else {
        clearSelection();
    }

    setDirty();
}

// ===== 布局和渲染 =====

void FOGLTextEditWidget::resize(float w, float h) {
    FOGLWidget::resize(w, h);
    rebuildLines();  // 重新计算换行
    updateVisibleRange();
    setDirty();
}

void FOGLTextEditWidget::setGeometry(float x, float y, float w, float h) {
    FOGLWidget::setGeometry(x, y, w, h);
    rebuildLines();
    updateVisibleRange();
    setDirty();
}

void FOGLTextEditWidget::setGeometry(FOGLRect rect) {
    setGeometry(rect.x, rect.y, rect.width, rect.height);
}

void FOGLTextEditWidget::layoutVisibleGlyphs() {
    m_visibleGlyphs.clear();
    if (m_codepoints.empty() || m_lines.empty()) return;

    float scale = fontSize / static_cast<float>(font.getBasePixelSize());

    for (size_t lineIdx = m_visibleLineStart; lineIdx < m_visibleLineEnd && lineIdx < m_lines.size(); lineIdx++) {
        const auto& line = m_lines[lineIdx];

        float x = m_rect.x + 4.0f;
        float y = m_rect.y + line.yPosition - m_scrollY + font.getAscent() * scale + 4.0f;

        for (size_t i = 0; i < line.length; i++) {
            size_t glyphIndex = line.startIndex + i;
            if (glyphIndex >= m_codepoints.size()) break;

            uint32_t cp = m_codepoints[glyphIndex];
            if (cp == '\n') continue;

            const auto& g = font.getGlyph(cp);

            GlyphLayout layout;
            layout.cp = cp;
            layout.glyph = &g;
            layout.index = glyphIndex;
            layout.lineIndex = lineIdx;
            layout.color = m_glyphs[glyphIndex].color;

            layout.pos = {
                x + g.bearing.x * scale,
                y - g.bearing.y * scale
            };
            layout.size = {
                g.size.x * scale,
                g.size.y * scale
            };

            m_visibleGlyphs.push_back(layout);
            x += g.advance * scale;
        }
    }
}

size_t FOGLTextEditWidget::hitTest(float x, float y) {
    for (const auto& g : m_visibleGlyphs) {
        if (x >= g.pos.x && x <= g.pos.x + g.size.x &&
            y >= g.pos.y && y <= g.pos.y + g.size.y) {
            return g.index;
        }
    }

    // 如果没有命中，返回最接近的位置
    if (!m_visibleGlyphs.empty()) {
        // 检查是否在最后一个字符之后
        const auto& last = m_visibleGlyphs.back();
        if (x > last.pos.x + last.size.x) {
            return last.index + 1;
        }
    }

    return m_codepoints.size();
}

glm::vec2 FOGLTextEditWidget::getCaretPosition() {
    float scale = fontSize / static_cast<float>(font.getBasePixelSize());
    float lineHeight = font.getLineHeight() * scale;

    // 边界检查：如果没有行信息，返回默认位置
    if (m_lines.empty()) {
        float y = m_rect.y + 4.0f;
        return {m_rect.x + 4.0f, y};
    }

    // 查找光标所在行
    size_t lineIndex = getLineForIndex(m_caret);

    // 边界检查：确保行索引有效
    if (lineIndex >= m_lines.size()) {
        lineIndex = m_lines.size() - 1;
    }

    const auto& line = m_lines[lineIndex];

    // 使用行的 Y 位置
    float y = m_rect.y + line.yPosition - m_scrollY + 4.0f;

    // 计算 X 位置
    float x = m_rect.x + 4.0f;

    // 边界检查：确保不会访问越界
    size_t endIndex = std::min(m_caret, line.startIndex + line.length);
    endIndex = std::min(endIndex, m_codepoints.size());

    for (size_t i = line.startIndex; i < endIndex; i++) {
        if (i >= m_codepoints.size()) break;  // 额外的安全检查

        uint32_t cp = m_codepoints[i];
        if (cp == '\n') continue;

        const auto& glyph = font.getGlyph(cp);
        x += glyph.advance * scale;
    }

    return {x, y};
}

void FOGLTextEditWidget::onPaint(FOGLRenderContext& ctx) {
    // 先清除背景（可选，避免字符叠加） 背景透明时渲染可能异常
    // ctx.drawRect(m_rect.x, m_rect.y, m_rect.width, m_rect.height, {0, 0, 0, 0.9f});

    // 更新可见范围
    updateVisibleRange();
    layoutVisibleGlyphs();

    // 绘制选择背景
    if (hasSelection()) {
        auto [start, end] = getSelection();
        glm::vec4 selectionColor = {0.3f, 0.5f, 0.8f, 0.4f};

        for (const auto& g : m_visibleGlyphs) {
            if (g.index >= start && g.index < end) {
                ctx.drawRect(g.pos.x, g.pos.y, g.size.x, g.size.y, selectionColor);
            }
        }
    }

    // 绘制文本（使用 FOGLTextWidget 的方法，但只渲染可见字形）
    if (!m_visibleGlyphs.empty()) {
        std::unordered_map<int, std::vector<GlyphInstance>> pageInstances;

        for (const auto& layout : m_visibleGlyphs) {
            const auto& g = *layout.glyph;

            GlyphInstance inst{};
            inst.position = layout.pos;
            inst.size = layout.size;
            inst.uv0 = g.uv0;
            inst.uv1 = g.uv1;
            inst.color = layout.color;

            pageInstances[g.texturePage].push_back(inst);
        }

        for (auto& [pageIdx, instances] : pageInstances) {
            ctx.drawGlyphInstances(font.getTexturePage(pageIdx), instances);
        }
    }

    // 绘制光标
    if (hasFocus()) {
        auto now = std::chrono::steady_clock::now();
        float elapsed = std::chrono::duration<float>(now - m_blinkStartTime).count();

        bool blink = (static_cast<int>(elapsed / m_blinkInterval) % 2) == 0;

        if (blink) {
            glm::vec2 caretPos = getCaretPosition();
            float scale = fontSize / static_cast<float>(font.getBasePixelSize());
            float caretHeight = font.getLineHeight() * scale;  // 使用行高

            ctx.drawRect(caretPos.x, caretPos.y, 2.0f, caretHeight, {1, 1, 1, 1});
        }
    }
}

// ===== 事件处理 =====

bool FOGLTextEditWidget::onMouseEvent(const MouseEvent& e) {
    if (!visible()) return true;

    if (e.action == MouseAction::Press && contains(e.x, e.y)) {
        setFocus(true);
        m_selecting = true;

        m_caret = hitTest(e.x, e.y);
        m_selStart = m_selEnd = m_caret;

        setDirty();
        return false;
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

bool FOGLTextEditWidget::onScrollEvent(const MouseEvent& e) {
    if (!contains(e.x, e.y)) return true;

    float scrollSpeed = 30.0f;
    m_scrollY -= e.scrollY * scrollSpeed;
    m_scrollY = std::max(0.0f, m_scrollY);

    updateVisibleRange();
    setDirty();

    return false;
}

void FOGLTextEditWidget::onTextInput(const TextEvent& e) {
    if (m_readOnly || !hasFocus()) return;

    char utf8[5] = {0};
    uint32_t cp = e.codepoint;

    if (cp <= 0x7F) {
        utf8[0] = static_cast<char>(cp);
    } else if (cp <= 0x7FF) {
        utf8[0] = static_cast<char>(0xC0 | ((cp >> 6) & 0x1F));
        utf8[1] = static_cast<char>(0x80 | (cp & 0x3F));
    } else if (cp <= 0xFFFF) {
        utf8[0] = static_cast<char>(0xE0 | ((cp >> 12) & 0x0F));
        utf8[1] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        utf8[2] = static_cast<char>(0x80 | (cp & 0x3F));
    } else if (cp <= 0x10FFFF) {
        utf8[0] = static_cast<char>(0xF0 | ((cp >> 18) & 0x07));
        utf8[1] = static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
        utf8[2] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        utf8[3] = static_cast<char>(0x80 | (cp & 0x3F));
    }

    insertText(utf8);
}

void FOGLTextEditWidget::onKeyEvent(const KeyEvent& e) {
    if (!hasFocus()) return;

    // 支持按住和重复触发（Press 和 Repeat）
    if (!e.pressed() && !e.repeat()) return;

    bool shift = e.mods & Mod_Shift;
    bool ctrl  = e.mods & Mod_Ctrl;

    switch (e.key) {
        case GLFW_KEY_LEFT:
            moveCursor(-1, shift);
            break;

        case GLFW_KEY_RIGHT:
            moveCursor(1, shift);
            break;

        case GLFW_KEY_HOME:
            moveCursorToLineStart(shift);
            break;

        case GLFW_KEY_END:
            moveCursorToLineEnd(shift);
            break;

        case GLFW_KEY_BACKSPACE:
            if (m_readOnly) break;

            if (hasSelection()) {
                deleteSelection();
            } else if (m_caret > 0) {
                m_caret--;
                deleteCharAt(m_caret);
            }
            break;

        case GLFW_KEY_DELETE:
            if (m_readOnly) break;

            if (hasSelection()) {
                deleteSelection();
            } else if (m_caret < m_codepoints.size()) {
                deleteCharAt(m_caret);
            }
            break;

        case GLFW_KEY_C:
            if (ctrl) copySelection();
            break;

        case GLFW_KEY_X:
            if (ctrl) cutSelection();
            break;

        case GLFW_KEY_V:
            if (ctrl) pasteFromClipboard();
            break;

        case GLFW_KEY_A:
            if (ctrl) {
                m_selStart = 0;
                m_selEnd = m_codepoints.size();
                m_caret = m_selEnd;
                setDirty();
            }
            break;
    }
}

void FOGLTextEditWidget::onFocusIn() {
    m_caretVisible = true;
    m_blinkStartTime = std::chrono::steady_clock::now();
}

void FOGLTextEditWidget::onFocusOut() {
    clearSelection();
    setDirty();
}

void FOGLTextEditWidget::updateCodepoints() {
    FOGLTextWidget::updateCodepoints();
    rebuildLines();
}

size_t FOGLTextEditWidget::findNext(const std::string& text, size_t startPos) {
    // TODO: 实现查找功能
    return std::string::npos;
}

void FOGLTextEditWidget::replaceSelection(const std::string& text) {
    if (!hasSelection()) return;
    deleteSelection();
    insertText(text);
}
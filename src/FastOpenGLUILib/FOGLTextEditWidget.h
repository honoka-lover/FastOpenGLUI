//
// Created by honoka on 2026/2/3.
// Improved version with rich text, viewport rendering, copy/paste support
//

#ifndef FASTOPENGLUI_FOGLTEXTEDITWIDGET_H
#define FASTOPENGLUI_FOGLTEXTEDITWIDGET_H

#include "FOGLFunction.h"
#include "FOGLTextWidget.h"
#include <chrono>

// 富文本片段
struct TextFragment {
    std::string text;
    glm::vec4 color;
    bool bold = false;
    bool italic = false;
};

// 行信息（用于快速定位）
struct LineInfo {
    size_t startIndex;      // 在 m_codepoints 中的起始索引
    size_t length;          // 这一行的字符数
    float yPosition;        // 行的 Y 坐标
    float height;           // 行高
};

class FOGLTextEditWidget : public FOGLTextWidget {
public:
    FOGLTextEditWidget(const std::string& name = "")
        : FOGLTextWidget(name) {
        m_blinkStartTime = std::chrono::steady_clock::now();
    }

    void setText(const char *str) override;
    void setText(const wchar_t *str) override;
    void setText(std::span<const char> str) override;
    void setText(std::span<const wchar_t> str) override;
    void setText(std::span<const char8_t> str) override;
    void setText(std::span<const char16_t> str) override;
    void setText(std::span<const char32_t> str) override;

    // ===== 富文本设置 =====
    void setRichTextFromShell(const std::string& shellText);
    void setRichTextFragments(const std::vector<TextFragment>& fragments);

    // 在指定位置插入富文本
    void insertRichText(size_t position, const std::string& text, const glm::vec4& color);

    // 设置指定范围的颜色
    void setColorRange(size_t start, size_t end, const glm::vec4& color);

    // ===== 编辑控制 =====
    void setReadOnly(bool v) { m_readOnly = v; }
    bool isReadOnly() const { return m_readOnly; }

    void setWordWrap(bool wrap) { m_wordWrap = wrap; rebuildLines(); }
    bool isWordWrap() const { return m_wordWrap; }

    // ===== 窗口管理 =====
    void resize(float w, float h) override;
    void setGeometry(float x, float y, float w, float h) override;
    void setGeometry(FOGLRect rect) override;
    // ===== 光标和选择 =====
    void setCursorPosition(size_t pos);
    size_t getCursorPosition() const { return m_caret; }

    void setSelection(size_t start, size_t end);
    void clearSelection();
    bool hasSelection() const { return m_selStart != m_selEnd; }

    std::pair<size_t, size_t> getSelection() const {
        return {(std::min)(m_selStart, m_selEnd), (std::max)(m_selStart, m_selEnd)};
    }

    // ===== 视口控制 =====
    void scrollToLine(size_t lineIndex);
    void scrollToCursor();
    void setScrollOffset(float offset) { m_scrollY = offset; }
    float getScrollOffset() const { return m_scrollY; }

    size_t getVisibleLineStart() const { return m_visibleLineStart; }
    size_t getVisibleLineEnd() const { return m_visibleLineEnd; }

    // ===== 复制粘贴 =====
    std::string getSelectedText() const;
    void copySelection();
    void cutSelection();
    void pasteFromClipboard();

    // ===== 查找替换 =====
    size_t findNext(const std::string& text, size_t startPos = 0);
    void replaceSelection(const std::string& text);

    // ===== 事件处理 =====
    bool onMouseEvent(const MouseEvent& e) override;
    void onTextInput(const TextEvent& e) override;
    void onKeyEvent(const KeyEvent& e) override;
    bool onScrollEvent(const MouseEvent& e) override;

    void onFocusIn() override;
    void onFocusOut() override;

    void onPaint(FOGLRenderContext& ctx) override;

protected:
    // ===== 编辑状态 =====
    size_t m_caret = 0;           // 光标位置（UTF-32 索引）
    size_t m_selStart = 0;        // 选择起始
    size_t m_selEnd = 0;          // 选择结束

    bool m_selecting = false;     // 是否正在拖拽选择
    bool m_readOnly = false;      // 只读模式
    bool m_wordWrap = true;       // 自动换行

    // ===== 光标闪烁 =====
    std::chrono::steady_clock::time_point m_blinkStartTime;
    float m_blinkInterval = 0.5f; // 闪烁间隔（秒）
    bool m_caretVisible = true;

    // ===== 视口管理 =====
    float m_scrollY = 0.0f;       // 垂直滚动偏移
    size_t m_visibleLineStart = 0;
    size_t m_visibleLineEnd = 0;

    std::vector<LineInfo> m_lines; // 行信息缓存

    // ===== 布局缓存 =====
    struct GlyphLayout {
        uint32_t cp;
        glm::vec2 pos;            // 屏幕坐标
        glm::vec2 size;
        glm::vec4 color;
        const FOGLGlyph* glyph;
        size_t index;             // 在 m_codepoints 中的位置
        size_t lineIndex;         // 所在行
    };

    std::vector<GlyphLayout> m_visibleGlyphs; // 仅可见区域的字形

    // ===== 内部方法 =====
    void rebuildLines();          // 重建行信息
    void updateVisibleRange();    // 更新可见范围
    void layoutVisibleGlyphs();   // 布局可见字形

    size_t hitTest(float x, float y); // 坐标 -> 字符索引
    glm::vec2 getCaretPosition();     // 获取光标屏幕坐标

    void insertText(const std::string& text);
    void deleteSelection();
    void deleteCharAt(size_t pos);

    void moveCursor(int delta, bool selecting = false);
    void moveCursorToLineStart(bool selecting = false);
    void moveCursorToLineEnd(bool selecting = false);

    void updateCodepoints() override;

    // 辅助：字符索引 -> 行号
    size_t getLineForIndex(size_t index) const;
};

#endif //FASTOPENGLUI_FOGLTEXTEDITWIDGET_H
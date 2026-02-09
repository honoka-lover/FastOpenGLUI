# FOGLTextEditWidget 改进版使用指南

## 功能概述

这个改进版的 `FOGLTextEditWidget` 提供了以下核心功能:

1. **富文本支持** - 不同颜色、样式的文本片段
2. **视口渲染** - 只渲染可见区域，支持大文本文件
3. **完整的编辑功能** - 复制、粘贴、剪切、选择
4. **光标管理** - 闪烁光标、光标定位
5. **滚动支持** - 鼠标滚轮滚动、自动滚动到光标

## 基本使用

### 1. 创建编辑器

```cpp
auto editor = std::make_shared<FOGLTextEditWidget>("MyEditor");
editor->setGeometry(10, 10, 600, 400);
editor->setFontSize(14.0f);
```

### 2. 设置普通文本

```cpp
editor->setText("Hello, World!\nThis is a text editor.");
```

### 3. 设置富文本（Shell 风格）

```cpp
std::string shellText = 
    "\x1b[31mError:\x1b[0m Something went wrong\n"
    "\x1b[32mSuccess:\x1b[0m Operation completed\n"
    "\x1b[33mWarning:\x1b[0m Be careful";

editor->setRichTextFromShell(shellText);
```

### 4. 使用文本片段设置富文本

```cpp
std::vector<TextFragment> fragments = {
    {"Hello ", {1.0f, 1.0f, 1.0f, 1.0f}},        // 白色
    {"World", {1.0f, 0.0f, 0.0f, 1.0f}},         // 红色
    {"!\n", {1.0f, 1.0f, 1.0f, 1.0f}},           // 白色
    {"Welcome ", {0.0f, 1.0f, 0.0f, 1.0f}},      // 绿色
    {"to ", {1.0f, 1.0f, 0.0f, 1.0f}},           // 黄色
    {"the editor", {0.0f, 0.5f, 1.0f, 1.0f}}     // 蓝色
};

editor->setRichTextFragments(fragments);
```

## 高级功能

### 1. 插入富文本到指定位置

```cpp
// 在位置 10 插入红色文本
editor->insertRichText(10, "ERROR: ", {1.0f, 0.0f, 0.0f, 1.0f});
```

### 2. 设置范围颜色

```cpp
// 将字符 5-15 设置为蓝色
editor->setColorRange(5, 15, {0.0f, 0.5f, 1.0f, 1.0f});
```

### 3. 光标操作

```cpp
// 设置光标位置
editor->setCursorPosition(100);

// 获取当前光标位置
size_t pos = editor->getCursorPosition();

// 滚动到光标位置
editor->scrollToCursor();
```

### 4. 选择操作

```cpp
// 设置选择范围
editor->setSelection(10, 50);

// 获取选择范围
auto [start, end] = editor->getSelection();

// 检查是否有选择
if (editor->hasSelection()) {
    std::string selected = editor->getSelectedText();
}

// 清除选择
editor->clearSelection();
```

### 5. 编辑操作

```cpp
// 复制选中内容到剪贴板
editor->copySelection();

// 剪切选中内容
editor->cutSelection();

// 从剪贴板粘贴
editor->pasteFromClipboard();
```

### 6. 滚动控制

```cpp
// 滚动到特定行
editor->scrollToLine(100);

// 设置滚动偏移
editor->setScrollOffset(500.0f);

// 获取滚动偏移
float offset = editor->getScrollOffset();

// 获取可见行范围
size_t startLine = editor->getVisibleLineStart();
size_t endLine = editor->getVisibleLineEnd();
```

### 7. 配置选项

```cpp
// 设置只读模式
editor->setReadOnly(true);

// 启用/禁用自动换行
editor->setWordWrap(true);

// 设置字体大小
editor->setFontSize(16.0f);
```

## 事件处理

编辑器自动处理以下事件:

- **鼠标点击** - 设置光标位置
- **鼠标拖拽** - 选择文本
- **键盘输入** - 插入文本
- **方向键** - 移动光标
- **Home/End** - 行首/行尾
- **Backspace/Delete** - 删除字符
- **Ctrl+C/X/V** - 复制/剪切/粘贴
- **Ctrl+A** - 全选
- **鼠标滚轮** - 滚动内容

## 性能优化

### 仅渲染可见区域

编辑器使用视口裁剪，只渲染屏幕上可见的文本:

```cpp
void FOGLTextEditWidget::updateVisibleRange() {
    // 计算可见行范围
    float viewTop = m_scrollY;
    float viewBottom = m_scrollY + m_rect.height;
    
    // 只布局可见行的字形
    for (size_t lineIdx = m_visibleLineStart; 
         lineIdx < m_visibleLineEnd; 
         lineIdx++) {
        // 布局这一行...
    }
}
```

### 行信息缓存

编辑器缓存每一行的位置和尺寸信息:

```cpp
struct LineInfo {
    size_t startIndex;      // 行起始字符索引
    size_t length;          // 行长度
    float yPosition;        // Y 坐标
    float height;           // 行高
};
```

## 实际应用示例

### 1. 日志查看器

```cpp
class LogViewer {
public:
    LogViewer() {
        editor = std::make_shared<FOGLTextEditWidget>("LogViewer");
        editor->setReadOnly(true);  // 只读模式
        editor->setWordWrap(false); // 不换行
    }
    
    void addLog(const std::string& level, const std::string& message) {
        glm::vec4 color;
        if (level == "ERROR") color = {1.0f, 0.0f, 0.0f, 1.0f};
        else if (level == "WARNING") color = {1.0f, 1.0f, 0.0f, 1.0f};
        else if (level == "INFO") color = {0.0f, 1.0f, 0.0f, 1.0f};
        else color = {1.0f, 1.0f, 1.0f, 1.0f};
        
        size_t pos = editor->getCursorPosition();
        editor->insertRichText(pos, level + ": " + message + "\n", color);
        
        // 自动滚动到最新日志
        editor->setCursorPosition(editor->getText(Encoding::UTF8).size());
        editor->scrollToCursor();
    }
    
private:
    std::shared_ptr<FOGLTextEditWidget> editor;
};
```

### 2. 代码编辑器

```cpp
class CodeEditor {
public:
    CodeEditor() {
        editor = std::make_shared<FOGLTextEditWidget>("CodeEditor");
        editor->setFontSize(14.0f);
    }
    
    void highlightSyntax() {
        std::string code = editor->getText(Encoding::UTF8);
        
        // 简单的关键字高亮
        std::vector<std::string> keywords = {
            "if", "else", "while", "for", "return"
        };
        
        for (const auto& keyword : keywords) {
            size_t pos = 0;
            while ((pos = code.find(keyword, pos)) != std::string::npos) {
                editor->setColorRange(pos, pos + keyword.length(), 
                                     {1.0f, 0.5f, 0.0f, 1.0f}); // 橙色
                pos += keyword.length();
            }
        }
    }
    
private:
    std::shared_ptr<FOGLTextEditWidget> editor;
};
```

### 3. Shell 输出显示器

```cpp
class ShellOutput {
public:
    ShellOutput() {
        editor = std::make_shared<FOGLTextEditWidget>("Shell");
        editor->setReadOnly(true);
    }
    
    void appendOutput(const std::string& shellText) {
        // 自动解析 ANSI 颜色代码
        editor->setRichTextFromShell(shellText);
        
        // 滚动到底部
        size_t lastLine = editor->getVisibleLineEnd();
        editor->scrollToLine(lastLine);
    }
    
private:
    std::shared_ptr<FOGLTextEditWidget> editor;
};
```

## 架构说明

### 数据结构

1. **m_codepoints** - UTF-32 编码的字符数组
2. **m_glyphs** - 每个字符的样式信息（颜色等）
3. **m_lines** - 行信息缓存，用于快速定位
4. **m_visibleGlyphs** - 仅包含可见区域的字形布局

### 渲染流程

```
updateVisibleRange()        // 计算可见行范围
    ↓
layoutVisibleGlyphs()       // 布局可见字形
    ↓
onPaint()                   // 渲染
    ├─ 绘制选择背景
    ├─ 绘制文本（分页）
    └─ 绘制光标
```

### 编辑流程

```
用户输入/按键
    ↓
insertText() / deleteChar()
    ↓
updateCodepoints()          // 更新内部数据
    ↓
rebuildLines()              // 重建行信息
    ↓
scrollToCursor()            // 确保光标可见
    ↓
setDirty()                  // 标记需要重绘
```

## 注意事项

1. **字体加载**: 确保在使用前调用 `FOGLTextWidget::setFont()`
2. **性能**: 对于超大文件（>100MB），考虑使用分块加载
3. **内存**: 富文本会为每个字符存储样式信息
4. **线程安全**: 不是线程安全的，确保在主线程调用

## 扩展建议

1. **语法高亮** - 添加语言解析器
2. **行号显示** - 在左侧添加行号栏
3. **多光标支持** - 支持多个编辑位置
4. **撤销/重做** - 添加命令历史
5. **查找替换** - 实现 findNext() 和 replaceSelection()
6. **代码折叠** - 支持代码块折叠
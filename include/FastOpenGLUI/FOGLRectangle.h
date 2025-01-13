#ifndef GLFW_ROUND_BUTTON_H
#define GLFW_ROUND_BUTTON_H

#include "shader.h"
#include <utility>
#include "GLFW/glfw3.h"
#include "CommonFunc.h"

class FOGLWindow;
class FOGLRectangle
{
public:
    // 禁止拷贝构造和赋值
    FOGLRectangle(const FOGLRectangle&) = delete;
    FOGLRectangle& operator=(const FOGLRectangle&) = delete;
    FOGLRectangle() = delete;

    // 提供一个静态方法，通过 new 创建对象
    static FOGLRectangle* createFOGLRectangle(float x, float y, float width, float height, float radius, glm::vec4 color,FOGLWindow* window);

    ~FOGLRectangle();
    
    Shader shader;
    // 绘制圆角矩形按钮
    virtual void draw(const glm::mat4 &projection);

    virtual void draw();

    virtual void setColor(const glm::vec4 &color);

    virtual void setBackgroundSource(int id);

    virtual void setHoverBackgroundSource(int id);

    float x, y, width, height, radius, windowWidth, windowHeight;
//    void move(float x,float y);
    bool isHovered = false;

    virtual void setVisible(bool show);

    //是否要阻止事件传递
    virtual bool processMouseEvent();

    virtual void setEventClickFunc(std::function<bool()> &&event){
        clickEvent = std::move(event);
    }

    virtual void move(float x,float y);

    virtual void resize(float newWidth, float newHeight);

protected:
    FOGLWindow *mainWindow = nullptr;

    glm::vec4 color{};
    unsigned int VAO{}, VBO{} ,EBO{};

    //是否使用背景图
    bool useTexture = false;

    //鼠标聚焦是是否有图
    bool useHoverFlag = false;

    //是否显示
    bool visibleFlag = true;

    GLuint normalTexture, hoverTexture;

    //记录是否按下瞬间
    bool clickFlag = false;

    std::function<bool()> clickEvent;


    // 构造函数，设置按钮的初始位置、大小和圆角半径
    FOGLRectangle(float x, float y, float width, float height, float radius, glm::vec4 color,FOGLWindow* window);

    bool insideFOGLRectangle(GLFWwindow *window) const
    {
        // 获取鼠标位置并检测 hover 和点击
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        if (mouseX >= x && mouseX <= x + width && mouseY >= y && mouseY <= y + height)
        {
            return true;
        }

        return false;
    }
};

#endif
#ifndef GLFW_ROUND_BUTTON_H
#define GLFW_ROUND_BUTTON_H

#include "GLFW/glfw3.h"
#include "shader.h"
#include "CommonFunc.h"
class MainWindow;
class FOGLRectangle
{
public:
    // 构造函数，设置按钮的初始位置、大小和圆角半径
    FOGLRectangle(float x, float y, float width, float height, float radius, glm::vec4 color, MainWindow *window);

    ~FOGLRectangle();
    Shader shader;
    // 绘制圆角矩形按钮
    void draw(const glm::mat4 &projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f));

    void setColor(const glm::vec4 &color);

    void setBackgroundSource(int id);

    void setHoverBackgroundSource(int id);

    float x, y, width, height, radius, windowWidth, windowHeight;
//    void move(float x,float y);
    bool isHovered = false;

private:

    glm::vec4 color;
    unsigned int VAO{}, VBO{} ,EBO{};

    bool useTexture;

    GLuint normalTexture, hoverTexture;
};

#endif
#include "glad/glad.h"
#include "FOGLRectangle.h"
#include <stb_image.h>
#include "FOGLWindow.h"
// 顶点着色器源码
static const char *vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoords;
out vec2 TextCoords;
// 片段着色器源码
uniform vec2 screenSize; // 屏幕尺寸
uniform mat4 projection;
void main() {
    // 将屏幕坐标转换为 NDC 坐标
    vec2 ndcPos = vec2(2.0 * aPos.x / screenSize.x - 1.0, 1.0 - 2.0 * aPos.y / screenSize.y);
    // 将 NDC 坐标转换为 [0.0, 1.0] 范围的纹理坐标
    TexCoords = aPos;
    TextCoords = aTexCoord;
    gl_Position = projection*vec4(ndcPos, 0.0, 1.0);
}
)";

// 片段着色器源码
static const char *fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
in vec2 TextCoords;
uniform vec2 resolution;  // 按钮分辨率
uniform vec4 buttonColor; // 按钮颜色
uniform float radius;     // 圆角半径
uniform vec2 screenSize; // 屏幕尺寸
uniform vec2 pos;
uniform sampler2D backgroundTexture;
uniform bool useBackTexture;
float minDistanceToCorner(vec2 point, vec2 A, vec2 B, vec2 C, vec2 D) {
    // 计算点到每个角的距离
    float distA = length(point - A);
    float distB = length(point - B);
    float distC = length(point - C);
    float distD = length(point - D);
    
    // 返回最小的距离
    return min(min(distA, distB), min(distC, distD));
}

void main() {
    // 将纹理坐标从 [0, 1] 映射到按钮的局部坐标系
    vec2 localCoord = TexCoords;

    vec2 pos1 = vec2(pos.x + radius , pos.y + radius);
    vec2 pos2 = vec2(pos.x + radius , pos.y + resolution.y - radius);
    vec2 pos3 = vec2(pos.x + resolution.x - radius , pos.y + radius);
    vec2 pos4 = vec2(pos.x + resolution.x - radius , pos.y + resolution.y - radius);

    if((localCoord.x > pos1.x  && localCoord.x < pos4.x && localCoord.y >= pos.y && localCoord.y <= pos.y+ resolution.y)
    ||( localCoord.y > pos1.y && localCoord.y < pos2.y && localCoord.x >= pos.x && localCoord.x <= pos.x + resolution.x)){
        if(useBackTexture){
            vec4 texColor = texture(backgroundTexture, TextCoords);
            if (texColor.a < 0.1) {
                discard; // 丢弃透明区域
            }
            FragColor = texColor;
        }else{
            FragColor = buttonColor;
        }
        return;
    }

    //寻找最近的点
    float point = minDistanceToCorner(localCoord, pos1, pos2,pos3,pos4);

    if(point > radius)
        discard;

    if(useBackTexture){
        vec4 texColor = texture(backgroundTexture, TextCoords);
        if (texColor.a < 0.1) {
            discard; // 丢弃透明区域
        }
        FragColor = texColor;
    }else{
        FragColor = buttonColor;
    }
}
)";


FOGLRectangle::FOGLRectangle(float x, float y, float width, float height, float radius, glm::vec4 color, FOGLWindow*window)
    : x(x)
    , y(y)
    , width(width)
    , height(height)
    , radius(radius)
    , color(color)
    , mainWindow(window)
{
    if(window){
        windowWidth = float(window->getWidth());
        windowHeight = float(window->getHeight());
    }else{
        throw std::invalid_argument("parentWindow* can not be null");
    }

    // 编译着色器
    this->shader.Compile(vertexShaderSource, fragmentShaderSource);
    shader.Use();
    // 初始化顶点数据
    // float vertices[] = {
    //     x / windowWidth * 2 - 1, 1.0f - y / windowHeight * 2 - height / windowHeight * 2,
    //     x / windowWidth * 2 - 1 + width / windowWidth * 2, 1.0f - y / windowHeight * 2 - height / windowHeight * 2,
    //     x / windowWidth * 2 - 1 + width / windowWidth * 2, 1.0f - y / windowHeight * 2 - height / windowHeight * 2 + height / windowHeight * 2,
    //     x / windowWidth * 2 - 1, 1.0f - y / windowHeight * 2 - height / windowHeight * 2 + height / windowHeight * 2};
    float vertices[] = {
        x,y, 0.0f,0.0f,
        x + width,y,1.0f,0.0f,
        x,y + height,0.0f,1.0f,
        x + width,y + height,1.0f,1.0f
    };
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0};
    // 生成 VAO 和 VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // 纹理坐标属性
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    shader.SetInteger("useBackTexture",0);

    glUseProgram(0);
}

void FOGLRectangle::draw(const glm::mat4 &projection)
{
    if(visibleFlag){
        glEnable(GL_BLEND);
        // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

        // activate corresponding render state
        shader.Use();

        // 设置 uniform
        shader.SetVector2f("screenSize", windowWidth, windowHeight);
        shader.SetMatrix4("projection", projection);
        shader.SetVector2f("resolution", width, height);
        shader.SetVector4f("buttonColor", color);
        shader.SetFloat("radius", radius);
        shader.SetVector2f("pos", x, y);

        if(useTexture){
            if(useHoverFlag){
                glBindTexture(GL_TEXTURE_2D, isHovered ? hoverTexture : normalTexture);
            }else{
                glBindTexture(GL_TEXTURE_2D,normalTexture);
            }
        }


        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }
}

void FOGLRectangle::setColor(const glm::vec4 &color) {
    this->color = color;
}

void FOGLRectangle::setBackgroundSource(int id) {
    shader.Use();
    shader.SetInteger("useBackTexture",1);
    useTexture = true;
    loadTextureFromResource(id,normalTexture);
    loadTextureFromResource(id,hoverTexture);
}

FOGLRectangle::~FOGLRectangle() {
    if(useTexture){
        glDeleteTextures(1,&normalTexture);
    }
}

void FOGLRectangle::setHoverBackgroundSource(int id) {
    shader.Use();
    if(useTexture == false)
        throw std::runtime_error("缺失背景图");
    loadTextureFromResource(id,hoverTexture);
    useHoverFlag = true;
}

void FOGLRectangle::setVisible(bool show) {
    visibleFlag = show;
}

FOGLRectangle *
FOGLRectangle::createFOGLRectangle(float x, float y, float width, float height, float radius, glm::vec4 color,FOGLWindow *window) {

    return new FOGLRectangle(x,y,width,height,radius,color, window);
}

bool FOGLRectangle::processMouseEvent() {
    // 获取鼠标位置并检测 hover 和点击
    double mouseX, mouseY;
    glfwGetCursorPos(mainWindow->getGLFWwindowPointer(), &mouseX, &mouseY);
    isHovered = false;

    if(!visibleFlag)
        return false;


    if (glfwGetMouseButton(mainWindow->getGLFWwindowPointer(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS
    && insideFOGLRectangle(mainWindow->getGLFWwindowPointer())) {
        if (!clickFlag) {
            clickFlag = true;
            if (static_cast<bool>(clickEvent) && clickEvent())
                return true;
        }
    }else if(insideFOGLRectangle(mainWindow->getGLFWwindowPointer())){
        isHovered = true;
        clickFlag = false;
    }else if(glfwGetMouseButton(mainWindow->getGLFWwindowPointer(), GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS ){
        clickFlag = false;
    }

    return false;
}

//void FOGLRectangle::move(float x, float y) {
//    shader.Use();
//    shader.SetVector2f("pos",x,y);
//}

//
// Created by m1393 on 2025/1/13.
//

#ifndef FASTOPENGLUI_FOGLPROGRESSBAR_H
#define FASTOPENGLUI_FOGLPROGRESSBAR_H

#include "FOGLRectangle.h"

class FOGLProgressBar:public FOGLRectangle{

public:
    // 禁止拷贝构造和赋值
    FOGLProgressBar(const FOGLProgressBar&) = delete;
    FOGLProgressBar& operator=(const FOGLProgressBar&) = delete;
    FOGLProgressBar() = delete;

    // 提供一个静态方法，通过 new 创建对象
    static FOGLProgressBar* createFOGLProgressBar(float x, float y, float width, float height, float radius, glm::vec4 color,FOGLWindow* window);

    ~FOGLProgressBar();

    Shader shader;
    // 绘制圆角矩形按钮
    void draw(const glm::mat4 &projection) override;

    void draw()override;

    void setInnerProgressColor(glm::vec4 color);

    void setVisible(bool show) override;

    void setValue(float v);

    void move(float x,float y) override;

    void resize(float newWidth, float newHeight) override;
private:
    //存放进度条百分比
    float value = 0.0f;

    FOGLRectangle* innerProgress;

    FOGLProgressBar(float x, float y, float width, float height, float radius, glm::vec4 color, FOGLWindow *window);

    void setEventClickFunc(std::function<bool()> &&event) override{}

    void setHoverBackgroundSource(int id) override{}

    //是否要阻止事件传递
    bool processMouseEvent()override { return false;}
};


#endif //FASTOPENGLUI_FOGLPROGRESSBAR_H

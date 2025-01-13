//
// Created by m1393 on 2025/1/13.
//

#include "FOGLProgressBar.h"

FOGLProgressBar::FOGLProgressBar(float x, float y, float width, float height, float radius, glm::vec4 color,FOGLWindow *window): FOGLRectangle(x,y,width,height,radius,color,window)
{
    innerProgress = createFOGLRectangle(x,y,width,height,radius,color,window);
}

FOGLProgressBar::~FOGLProgressBar() {

}

void FOGLProgressBar::draw(const glm::mat4 &projection) {
    FOGLRectangle::draw(projection);
    innerProgress->draw(projection);
}

void FOGLProgressBar::move(float x, float y) {
    innerProgress->move(x,y);
    FOGLRectangle::move(x, y);
}

void FOGLProgressBar::resize(float newWidth, float newHeight) {
    innerProgress->resize(newWidth*value,newHeight);
    FOGLRectangle::resize(newWidth, newHeight);
}

FOGLProgressBar *
FOGLProgressBar::createFOGLProgressBar(float x, float y, float width, float height, float radius, glm::vec4 color,FOGLWindow *window) {
    return new FOGLProgressBar(x,y,width,height,radius,color, window);
}

void FOGLProgressBar::setValue(float v) {
    if(v>1.001f)
        throw std::invalid_argument("FOGLProgressBar: Value must be less than 1");
    value = v;
    innerProgress->resize(width*value,height);
}

void FOGLProgressBar::setVisible(bool show) {
    innerProgress->setVisible(show);
    FOGLRectangle::setVisible(show);
}

void FOGLProgressBar::setInnerProgressColor(glm::vec4 color) {
    innerProgress->setColor(color);
}

void FOGLProgressBar::draw() {
    draw(glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f));
}

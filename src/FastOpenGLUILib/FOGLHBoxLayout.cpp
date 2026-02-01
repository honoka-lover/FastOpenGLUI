//
// Created by honoka on 2026/1/31.
//

#include "FOGLHBoxLayout.h"

void FOGLHBoxLayout::setSpacing(float spacing) {
    this->m_spacing = spacing;
}

void FOGLHBoxLayout::apply(FOGLWidget *parent) {
    {
        std::vector<int> indexVector;
        for (int i=0;i<parent->children().size();i++) {
            if (parent->children().at(i)->visible())
                indexVector.push_back(i);
        }
        for (int i=0;i<indexVector.size();i++) {
            FOGLRect parentRect = parent->geometry();
            FOGLRect geo = parent->children().at(indexVector.at(i))->geometry();
            geo.width = (parentRect.width - (indexVector.size() -1) * m_spacing) / indexVector.size();
            geo.x = i*m_spacing + geo.width*i;
            geo.y = 0;
            geo.height = parentRect.height;
            parent->children().at(indexVector.at(i))->setGeometry(geo);
        }
    }
}

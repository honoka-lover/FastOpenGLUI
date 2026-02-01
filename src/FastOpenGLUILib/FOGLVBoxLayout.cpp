//
// Created by honoka on 2026/1/31.
//

#include "FOGLVBoxLayout.h"

void FOGLVBoxLayout::setSpacing(float spacing) {
    this->m_spacing = spacing;
}

void FOGLVBoxLayout::apply(FOGLWidget *parent){
    if (!parent)
        return;
    std::vector<int> indexVector;
    for (int i=0;i<parent->children().size();i++) {
        if (parent->children().at(i)->visible())
            indexVector.push_back(i);
    }
    for (int i=0;i<indexVector.size();i++) {
        FOGLRect parentRect = parent->geometry();
        FOGLRect geo = parent->children().at(indexVector.at(i))->geometry();
        geo.height = (parentRect.height - (indexVector.size() -1) * m_spacing) / indexVector.size();
        geo.x =0;
        geo.width = parentRect.width;
        geo.y = i*m_spacing + geo.height*i;
        parent->children().at(indexVector.at(i))->setGeometry(geo);
    }
}

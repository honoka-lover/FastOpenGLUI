/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "text_renderer.h"
// #include "resource_manager.h"

// 顶点着色器源码
static const char *vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
} 
)";

// 片段着色器源码
static const char *fragmentShaderSource = R"(
#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec4 textColor;

void main()
{    
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    color = textColor * sampled;
}  
)";

TextRenderer::TextRenderer(unsigned int width, unsigned int height)
{
    // load and configure shader
    // this->TextShader = ResourceManager::LoadShader("text_2d.vs", "text_2d.fs", nullptr, "text");
    this->TextShader.Compile(vertexShaderSource, fragmentShaderSource);
    this->TextShader.SetMatrix4("projection", glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f), true);
    this->TextShader.SetInteger("text", 0);
    // configure VAO/VBO for texture quads
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);
    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

TextRenderer::~TextRenderer()
{
    glDeleteBuffers(1, &this->VBO);
    glDeleteBuffers(1, &this->VAO);

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void TextRenderer::LoadFont(std::string font)
{
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return;
    }

    std::string font_name = font;
    if (FT_New_Face(ft, font_name.c_str(), 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return;
    }

    // disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    freeLibLoad = true;
}

void TextRenderer::RenderText(std::wstring text, float x, float y, float scale, unsigned fontsize, glm::vec4 color)
{
    if (!freeLibLoad)
    {
        std::cout << "FreeType库初始化失败";
        return;
    }
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 激活对应的渲染状态
    this->TextShader.Use();
    this->TextShader.SetVector4f("textColor", color);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(this->VAO);

    FT_Set_Char_Size(face, 0, fontsize * 64, 96, 96);
    // 重新计算基线偏移量
    float baselineOffset = (face->size->metrics.ascender - face->size->metrics.descender) / 64.0f + 0.0001f;;

    // 遍历每个字符
    for (auto c = text.begin(); c != text.end(); ++c)
    {
        const CharacterID id = CharacterID{static_cast<unsigned>(*c), fontsize};
        CharacterData ch;

        // 如果字符不存在于缓存中，加载并缓存
        if (!characters.contains(id))
        {
            if (FT_Load_Char(face, *c, FT_LOAD_RENDER))
            {
                std::cout << "Failed to load Glyph: " << static_cast<unsigned>(*c) << std::endl;
                continue;
            }
            addCharacter(*c, fontsize);
        }

        ch = characters[id];

        // 修正字符位置
        float xpos = x + ch.Bearing.x * scale;
        float ypos = y + (baselineOffset - ch.Bearing.y * scale);
//        float ypos = y+ float(ch.Size.y - ch.Bearing.y)* scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        // 更新每个字符的顶点数据
        float vertices[6][4] = {
                {xpos, ypos + h, 0.0f, 1.0f},
                {xpos + w, ypos, 1.0f, 0.0f},
                {xpos, ypos, 0.0f, 0.0f},

                {xpos, ypos + h, 0.0f, 1.0f},
                {xpos + w, ypos + h, 1.0f, 1.0f},
                {xpos + w, ypos, 1.0f, 0.0f}};

        // 绑定纹理并更新顶点缓冲区
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // 渲染三角形
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // 移动光标到下一个字符
        x += (ch.Advance >> 6) * scale; // 位移到下一个字符
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}



void TextRenderer::addCharacter(unsigned ch, unsigned fontsize)
{
    FT_Set_Char_Size(face, 0, fontsize * 64, 96, 96);

    if (FT_Load_Char(face, ch, FT_LOAD_RENDER))
    {
        std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
        return;
    }
    // generate texture
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RED,
        face->glyph->bitmap.width,
        face->glyph->bitmap.rows,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        face->glyph->bitmap.buffer);
    // set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // now store character for later use
    CharacterData character = {
        texture,
        glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
        glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
        static_cast<unsigned int>(face->glyph->advance.x)};
    CharacterID id{ch, fontsize};
    characters[id] = character;
}

void
TextRenderer::RenderText(std::wstring text, float x, float y, float scale, unsigned int fontsize, glm::vec3 color) {
    RenderText(std::move(text),x,y,scale,fontsize,glm::vec4(color,1.0));
}

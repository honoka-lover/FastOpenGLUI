/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <map>

#include "glad/glad.h"
#include "glm/glm.hpp"

#include "shader.h"
#include "freetype/freetype.h"
/// Holds all state information relevant to a character as loaded using FreeType
struct CharacterID
{
    unsigned ch;
    unsigned fontsize;
    bool operator==(const CharacterID id) const
    {
        if (ch == id.ch && fontsize == id.fontsize)
        {
            return true;
        }
        return false;
    }

    bool operator<(const CharacterID id) const
    {
        if (ch < id.ch)
        {
            return true;
        }
        else if (ch == id.ch)
        {
            if (fontsize < id.fontsize)
                return true;
        }
        return false;
    }

    bool operator>(const CharacterID id) const
    {
        if (ch > id.ch)
        {
            return true;
        }
        else if (ch == id.ch)
        {
            if (fontsize > id.fontsize)
                return true;
        }
        return false;
    }
};
struct CharacterData
{
    unsigned int TextureID; // 字符纹理的ID
    glm::ivec2 Size;        // 字符的尺寸
    glm::ivec2 Bearing;     // 字符的边界
    unsigned int Advance;   // 字符的水平位移
};

// A renderer class for rendering text displayed by a font loaded using the
// FreeType library. A single font is loaded, processed into a list of Character
// items for later rendering.
class TextRenderer
{
public:
    // holds a list of pre-compiled Characters
    std::map<CharacterID, CharacterData> characters;
    // shader used for text rendering
    Shader TextShader;
    // constructor
    TextRenderer(unsigned int width, unsigned int height);

    ~TextRenderer();
    // pre-compiles a list of characters from the given font
    void LoadFont(std::string font);
    // renders a string of text using the precompiled list of characters
    void RenderText(std::wstring text, float x, float y, float scale, unsigned fontsize, glm::vec4 color = glm::vec4(1.0f));

    void RenderText(std::wstring text, float x, float y, float scale, unsigned fontsize, glm::vec3 color = glm::vec3(1.0f));
private:
    // render state
    unsigned int VAO, VBO;
    FT_Library ft;
    FT_Face face;
    bool freeLibLoad = false;

    void addCharacter(unsigned ch, unsigned fontsize);
};

#endif
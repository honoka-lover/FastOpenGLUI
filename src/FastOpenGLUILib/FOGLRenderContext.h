//
// Created by honoka on 2026/1/31.
//

#ifndef FASTOPENGLUI_FOGLRENDERCONTEXT_H
#define FASTOPENGLUI_FOGLRENDERCONTEXT_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

// 前置声明资源管理器
class FOGLResourceManager;
// 简单渲染上下文，用于 UI 绘制
class FOGLRenderContext {
public:
    FOGLRenderContext(FOGLResourceManager & resMgr);

    ~FOGLRenderContext();

    // 必须在有效 OpenGL 上下文后调用
    void init(int width, int height);

    // 绘制纯色矩形
    void drawRect(float x, float y, float w, float h, const glm::vec4& color);

    // 绘制纹理（使用资源管理器获取）
    void drawTexture(const std::string& path, float x, float y, float w, float h);

    void drawTexture(int rcID, float x, float y, float w, float h);

private:
    int m_width = 0, m_height = 0;
    GLuint m_colorProgram = 0, m_vao = 0, m_vbo = 0;
    GLuint m_textureProgram = 0, m_vaoTex = 0, m_vboTex = 0;

    bool initStatus = false;

    FOGLResourceManager &m_resourceManager;

    std::unordered_map<std::string, GLuint> m_textureCacheStr;
    std::unordered_map<int, GLuint> m_textureCacheRC;

    // 内部绘制纹理（已有纹理ID）
    void drawTexture(GLuint tex, float x, float y, float w, float h);

    // 获取纹理
    GLuint getTexture(const std::string& path);

    GLuint getTexture(int rcID);

    void setupColorProgram();

    void setupTextureProgram();
};


#endif //FASTOPENGLUI_FOGLRENDERCONTEXT_H
//
// Created by honoka on 2026/1/31.
//

#include "FOGLRenderContext.h"

#include "FOGLResourceManager.h"

// 简单 Shader 工具
static GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info[512];
        glGetShaderInfoLog(shader, 512, nullptr, info);
        std::cerr << "Shader compile error: " << info << std::endl;
    }
    return shader;
}

static GLuint createProgram(const char* vs, const char* fs) {
    GLuint program = glCreateProgram();
    GLuint v = compileShader(GL_VERTEX_SHADER, vs);
    GLuint f = compileShader(GL_FRAGMENT_SHADER, fs);
    glAttachShader(program, v);
    glAttachShader(program, f);
    glLinkProgram(program);
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char info[512];
        glGetProgramInfoLog(program, 512, nullptr, info);
        std::cerr << "Program link error: " << info << std::endl;
    }
    glDeleteShader(v);
    glDeleteShader(f);
    return program;
}

FOGLRenderContext::FOGLRenderContext(FOGLResourceManager & resMgr): m_resourceManager(resMgr)  {

}

FOGLRenderContext::~FOGLRenderContext() {
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_colorProgram) glDeleteProgram(m_colorProgram);

    if (m_vboTex) glDeleteBuffers(1, &m_vboTex);
    if (m_vaoTex) glDeleteVertexArrays(1, &m_vaoTex);
    if (m_textureProgram) glDeleteProgram(m_textureProgram);
}

void FOGLRenderContext::init(int width, int height) {
    if (initStatus)
        return;
    m_width = width;
    m_height = height;

    glViewport(0, 0, width, height);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    setupColorProgram();
    setupTextureProgram();
    initStatus = true;
}

void FOGLRenderContext::drawRect(float x, float y, float w, float h, const glm::vec4 &color)  {
    glm::mat4 mvp = glm::ortho(0.f, float(m_width), float(m_height), 0.f, -1.f, 1.f);
    glUseProgram(m_colorProgram);
    glUniformMatrix4fv(glGetUniformLocation(m_colorProgram, "uMVP"), 1, GL_FALSE, &mvp[0][0]);
    glUniform4fv(glGetUniformLocation(m_colorProgram, "uColor"), 1, &color[0]);

    float vertices[] = { x, y, x + w, y, x + w, y + h, x, y + h };
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}

void FOGLRenderContext::drawTexture(const std::string &path, float x, float y, float w, float h)  {
    GLuint tex = getTexture(path);
    if (tex != 0) drawTexture(tex, x, y, w, h);
}

void FOGLRenderContext::drawTexture(int rcID, float x, float y, float w, float h) {
    GLuint tex = getTexture(rcID);
    if (tex != 0) drawTexture(tex, x, y, w, h);
}

void FOGLRenderContext::drawTexture(GLuint tex, float x, float y, float w, float h) {
    glm::mat4 mvp = glm::ortho(0.f, float(m_width), float(m_height), 0.f, -1.f, 1.f);
    glUseProgram(m_textureProgram);
    glUniformMatrix4fv(glGetUniformLocation(m_textureProgram, "uMVP"), 1, GL_FALSE, &mvp[0][0]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(glGetUniformLocation(m_textureProgram, "uTexture"), 0);

    float vertices[] = {
        x, y, 0.f, 0.f,
        x + w, y, 1.f, 0.f,
        x + w, y + h, 1.f, 1.f,
        x, y + h, 0.f, 1.f
    };

    glBindVertexArray(m_vaoTex);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboTex);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}

GLuint FOGLRenderContext::getTexture(const std::string &path) {
    if (m_textureCacheStr.count(path)) return m_textureCacheStr[path];

    GLuint tex = m_resourceManager.loadTextureFromFile(path);
    m_textureCacheStr[path] = tex;
    return tex;
}

GLuint FOGLRenderContext::getTexture(int rcID) {
    if (m_textureCacheRC.count(rcID)) return m_textureCacheRC[rcID];

    GLuint tex = m_resourceManager.loadTextureFromRC(rcID);
    m_textureCacheRC[rcID] = tex;
    return tex;
}

void FOGLRenderContext::setupColorProgram() {
    const char* vs = R"(
            #version 330 core
            layout(location=0) in vec2 aPos;
            uniform mat4 uMVP;
            void main() { gl_Position = uMVP * vec4(aPos,0.0,1.0); }
        )";

    const char* fs = R"(
            #version 330 core
            out vec4 FragColor;
            uniform vec4 uColor;
            void main() { FragColor = uColor; }
        )";

    m_colorProgram = createProgram(vs, fs);

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glBindVertexArray(0);
}

void FOGLRenderContext::setupTextureProgram() {
    const char* vs = R"(
            #version 330 core
            layout(location=0) in vec2 aPos;
            layout(location=1) in vec2 aTex;
            uniform mat4 uMVP;
            out vec2 vTex;
            void main() { gl_Position = uMVP * vec4(aPos,0.0,1.0); vTex = aTex; }
        )";

    const char* fs = R"(
            #version 330 core
            in vec2 vTex;
            out vec4 FragColor;
            uniform sampler2D uTexture;
            void main() { FragColor = texture(uTexture, vTex); }
        )";

    m_textureProgram = createProgram(vs, fs);

    glGenVertexArrays(1, &m_vaoTex);
    glGenBuffers(1, &m_vboTex);
    glBindVertexArray(m_vaoTex);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboTex);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 16, nullptr, GL_DYNAMIC_DRAW); // 4顶点 * (pos+tex) 4 floats
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);
}

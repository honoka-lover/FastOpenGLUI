//
// Created by honoka on 2026/1/31.
//

#include "FOGLRenderContext.h"

#include "FOGLResourceManager.h"
#include "FOGLFunction.h"
#include "glm/gtc/type_ptr.inl"
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

    // if (m_vboTex) glDeleteBuffers(1, &m_vboTex);
    // if (m_vaoTex) glDeleteVertexArrays(1, &m_vaoTex);
    // if (m_textureProgram) glDeleteProgram(m_textureProgram);
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
    // setupTextureProgram();
    setupTextProgram();
    initStatus = true;
}

void FOGLRenderContext::drawRect(float x, float y, float w, float h, const glm::vec4 &color,float radius)  {
    drawTexture(0,x,y,w,h,radius,color,false);
}

void FOGLRenderContext::drawTexture(const std::string &path, float x, float y, float w, float h,float radius)  {
    GLuint tex = getTexture(path);
    if (tex != 0) drawTexture(tex, x, y, w, h,radius);
}

void FOGLRenderContext::drawTexture(int rcID, float x, float y, float w, float h,float radius) {
    GLuint tex = getTexture(rcID);
    if (tex != 0) drawTexture(tex, x, y, w, h,radius);
}

void FOGLRenderContext::drawGlyphInstances(GLuint atlasTex, const std::vector<GlyphInstance>& instances) const {
    if (instances.empty()) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(m_textProgram);
    glBindVertexArray(m_textVAO);

    auto m_projection = glm::ortho(0.0f, (float)600,
                             (float)400, 0.0f,  // Y 轴向下
                             -1.0f, 1.0f);
    // 设置投影矩阵
    GLint projLoc = glGetUniformLocation(m_textProgram, "u_projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(m_projection));

    // 绑定纹理
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, atlasTex);
    glUniform1i(glGetUniformLocation(m_textProgram, "u_texture"), 0);

    // 上传实例数据
    glBindBuffer(GL_ARRAY_BUFFER, m_textInstanceVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 instances.size() * sizeof(GlyphInstance),
                 instances.data(),
                 GL_DYNAMIC_DRAW);

    // 实例化绘制
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, (GLsizei)instances.size());

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
}

void FOGLRenderContext::drawTexture(GLuint tex, float x, float y, float w, float h,float radius,const glm::vec4& color,bool useTexture) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 mvp = glm::ortho(-1.0f, 1.0f, -1.0f, 1.f, -1.f, 1.f);
    glUseProgram(m_colorProgram);

    float vertices[] = {
        x,y, 0.0f,0.0f,
        x + w,y,1.0f,0.0f,
        x,y + h,0.0f,1.0f,
        x + w,y + h,1.0f,1.0f
    };
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glUniform1i(glGetUniformLocation(m_colorProgram, "useBackTexture"), useTexture);

    glUniform2f(glGetUniformLocation(m_colorProgram, "screenSize"), m_width, m_height);

    glUniform2f(glGetUniformLocation(m_colorProgram, "resolution"), w, h);

    if (radius > h/2)
        radius = h/2;
    glUniform1f(glGetUniformLocation(m_colorProgram, "radius"), radius);

    glUniform2f(glGetUniformLocation(m_colorProgram, "pos"), x, y);

    glUniformMatrix4fv(glGetUniformLocation(m_colorProgram, "projection"), 1, GL_FALSE, &mvp[0][0]);
    glUniform4fv(glGetUniformLocation(m_colorProgram, "buttonColor"), 1, &color[0]);

    if (useTexture)
        glBindTexture(GL_TEXTURE_2D,tex);
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glUseProgram(0);
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

    const char* fs = R"(
            #version 330 core
            out vec4 FragColor;
            in vec2 TexCoords;
            in vec2 TextCoords;       // 存放图片四个角位置
            uniform vec2 resolution;  // 按钮宽高
            uniform vec4 buttonColor; // 按钮颜色
            uniform float radius;     // 圆角半径
            uniform vec2 pos;
            uniform sampler2D backgroundTexture;
            uniform bool useBackTexture;    //是否使用背景图
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
                vec4 resultColor = buttonColor;
                if(useBackTexture)
                    resultColor = texture(backgroundTexture, TextCoords);

                // 将纹理坐标从 [0, 1] 映射到按钮的局部坐标系
                vec2 localCoord = TexCoords;

                vec2 pos1 = vec2(pos.x + radius , pos.y + radius);
                vec2 pos2 = vec2(pos.x + radius , pos.y + resolution.y - radius);
                vec2 pos3 = vec2(pos.x + resolution.x - radius , pos.y + radius);
                vec2 pos4 = vec2(pos.x + resolution.x - radius , pos.y + resolution.y - radius);

                if((localCoord.x > pos1.x  && localCoord.x < pos4.x && localCoord.y >= pos.y && localCoord.y <= pos.y+ resolution.y)
                ||( localCoord.y > pos1.y && localCoord.y < pos2.y && localCoord.x >= pos.x && localCoord.x <= pos.x + resolution.x)){
                    FragColor = resultColor;
                    return;
                }

                //寻找最近的点
                float point = minDistanceToCorner(localCoord, pos1, pos2,pos3,pos4);

                // 计算透明度（抗锯齿平滑效果）
                float alpha = 1.0 - smoothstep(radius - 1.0, radius, point);
                FragColor = vec4(resultColor.rgb, alpha * resultColor.a);
            }
        )";

    m_colorProgram = createProgram(vs, fs);
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0};
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void FOGLRenderContext::setupTextureProgram() {
    setupColorProgram();
}

void FOGLRenderContext::setupTextProgram() {
    // ===== 创建着色器 =====
    const char* vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec2 a_vertex; // (0,0) (1,0) (0,1) (1,1)

        layout(location = 1) in vec2 a_position;  // 实例：位置
        layout(location = 2) in vec2 a_size;      // 实例：尺寸
        layout(location = 3) in vec2 a_uv0;       // 实例：UV0
        layout(location = 4) in vec2 a_uv1;       // 实例：UV1
        layout(location = 5) in vec4 a_color;     // 实例：颜色

        uniform mat4 u_projection;
        out vec2 v_texCoord;
        out vec4 v_color;

        void main() {
            vec2 pos = a_position + a_vertex * a_size;
            gl_Position = u_projection * vec4(pos, 0.0, 1.0);

            v_texCoord = mix(a_uv0, a_uv1, a_vertex);
            v_color = a_color;
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        in vec2 v_texCoord;
        in vec4 v_color;
        out vec4 fragColor;

        uniform sampler2D u_texture;

        void main() {
            float alpha = texture(u_texture, v_texCoord).a;
            fragColor = vec4(v_color.rgb, v_color.a * alpha);
        }
    )";

    // 编译着色器...
    m_textProgram = createProgram(vertexShaderSource, fragmentShaderSource);

    // ===== 创建 VAO/VBO =====
    glGenVertexArrays(1, &m_textVAO);
    glBindVertexArray(m_textVAO);

    // 顶点数据 (quad 的 4 个角)
    float quadVertices[] = {
        0.0f, 0.0f,  // 左下
        1.0f, 0.0f,  // 右下
        0.0f, 1.0f,  // 左上
        1.0f, 1.0f   // 右上
    };

    GLuint quadVBO;
    glGenBuffers(1, &quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    // 实例数据 VBO
    glGenBuffers(1, &m_textInstanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_textInstanceVBO);

    // 属性 1: position
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GlyphInstance), (void*)offsetof(GlyphInstance, position));
    glVertexAttribDivisor(1, 1);

    // 属性 2: size
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GlyphInstance), (void*)offsetof(GlyphInstance, size));
    glVertexAttribDivisor(2, 1);

    // 属性 3: uv0
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(GlyphInstance), (void*)offsetof(GlyphInstance, uv0));
    glVertexAttribDivisor(3, 1);

    // 属性 4: uv1
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(GlyphInstance), (void*)offsetof(GlyphInstance, uv1));
    glVertexAttribDivisor(4, 1);

    // 属性 5: color
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(GlyphInstance), (void*)offsetof(GlyphInstance, color));
    glVertexAttribDivisor(5, 1);

    glBindVertexArray(0);
}

// 调试用：直接把字体 atlas 显示到屏幕上
void FOGLRenderContext::drawAtlasDebug(GLuint atlasTex, float x, float y, float w, float h) {
    drawTexture(atlasTex, x, y, w, h,0,glm::vec4(0,0,0,0));
}


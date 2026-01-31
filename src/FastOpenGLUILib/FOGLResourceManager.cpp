//
// Created by honoka on 2026/1/31.
//

#include "FOGLResourceManager.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

unsigned int FOGLResourceManager::loadTextureFromFile(const std::string &path) {
    int width, height, channels;
    stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 4); // 强制4通道RGBA
    if (!data) {
        printf("Failed to load image: %s\n", path.c_str());
        return 0;
    }

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    return tex;
}

unsigned int FOGLResourceManager::loadTextureFromRC(int rcID) {
    HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(rcID), RT_RCDATA);
    if (!hRes) {
        printf("Failed to find RC resource: %d\n", rcID);
        return 0;
    }

    HGLOBAL hData = LoadResource(NULL, hRes);
    if (!hData) {
        printf("Failed to load RC resource: %d\n", rcID);
        return 0;
    }

    DWORD size = SizeofResource(NULL, hRes);
    void* pData = LockResource(hData);
    if (!pData || size == 0) {
        printf("Failed to lock RC resource: %d\n", rcID);
        return 0;
    }

    int width, height, channels;
    stbi_uc* img = stbi_load_from_memory((stbi_uc*)pData, size, &width, &height, &channels, 4);
    if (!img) {
        printf("Failed to decode RC resource: %d\n", rcID);
        return 0;
    }

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(img);
    return tex;
}

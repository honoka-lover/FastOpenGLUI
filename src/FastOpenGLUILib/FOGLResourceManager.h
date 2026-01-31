//
// Created by honoka on 2026/1/31.
//

#ifndef FASTOPENGLUI_FOGLRESOURCEMANAGER_H
#define FASTOPENGLUI_FOGLRESOURCEMANAGER_H
#include <glad/glad.h>
#include "FOGLResourceProvider.h"
#include <vector>
#include <memory>


class FOGLResourceManager {
public:

    // 添加 Provider
    void addProvider(std::shared_ptr<FOGLResourceProvider> provider) {
        m_providers.push_back(std::move(provider));
    }

    // 核心加载函数
    std::vector<uint8_t> load(const std::string& name) {
        for (auto& p : m_providers) {
            std::vector<uint8_t> data;
            if (p->loadBinary(name, data)) return data;
        }
        return {};
    }

    // 从本地文件加载纹理
    unsigned int loadTextureFromFile(const std::string& path);

    // 从 Windows RC 资源加载纹理
    unsigned int loadTextureFromRC(int rcID);

private:
    std::vector<std::shared_ptr<FOGLResourceProvider>> m_providers;
};


#endif //FASTOPENGLUI_FOGLRESOURCEMANAGER_H
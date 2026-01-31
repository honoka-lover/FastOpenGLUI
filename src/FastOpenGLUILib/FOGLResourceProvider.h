//
// Created by honoka on 2026/1/31.
//

#ifndef FASTOPENGLUI_FOGLRESOURCEPROVIDER_H
#define FASTOPENGLUI_FOGLRESOURCEPROVIDER_H
#include <vector>
#include <cstdint>
#include <string>
#include <memory>

class FOGLResourceProvider {
public:
    virtual ~FOGLResourceProvider() = default;
    virtual bool loadBinary(const std::string& name, std::vector<uint8_t>& outData) = 0;
};

// Windows RC Provider
#ifdef _WIN32
#include <windows.h>

class FOGLRCProvider : public FOGLResourceProvider {
public:
    FOGLRCProvider(HMODULE hModule = GetModuleHandle(nullptr)) : m_module(hModule) {}

    bool loadBinary(const std::string& name, std::vector<uint8_t>& outData) override {
        HRSRC res = FindResourceA(m_module, name.c_str(), "BINARY");
        if (!res) return false;

        HGLOBAL hGlobal = LoadResource(m_module, res);
        DWORD size = SizeofResource(m_module, res);
        void* pData = LockResource(hGlobal);
        if (!pData || size == 0) return false;

        outData.assign((uint8_t*)pData, (uint8_t*)pData + size);
        return true;
    }

private:
    HMODULE m_module;
};
#endif

// File Provider
#include <fstream>
class FOGLFileProvider : public FOGLResourceProvider {
public:
    bool loadBinary(const std::string& name, std::vector<uint8_t>& outData) override {
        std::ifstream file(name, std::ios::binary);
        if (!file) return false;
        outData.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
        return true;
    }
};

#endif //FASTOPENGLUI_FOGLRESOURCEPROVIDER_H
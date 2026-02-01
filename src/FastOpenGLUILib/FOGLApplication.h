//
// Created by honoka on 2026/1/31.
//

#ifndef FASTOPENGLUI_FOGLAPPLICATION_H
#define FASTOPENGLUI_FOGLAPPLICATION_H
#include "vector"
#include "memory"
#include "FOGLWidget.h"
class FOGLApplication {
public:
    // 获取唯一实例
    static FOGLApplication& instance();

    void init();

    void registerRoot(FOGLWidget* root);
    void unregisterRoot(FOGLWidget* root);


    void run();

    // 禁止拷贝 / 赋值
    FOGLApplication(const FOGLApplication&) = delete;
    FOGLApplication& operator=(const FOGLApplication&) = delete;

private:
    FOGLApplication();   // 构造私有
    ~FOGLApplication();  // 析构私有或默认

    void updateRoot(); // ⭐ 统一处理 root 变更
private:
    std::vector<FOGLWidget*> m_roots;       // 只存指针，不拥有
    std::vector<FOGLWidget*> m_pendingAdd;
    std::vector<FOGLWidget*> m_pendingRemove;
};


#endif //FASTOPENGLUI_FOGLAPPLICATION_H
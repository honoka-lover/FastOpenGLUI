cmake_minimum_required(VERSION 3.16)

project(FastOpenGLUI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
        src/Component
        src/regeditModel
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include/FastOpenGLUI
)

## 查找 GLFW 静态库
#find_library(GLFW_LIBRARY glfw3 PATHS ${CMAKE_SOURCE_DIR}/lib REQUIRED)


add_library(FOGLLib STATIC
        ${CMAKE_SOURCE_DIR}/include/glad.c
        ${CMAKE_SOURCE_DIR}/src/Component/FOGLRectangle.cpp
        ${CMAKE_SOURCE_DIR}/src/Component/text_renderer.cpp
        ${CMAKE_SOURCE_DIR}/src/Component/shader.cpp
        ${CMAKE_SOURCE_DIR}/src/Component/regeditFunction.cpp
        ${CMAKE_SOURCE_DIR}/src/Component/CommonFunc.cpp
        ${CMAKE_SOURCE_DIR}/src/Component/FOGLWindow.cpp
        ${CMAKE_SOURCE_DIR}/src/Component/FOGLProgressBar.cpp
        ${CMAKE_SOURCE_DIR}/src/Component/Timer.cpp
)

target_include_directories(FOGLLib PRIVATE
        ${CMAKE_SOURCE_DIR}/src/Component
        ${CMAKE_SOURCE_DIR}/src/regeditModel
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include/FastOpenGLUI
)


# 检测当前构建模式
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    file(GLOB LIBS
            ${CMAKE_SOURCE_DIR}/lib/release/freetype.lib
            ${CMAKE_SOURCE_DIR}/lib/release/glfw3.lib
    )
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    file(GLOB LIBS
            ${CMAKE_SOURCE_DIR}/lib/debug/freetyped.lib
            ${CMAKE_SOURCE_DIR}/lib/debug/glfw3.lib
    )
endif()
target_link_libraries(FOGLLib PRIVATE
        opengl32
        Shlwapi
        Shell32
        psapi
        ${CMAKE_SOURCE_DIR}/lib/bz2.lib
        ${CMAKE_SOURCE_DIR}/lib/lzma.lib
        ${CMAKE_SOURCE_DIR}/lib/lz4.lib
        ${CMAKE_SOURCE_DIR}/lib/zstd.lib
        ${CMAKE_SOURCE_DIR}/lib/zlib.lib
        ${CMAKE_SOURCE_DIR}/lib/archive.lib
        ${LIBS}
)

set_property(TARGET FOGLLib PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_subdirectory(src/InstallWindow)
add_subdirectory(src/UninstallWindow)
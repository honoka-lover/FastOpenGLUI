cmake_minimum_required(VERSION 3.16)

project(FastOpenGLUI)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#include(FetchContent)
#
#FetchContent_Declare(
#        bit7z
#        GIT_REPOSITORY https://github.com/rikyoz/bit7z.git
#        GIT_TAG        master
#)

#FetchContent_MakeAvailable(bit7z)

# 设置 bit7z 的头文件和库路径
#set(BIT7Z_INCLUDE_DIR ${CMAKE_BINARY_DIR}/_deps/bit7z-src/include)  # 确保路径正确

include_directories(
        src/Component
        src/regeditModel
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include/FastOpenGLUI
)

## 查找 GLFW 静态库
#find_library(GLFW_LIBRARY glfw3 PATHS ${CMAKE_SOURCE_DIR}/lib REQUIRED)

set_source_files_properties(${CMAKE_SOURCE_DIR}/src/Component/FOGL.rc PROPERTIES LANGUAGE RC)

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
        src/test/main.cpp
        ${CMAKE_SOURCE_DIR}/src/Component/FOGL.rc
)

# 使用 bit7z 库
#add_dependencies(FOGLLib bit7z)  # 确保在构建时依赖 bit7z

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
#            ${CMAKE_SOURCE_DIR}/lib/release/bit7z64.lib
    )
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    file(GLOB LIBS
            ${CMAKE_SOURCE_DIR}/lib/debug/freetyped.lib
            ${CMAKE_SOURCE_DIR}/lib/debug/glfw3.lib
#            ${CMAKE_SOURCE_DIR}/lib/release/bit7z64.lib
    )
endif()
target_link_libraries(FOGLLib PRIVATE
        opengl32
        Shlwapi
        Shell32
        psapi
        ws2_32
        wldap32
        crypt32
        ${CMAKE_SOURCE_DIR}/lib/bz2.lib
        ${CMAKE_SOURCE_DIR}/lib/lzma.lib
        ${CMAKE_SOURCE_DIR}/lib/lz4.lib
        ${CMAKE_SOURCE_DIR}/lib/zstd.lib
        ${CMAKE_SOURCE_DIR}/lib/zlib.lib
        ${CMAKE_SOURCE_DIR}/lib/archive.lib
        ${CMAKE_SOURCE_DIR}/lib/libcurl.lib
        ${CMAKE_SOURCE_DIR}/lib/libssl.lib
        ${CMAKE_SOURCE_DIR}/lib/libcrypto.lib
        ${LIBS}
)

set_property(TARGET FOGLLib PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")


add_subdirectory(src/InstallWindow)
add_subdirectory(src/UninstallWindow)
add_subdirectory(src/test)
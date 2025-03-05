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


# 使用 bit7z 库
#add_dependencies(FOGLLib bit7z)  # 确保在构建时依赖 bit7z

target_include_directories(FOGLLib PRIVATE
        ${CMAKE_SOURCE_DIR}/src/Component
        ${CMAKE_SOURCE_DIR}/src/regeditModel
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include/FastOpenGLUI
)

if(MSVC)
    # 检测当前构建模式
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(FOGL_LIBS
                ${CMAKE_SOURCE_DIR}/lib/MSVC/release/freetype.lib
                ${CMAKE_SOURCE_DIR}/lib/MSVC/release/glfw3.lib
                ${CMAKE_SOURCE_DIR}/lib/MSVC/release/bit7z64.lib
        )
    elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(FOGL_LIBS
                ${CMAKE_SOURCE_DIR}/lib/MSVC/debug/freetyped.lib
                ${CMAKE_SOURCE_DIR}/lib/MSVC/debug/glfw3.lib
                ${CMAKE_SOURCE_DIR}/lib/MSVC/debug/bit7z.lib

        )
    endif()
    set(FOGL_LIBS
            ${FOGL_LIBS}
            ${CMAKE_SOURCE_DIR}/lib/MSVC/libcurl.lib
            ${CMAKE_SOURCE_DIR}/lib/MSVC/libssl.lib
            ${CMAKE_SOURCE_DIR}/lib/MSVC/libcrypto.lib
    )
    target_link_libraries(FOGLLib PRIVATE
            opengl32
            Shlwapi
            Shell32
            psapi
            ws2_32
            wldap32
            crypt32
            ${FOGL_LIBS}
    )

    set_property(TARGET FOGLLib PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()

if(MINGW)
     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc -pthread")
     set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")

     if(CMAKE_BUILD_TYPE STREQUAL "Release")
         set(FOGL_LIBS
                 ${CMAKE_SOURCE_DIR}/lib/MINGW/libbit7z64.a
         )
     elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
         set(FOGL_LIBS
                 ${CMAKE_SOURCE_DIR}/lib/MINGW/libbit7z64_d.a
         )
     endif()
     set(FOGL_LIBS ${FOGL_LIBS}
             ${CMAKE_SOURCE_DIR}/lib/MINGW/libfreetype.a
             ${CMAKE_SOURCE_DIR}/lib/MINGW/libglfw3.a
     )

    target_link_libraries(FOGLLib PRIVATE
            opengl32
            Shlwapi
            Shell32
            psapi
            ws2_32
            wldap32
            crypt32
            ${FOGL_LIBS}
    )
endif ()

add_subdirectory(src/InstallWindow)
add_subdirectory(src/UninstallWindow)
add_subdirectory(src/test)
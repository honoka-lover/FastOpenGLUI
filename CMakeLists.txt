cmake_minimum_required(VERSION 3.16)

project(FastOpenGLUI)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
        src/Component
        src/regeditModel
        ${CMAKE_SOURCE_DIR}/include
        src
)

## 查找 GLFW 静态库
#find_library(GLFW_LIBRARY glfw3 PATHS ${CMAKE_SOURCE_DIR}/lib REQUIRED)

# 检测当前构建模式
if(CMAKE_BUILD_TYPE STREQUAL "Release")
#    set(BlueScreenProject_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/blueScreenPackage)
#    set_target_properties(BlueScreenProject PROPERTIES
#            RUNTIME_OUTPUT_DIRECTORY ${BlueScreenProject_OUTPUT_DIR}
#    )

    file(GLOB LIBFILES
            ${CMAKE_SOURCE_DIR}/lib/release/freetype.lib
            ${CMAKE_SOURCE_DIR}/lib/release/glfw3.lib
    )

elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
#    set(BlueScreenProject_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/bin)
#    set_target_properties(BlueScreenProject PROPERTIES
#            RUNTIME_OUTPUT_DIRECTORY ${BlueScreenProject_OUTPUT_DIR}
#    )
    file(GLOB LIBFILES
            ${CMAKE_SOURCE_DIR}/lib/debug/freetyped.lib
            ${CMAKE_SOURCE_DIR}/lib/debug/glfw3.lib
    )
endif()


# 主程序
add_executable(${PROJECT_NAME}
        ${CMAKE_SOURCE_DIR}/include/ft2build.h
        src/main.cpp
        src/Component/FOGLRectangle.h
        src/Component/FOGLRectangle.cpp
        src/MainWindow.h
        src/MainWindow.cpp
        src/CommonFunc.cpp
        src/CommonFunc.h
        ${CMAKE_SOURCE_DIR}/include/glad.c
        resources/resources.rc
        src/Component/text_renderer.h
        src/Component/text_renderer.cpp
        src/Component/shader.h
        src/Component/shader.cpp
        src/Component/texture.h
        src/Component/texture.cpp
        src/regeditModel/regeditFunction.h
        src/regeditModel/regeditFunction.cpp
        src/CommonFunc.h
        src/CommonFunc.cpp
)


if(MSVC)
    target_link_libraries(${PROJECT_NAME} PRIVATE
            opengl32
            Shlwapi
            ${CMAKE_SOURCE_DIR}/lib/bz2.lib
            ${CMAKE_SOURCE_DIR}/lib/lzma.lib
            ${CMAKE_SOURCE_DIR}/lib/lz4.lib
            ${CMAKE_SOURCE_DIR}/lib/zstd.lib
            ${CMAKE_SOURCE_DIR}/lib/zlib.lib
            ${CMAKE_SOURCE_DIR}/lib/archive.lib
            ${LIBFILES}
    )

    #提权
#    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"")
    set_property(TARGET ${PROJECT_NAME} PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
elseif (MINGW)
    target_link_libraries(${PROJECT_NAME} PRIVATE
            ${GLFW_LIBRARY}
            ${CMAKE_SOURCE_DIR}/lib/libfreetype.a
            opengl32
            ${LIBFILES}
#            ${CMAKE_SOURCE_DIR}/lib/libarchive.dll.a
            imm32 winmm setupapi gdi32 kernel32 user32 version Shlwapi
    )
endif ()


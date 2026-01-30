project(FOGLLib)

add_library(FOGLLib STATIC
        glad.c
        FOGLRectangle.cpp
        text_renderer.cpp
        shader.cpp
        regeditFunction.cpp
        CommonFunc.cpp
        FOGLWindow.cpp
        FOGLProgressBar.cpp
        Timer.cpp
)

target_include_directories(FOGLLib PUBLIC
    .
    ${CMAKE_SOURCE_DIR}/include
)

if(MSVC)
    target_link_libraries(FOGLLib PUBLIC
            opengl32
            Shlwapi
            Shell32
            psapi
            ws2_32
            wldap32
            crypt32
            bit7z
            glfw
            glm::glm
            freetype
    )
    target_link_libraries(FOGLLib PRIVATE legacy_stdio_definitions)
endif ()




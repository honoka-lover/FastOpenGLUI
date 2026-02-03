project(FastOpenGLUILib)


add_library(${PROJECT_NAME} STATIC
        FOGLResourceManager.cpp
        FOGLResourceProvider.cpp
        FOGLWidget.cpp
        FOGLSubWindow.cpp
        FOGLRenderContext.cpp
        FOGLLayout.cpp
        FOGLVBoxLayout.cpp
        FOGLHBoxLayout.cpp
        FOGLGridLayout.cpp
        FOGLButton.cpp
        FOGLApplication.cpp
        FOGLContainer.cpp
        FOGLTextWidget.cpp
        FOGLTextEditWidget.cpp
        FOGLFont.cpp
        FOGLFunction.cpp
        glad.c
)

target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        .
)

if(MSVC)
    target_link_libraries(${PROJECT_NAME} PUBLIC
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
endif ()
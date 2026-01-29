project(imgUISample)

add_executable(imgUISample WIN32
        FoglSample.cpp
        FoglSample.h
        Sample.cpp
        imgui.ini
        ../../include/imgui/imgui.cpp
        ../../include/imgui/imgui_draw.cpp
        ../../include/imgui/backends/imgui_impl_glfw.cpp
        ../../include/imgui/imgui_widgets.cpp
        ../../include/imgui/imgui_tables.cpp
        ../../include/imgui/imgui_demo.cpp
        ../../include/imgui/backends/imgui_impl_opengl3.cpp
        ${CMAKE_SOURCE_DIR}/resources/Install.rc

)

target_link_libraries(imgUISample PRIVATE
        FOGLLib
)

if(MSVC)
    set_property(TARGET imgUISample PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()

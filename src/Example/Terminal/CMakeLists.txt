

add_executable(Terminal
        main.cpp
        imgui.ini
        ${CMAKE_SOURCE_DIR}/include/imgui/imgui.cpp
        ${CMAKE_SOURCE_DIR}/include/imgui/imgui_draw.cpp
        ${CMAKE_SOURCE_DIR}/include/imgui/backends/imgui_impl_glfw.cpp
        ${CMAKE_SOURCE_DIR}/include/imgui/imgui_widgets.cpp
        ${CMAKE_SOURCE_DIR}/include/imgui/imgui_tables.cpp
        ${CMAKE_SOURCE_DIR}/include/imgui/imgui_demo.cpp
        ${CMAKE_SOURCE_DIR}/include/imgui/backends/imgui_impl_opengl3.cpp
)

target_link_libraries(Terminal PRIVATE
        glfw
        opengl32
)
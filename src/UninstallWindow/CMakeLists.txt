project(UninstallProject)

# 主程序
add_executable(UninstallProject
        Uninstall.cpp
        UninstallWindow.cpp
        UninstallWindow.h
        ${CMAKE_SOURCE_DIR}/resources/Uninstall.rc
        UninstallResources.h
)

if(MSVC)
    target_link_libraries(UninstallProject PRIVATE
            FOGLLib
    )
endif ()

set_target_properties(UninstallProject PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)




project(InstallProject)

# 主程序
add_executable(InstallProject
        Install.cpp
        InstallWindow.cpp
        InstallWindow.h
        ${CMAKE_SOURCE_DIR}/resources/Install.rc
        InstallResources.h
)

if(MSVC)
    target_link_libraries(InstallProject PRIVATE
            FOGLLib
    )

endif ()

set_target_properties(InstallProject PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)




project(InstallProject)

# 主程序
add_executable(InstallProject WIN32
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

set_property(TARGET InstallProject PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
#提权
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"")


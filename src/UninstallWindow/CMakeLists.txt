project(UninstallProject)

# 主程序
add_executable(UninstallProject WIN32
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

set_target_properties(UninstallProject PROPERTIES OUTPUT_NAME "uninstall")

set_property(TARGET UninstallProject PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
#提权
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"")

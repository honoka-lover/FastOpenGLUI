project(UninstallProject)

if(MSVC)
    #提权
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"")
endif ()

set(SOURCE_FILE
        Uninstall.cpp
        UninstallWindow.cpp
        UninstallWindow.h
        Uninstall.rc
        UninstallResources.h
)


# 主程序
add_executable(UninstallProject WIN32
        ${SOURCE_FILE}
)

target_link_libraries(UninstallProject PRIVATE
        FOGLLib
)


set_target_properties(UninstallProject PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set_target_properties(UninstallProject PROPERTIES
        OUTPUT_NAME "uninstall"
)





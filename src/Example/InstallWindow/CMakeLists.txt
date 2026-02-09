project(InstallProject)

if(MSVC)
    #提权
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"")

endif ()

set(SOURCE_FILE
        Install.cpp
        InstallWindow.cpp
        InstallWindow.h
        Install.rc
        InstallResources.h
)

# 主程序
add_executable(InstallProject WIN32
        ${SOURCE_FILE}
)


target_link_libraries(InstallProject PRIVATE
        FOGLLib
)





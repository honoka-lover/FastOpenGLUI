project(UninstallProject)

if(MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc -pthread")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")

    # 关键配置：显式禁用自动清单生成
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-insert-timestamp")
     #强制使用 MinGW 资源编译器
    set(CMAKE_RC_COMPILER windres)
    enable_language(RC)
elseif(MSVC)
    #提权
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"")
endif ()

set(SOURCE_FILE
        Uninstall.cpp
        UninstallWindow.cpp
        UninstallWindow.h
        ${CMAKE_SOURCE_DIR}/resources/Uninstall.rc
        UninstallResources.h
)

if(MINGW)
    set(SOURCE_FILE
        ${SOURCE_FILE}
        ${CMAKE_SOURCE_DIR}/resources/app.rc
        ${CMAKE_SOURCE_DIR}/resources/app.manifest
    )
endif ()

# 主程序
add_executable(UninstallProject WIN32
        ${SOURCE_FILE}
)

target_link_libraries(UninstallProject PRIVATE
        FOGLLib
)


set_target_properties(UninstallProject PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set_target_properties(UninstallProject PROPERTIES OUTPUT_NAME "uninstall")


if(MSVC)
    set_property(TARGET UninstallProject PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()


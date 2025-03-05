project(InstallProject)

if(MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc -pthread")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")

    # 关键配置：显式禁用自动清单生成
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-insert-timestamp")
    # 强制使用 MinGW 资源编译器
    set(CMAKE_RC_COMPILER windres)
    enable_language(RC)
elseif(MSVC)
    #提权
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"")

endif ()

set(SOURCE_FILE
        Install.cpp
        InstallWindow.cpp
        InstallWindow.h
        ${CMAKE_SOURCE_DIR}/resources/Install.rc
        InstallResources.h
)

if(MINGW)
    set(SOURCE_FILE
        ${SOURCE_FILE}
        ${CMAKE_SOURCE_DIR}/resources/app.rc
        ${CMAKE_SOURCE_DIR}/resources/app.manifest
    )
endif ()

# 主程序
add_executable(InstallProject WIN32
        ${SOURCE_FILE}
)


target_link_libraries(InstallProject PRIVATE
        FOGLLib
)

set_target_properties(InstallProject PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

if(MSVC)
    set_property(TARGET InstallProject PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()


project(test)

# 主程序
add_executable(test
        main.cpp

)

if(MSVC)
    target_link_libraries(test PRIVATE
            FOGLLib
    )
endif ()

set_target_properties(test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set_target_properties(test PROPERTIES OUTPUT_NAME "test")

set_property(TARGET test PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")


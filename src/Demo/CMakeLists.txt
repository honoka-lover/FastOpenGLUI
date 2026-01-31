project(Demo)

add_executable(Demo WIN32
    main.cpp
)

target_link_libraries(Demo PRIVATE
    FastOpenGLUILib
)
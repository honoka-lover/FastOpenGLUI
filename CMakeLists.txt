cmake_minimum_required(VERSION 3.16)

project(FastOpenGLUI)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include(cmake/dependencies.cmake)

include_directories(
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include/InstallHeader
        ${CMAKE_CURRENT_SOURCE_DIR}/include/imgui
        ${CMAKE_CURRENT_SOURCE_DIR}/include/imgui/backends
)


add_subdirectory(src/FOGLLib)
add_subdirectory(src/FastOpenGLUILib)
add_subdirectory(src/InstallWindow)
add_subdirectory(src/UninstallWindow)
add_subdirectory(src/imguiSample)
add_subdirectory(src/demo)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

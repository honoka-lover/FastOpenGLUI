## 所有 FetchContent / CPM 的源码统一放到 third_party
#set(FETCHCONTENT_BASE_DIR ${CMAKE_SOURCE_DIR}/_deps CACHE PATH "")
#
## 禁止每次 configure 去联网检查
#set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE BOOL "")
#
## 如果用 CPM（bit7z 在用）
#set(CPM_SOURCE_CACHE ${CMAKE_SOURCE_DIR}/_deps CACHE PATH "")
#set(CPM_USE_LOCAL_PACKAGES ON CACHE BOOL "")


include(${CMAKE_CURRENT_LIST_DIR}/deps/glfw.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/deps/freetype.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/deps/glm.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/deps/bit7z.cmake)



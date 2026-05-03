add_library(bagl)
add_library(bagl::bagl ALIAS bagl)

set(BAGL_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
set(BAGL_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")

# Specify sources
target_sources(bagl
  PRIVATE
    "${BAGL_SOURCE_DIR}/bagl.c"
    "${BAGL_SOURCE_DIR}/window.c"
    "${BAGL_SOURCE_DIR}/internal/bagl_window.h"
    "${BAGL_SOURCE_DIR}/internal/bagl_window.c"
    "${BAGL_SOURCE_DIR}/internal/bagl_state.h"
    "${BAGL_SOURCE_DIR}/internal/utils.h"
  PUBLIC
    FILE_SET bagl_headers
    TYPE HEADERS
    BASE_DIRS
      "${BAGL_INCLUDE_DIR}"
    FILES
      "${BAGL_INCLUDE_DIR}/bagl/bagl.h"
      "${BAGL_INCLUDE_DIR}/bagl/window.h"
)
target_include_directories(bagl
  PUBLIC
    "${BAGL_INCLUDE_DIR}"
)

# Fetch GLFW
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.4
)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)
add_library(bagl::glfw ALIAS glfw)

# Link against dependencies
target_link_libraries(bagl
  PRIVATE
    bagl::glfw
    bagl::glad
    bagl::stb_image
)
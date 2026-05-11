add_library(bagl)
add_library(bagl::bagl ALIAS bagl)

set(BAGL_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
set(BAGL_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")

# Specify sources
target_sources(bagl
  PRIVATE
    "${BAGL_SOURCE_DIR}/bagl.c"
    "${BAGL_SOURCE_DIR}/window.c"
    "${BAGL_SOURCE_DIR}/frame.c"
    "${BAGL_SOURCE_DIR}/image.c"
    "${BAGL_SOURCE_DIR}/shader.c"
    "${BAGL_SOURCE_DIR}/material.c"
    "${BAGL_SOURCE_DIR}/mesh.c"
    "${BAGL_SOURCE_DIR}/model.c"
    "${BAGL_SOURCE_DIR}/camera.c"
    "${BAGL_SOURCE_DIR}/obj.c"
    "${BAGL_SOURCE_DIR}/internal/matrix.c"
    "${BAGL_SOURCE_DIR}/internal/matrix.h"
    "${BAGL_SOURCE_DIR}/internal/bagl_window.c"
    "${BAGL_SOURCE_DIR}/internal/bagl_window.h"
    "${BAGL_SOURCE_DIR}/internal/bagl_shader.h"
    "${BAGL_SOURCE_DIR}/internal/bagl_model.h"
    "${BAGL_SOURCE_DIR}/internal/bagl_image.h"
    "${BAGL_SOURCE_DIR}/internal/bagl_state.h"
    "${BAGL_SOURCE_DIR}/internal/bagl_mesh.h"
    "${BAGL_SOURCE_DIR}/internal/bagl_material.h"
    "${BAGL_SOURCE_DIR}/internal/bagl_camera.h"
    "${BAGL_SOURCE_DIR}/internal/utils.h"
  PUBLIC
    FILE_SET bagl_headers
    TYPE HEADERS
    BASE_DIRS
      "${BAGL_INCLUDE_DIR}"
    FILES
      "${BAGL_INCLUDE_DIR}/bagl/bagl.h"
      "${BAGL_INCLUDE_DIR}/bagl/window.h"
      "${BAGL_INCLUDE_DIR}/bagl/frame.h"
      "${BAGL_INCLUDE_DIR}/bagl/image.h"
      "${BAGL_INCLUDE_DIR}/bagl/shader.h"
      "${BAGL_INCLUDE_DIR}/bagl/mesh.h"
      "${BAGL_INCLUDE_DIR}/bagl/material.h"
      "${BAGL_INCLUDE_DIR}/bagl/model.h"
      "${BAGL_INCLUDE_DIR}/bagl/camera.h"
      "${BAGL_INCLUDE_DIR}/bagl/obj.h"
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
add_library(glad)
add_library(bagl::glad ALIAS glad)

set(BAGL_GLAD_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/thirdparty/glad/include/")
set(BAGL_GLAD_SOURCE_DIR "${CMAKE_SOURCE_DIR}/thirdparty/glad/src")

target_sources(glad
  PRIVATE
    "${BAGL_GLAD_SOURCE_DIR}/glad.c"
  PUBLIC 
    FILE_SET glad_headers
    TYPE HEADERS
    FILES
      "${BAGL_GLAD_INCLUDE_DIR}/glad/glad.h"
      "${BAGL_GLAD_INCLUDE_DIR}/KHR/khrplatform.h"
)
target_include_directories(glad
  PUBLIC
    "${BAGL_GLAD_INCLUDE_DIR}"
)
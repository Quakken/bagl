add_library(stb_image)
add_library(bagl::stb_image ALIAS stb_image)

set(BAGL_STB_IMAGE_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/thirdparty/stb_image/include/")
set(BAGL_STB_IMAGE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/thirdparty/stb_image/src")

target_sources(stb_image
  PRIVATE
    "${BAGL_STB_IMAGE_SOURCE_DIR}/stb_image.c"
  PUBLIC 
    FILE_SET stb_image_headers
    TYPE HEADERS
    FILES
      "${BAGL_STB_IMAGE_INCLUDE_DIR}/stb/stb_image.h"
)
target_include_directories(stb_image
  PUBLIC
    "${BAGL_STB_IMAGE_INCLUDE_DIR}"
)
cmake_minimum_required(VERSION 3.50)
project(bagl
  VERSION 0.0.1
  DESCRIPTION "A simple 3D rendering library for C"
  LANGUAGES C
)
include(FetchContent)

# Build options
option(BAGL_BUILD_EXAMPLES OFF "Whether to build bagl example projects")

# Build dependencies
add_subdirectory(thirdparty)
# Build the main library
add_subdirectory(src)

# Build example projects
if (${BAGL_BUILD_EXAMPLES})
  add_subdirectory(examples)
endif()
add_custom_target(  
  example_assets  
  COMMAND ${CMAKE_COMMAND} -E copy_directory  
    ${CMAKE_SOURCE_DIR}/examples/assets/  
    ${CMAKE_BINARY_DIR}/examples/assets/ 
)

add_subdirectory(hello_bagl)
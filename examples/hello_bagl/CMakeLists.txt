add_executable(hello_bagl)
target_sources(hello_bagl
  PRIVATE
    main.c
)

target_link_libraries(hello_bagl
  PRIVATE
    bagl::bagl
)
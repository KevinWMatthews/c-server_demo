cmake_minimum_required(VERSION 3.10.2)
project(server_demo VERSION 0.1.0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
add_compile_options(-Wall)

add_subdirectory(tcp_socket)
add_subdirectory(unix_socket)

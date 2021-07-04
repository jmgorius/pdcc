cmake_minimum_required(VERSION 3.10)
project(pdcc C)

set(CMAKE_C_STANDARD 11)

add_executable(hello hello.c)

add_executable(log-example log-example.c log.c)
target_link_libraries(log-example pthread)

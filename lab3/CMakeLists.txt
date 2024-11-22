cmake_minimum_required(VERSION 3.10)

project(lab3)

set(CMAKE_C_STANDARD 17)

include_directories(include)

add_executable(lab3_exe src/parent.c src/tools.c)
add_executable(child3 src/child.c src/tools.c)

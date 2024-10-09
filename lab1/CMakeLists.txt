cmake_minimum_required(VERSION 3.10)

project(lab1)

set(CMAKE_C_STANDARD 17)

add_executable(parent src/parent.c)
add_executable(child src/child.c)


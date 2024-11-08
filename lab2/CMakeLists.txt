cmake_minimum_required(VERSION 3.10)

project(lab2)

set(CMAKE_C_STANDARD 17)

include_directories(../lab2/include)

add_executable(lab2_exe
    main.c
    src/lab2.c
)

find_package(Threads REQUIRED)
target_link_libraries(lab2_exe Threads::Threads)
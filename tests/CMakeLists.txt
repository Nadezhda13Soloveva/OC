cmake_minimum_required(VERSION 3.10)

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.11.0
)

FetchContent_MakeAvailable(googletest)

set(CMAKE_CXX_STANDARD 17)

add_executable(test1 test1.cpp)
add_dependencies(test1 parent)

target_link_libraries(test1 PRIVATE GTest::gtest GTest::gtest_main)

add_test(NAME test1 COMMAND test1)

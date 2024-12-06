cmake_minimum_required(VERSION 3.10)

# Load Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.11.0
)
FetchContent_MakeAvailable(googletest)

# Add test executables
add_executable(test1 test1.cpp)
target_link_libraries(test1 PRIVATE GTest::gtest GTest::gtest_main)
add_test(NAME test1 COMMAND test1)

add_executable(test2 test2.cpp)
target_link_libraries(test2 PRIVATE lab2_lib GTest::gtest GTest::gtest_main pthread)
add_test(NAME test2 COMMAND test2)

add_executable(test3 test3.cpp)
target_link_libraries(test3 PRIVATE lab3_lib GTest::gtest GTest::gtest_main pthread)
add_test(NAME test3 COMMAND test3)

add_executable(test4 test4.cpp)
target_link_libraries(test4 PRIVATE GTest::gtest GTest::gtest_main dl)
add_test(NAME test4 COMMAND test4)

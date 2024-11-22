cmake_minimum_required(VERSION 3.10)

# Подключаем FetchContent для загрузки Google Test
include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.11.0
)

FetchContent_MakeAvailable(googletest)

# Устанавливаем стандарты C и C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 17)

# Добавляем исполняемый файл для теста test1
add_executable(test1 test1.cpp)
target_link_libraries(test1 PRIVATE GTest::gtest GTest::gtest_main)
add_test(NAME test1 COMMAND test1)

# Добавляем исполняемый файл для теста test2
include_directories(../lab2/include)
add_library(lab2_lib ../lab2/src/lab2.c)
target_include_directories(lab2_lib PUBLIC ../lab2/include)

add_executable(test2 test2.cpp)
target_link_libraries(test2 PRIVATE lab2_lib GTest::gtest GTest::gtest_main pthread)
add_test(NAME test2 COMMAND test2)

# Добавляем исполняемый файл для теста test3
include_directories(../lab3/include)
add_library(lab3_lib ../lab3/src/parent.c ../lab3/src/child.c ../lab3/src/tools.c)
target_include_directories(lab3_lib PUBLIC ../lab3/include)

add_executable(test3 test3.cpp)
target_link_libraries(test3 PRIVATE lab3_lib GTest::gtest GTest::gtest_main pthread)
add_test(NAME test3 COMMAND test3)
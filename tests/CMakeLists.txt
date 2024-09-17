include(FetchContent)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        release-1.11.0
)

FetchContent_MakeAvailable(googletest)
add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE gtest_main)


set(CMAKE_CXX_STANDARD 17)

add_executable(lab1_test lab1_test.cpp ../lab1/src/parent.c ../lab1/include/parent.h
        ../lab1/src/utils.c ../lab1/include/utils.h)

target_link_libraries(lab1_test PRIVATE GTest::GTest)
target_include_directories(lab1_test PRIVATE ../lab2/include)

add_test(lab1_test lab1_test)


#add_executable(lab3_test lab3_test.cpp ../lab3/include/lab3.h ../lab3/src/lab3.cpp ../lab3/include/utils.h)

#target_link_libraries(lab3_test PRIVATE GTest::GTest)
#target_include_directories(lab3_test PRIVATE ../lab3/include)

#add_test(lab3_test lab3_test)
cmake_minimum_required(VERSION 3.10)
project(lab5)

# Установка стандарта C
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Установка стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Поиск ZeroMQ через pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(ZMQ REQUIRED libzmq)

# Добавление директорий с заголовочными файлами
include_directories(include ${ZMQ_INCLUDE_DIRS})

# Создание библиотек
add_library(dict_lib src/dict.c)
add_library(message_lib src/message.c)
add_library(pending_ops_lib src/pending_ops.c)
add_library(node_manager_lib src/node_manager.c)

# Исполняемые файлы
add_executable(controller controller.c)
add_executable(worker worker.c)

# Линковка библиотек
target_link_libraries(controller
    PRIVATE
    ${ZMQ_LIBRARIES}
    message_lib
    pending_ops_lib
    node_manager_lib
)

target_link_libraries(worker
    PRIVATE
    ${ZMQ_LIBRARIES}
    dict_lib
    message_lib
)

# Загружаем Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
FetchContent_MakeAvailable(googletest)

# Создание тестов
enable_testing()

add_executable(test5
    tests/test5_controller.cpp
)

target_link_libraries(test5
    PRIVATE
    gtest_main
    gtest
    ${ZMQ_LIBRARIES}
    dict_lib
    message_lib
    pending_ops_lib
    node_manager_lib
)

include(GoogleTest)
gtest_discover_tests(test5)
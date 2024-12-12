cmake_minimum_required(VERSION 3.10)
project(CP)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add include directory
include_directories(${PROJECT_SOURCE_DIR}/include)

# Create list of source files
set(SOURCES
    src/best_fit_allocator.c
    src/power2_allocator.c
    src/main.c
)

# Create executable
add_executable(allocator_test ${SOURCES})



# Optional: Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) 
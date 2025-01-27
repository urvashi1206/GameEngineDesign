cmake_minimum_required(VERSION 3.15)
project(ECSProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Where to put built libraries and executables
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Find Raylib using the vcpkg integration
find_package(raylib CONFIG REQUIRED)

add_executable(ECSProject
    src/main.cpp
    src/Systems.cpp
)

target_include_directories(ECSProject PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

# Link Raylib (vcpkg sets up the correct paths automatically)
target_link_libraries(ECSProject PRIVATE raylib)

cmake_minimum_required(VERSION 3.15)

# Include the vcpkg toolchain file
set(CMAKE_TOOLCHAIN_FILE "C:/Users/Owner/Documents/GameEngineDesign/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

project(ECSProject)

# Set the C++ standard for the target
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directories for built libraries and executables
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add the executable
add_executable(ECSProject
    src/main.cpp
    src/Systems.cpp
)

# Include directories
target_include_directories(ECSProject PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Find Raylib using the vcpkg integration
find_package(raylib REQUIRED CONFIG)

# Link Raylib (vcpkg sets up the correct paths automatically)
target_link_libraries(ECSProject PRIVATE raylib)

# Set the C++ standard for the target
target_compile_features(ECSProject PUBLIC cxx_std_17)
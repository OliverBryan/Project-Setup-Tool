cmake_minimum_required(VERSION 3.19)

project(setup VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(setup ./src/Main.cpp)

target_include_directories(setup PRIVATE "${PROJECT_BINARY_DIR}/src")
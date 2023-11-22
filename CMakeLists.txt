cmake_minimum_required(VERSION 3.1...3.24)

project(hft-dev-exercise)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_library(hft-lib INTERFACE)
target_include_directories(hft-lib INTERFACE include)

add_executable(client src/main.cpp)
target_link_libraries(client hft-lib)

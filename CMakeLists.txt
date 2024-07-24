cmake_minimum_required(VERSION 3.20)
project(simulator)

file(GLOB_RECURSE sources "src/*.cpp")

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

add_executable(simulator ${sources})

add_executable(alu demo/alu.cpp)

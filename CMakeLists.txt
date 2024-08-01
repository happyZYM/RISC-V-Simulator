cmake_minimum_required(VERSION 3.20)
project(simulator)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()
file(GLOB_RECURSE sources "src/*.cpp")

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

# add_executable(simulator ${sources})

add_executable(alu demo/alu.cpp)

# For debug build
add_executable(modules demo/modules.cpp)
target_compile_definitions(modules PRIVATE _DEBUG)

add_executable(interpreter src/rv32iinterpreter.cpp)
set_target_properties(interpreter
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

add_executable(code src/main.cpp)
set_target_properties(code
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# Enable sanitizer for debug builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set(SANITIZER_FLAGS "-fsanitize=address,undefined -fno-omit-frame-pointer -D _DEBUG")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${SANITIZER_FLAGS}")
        set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${SANITIZER_FLAGS}")
    endif()
endif()

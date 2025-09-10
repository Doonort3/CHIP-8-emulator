cmake_minimum_required(VERSION 3.31)
project(CHIP_8_emulator)

set(CMAKE_CXX_STANDARD 14)
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(CHIP_8_emulator
        src/cpu.cpp
        src/cpu.h
        src/main.cpp)

target_link_libraries(CHIP_8_emulator ${SDL2_LIBRARIES})

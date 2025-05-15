cmake_minimum_required(VERSION 3.10)
project(distributed_mmio C)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

add_library(distributed_mmio STATIC ${SRC_FILES})

target_include_directories(distributed_mmio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

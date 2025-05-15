cmake_minimum_required(VERSION 3.10)
project(distributed_mmio LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(distributed_mmio STATIC ${SRC_FILES})

target_include_directories(distributed_mmio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

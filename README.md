# Distributed MMIO

Lightweight `C/C++` library for local or distributed reading of Matrix Market files.

## Including "Distributed MMIO" with CMake

Simply add to your `CMakeLists.txt` the following:

```cmake
include(FetchContent)

FetchContent_Declare(
  distributed_mmio
  GIT_REPOSITORY https://github.com/HicrestLaboratory/distributed_mmio.git
  GIT_TAG        main # or a specific tag/commit
)

FetchContent_MakeAvailable(distributed_mmio)

target_link_libraries(my_target PRIVATE distributed_mmio)
```
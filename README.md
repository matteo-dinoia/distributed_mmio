# Distributed MMIO

Lightweight Templated `C++` library for local or distributed reading of Matrix Market files.

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

## Usage Examples

### Non-distributed Matrix Market File CSR Read 

```c++
#include "../distributed_mmio/include/mmio.h"
// ...
CSR_local<uint32_t, float> *csr_matrix = Distr_MMIO_CSR_local_read<uint32_t, float>("path/to/mtx_file");
```

# Binary Matrix Market (.bmtx)

TODO
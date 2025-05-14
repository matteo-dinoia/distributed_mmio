cmake_minimum_required(VERSION 3.10)
project(distributed_mmio LANGUAGES C)

# Define the library target
add_library(distributed_mmio STATIC src/mmio.c)

# Specify include directories for the library
target_include_directories(distributed_mmio
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Optionally set C standard
# set_target_properties(distributed_mmio PROPERTIES
#     C_STANDARD 99
#     C_STANDARD_REQUIRED YES
# )

# Installation rules
include(GNUInstallDirs)

install(TARGETS distributed_mmio
        EXPORT distributed_mmioTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Exporting the targets
install(EXPORT distributed_mmioTargets
        FILE distributed_mmioTargets.cmake
        NAMESPACE distributed_mmio::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/distributed_mmio)

# Create a Config file for find_package
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/distributed_mmioConfigVersion.cmake"
  VERSION 1.0
  COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/distributed_mmioConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/distributed_mmioConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/distributed_mmio
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/distributed_mmioConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/distributed_mmioConfigVersion.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/distributed_mmio
)

cmake_minimum_required(VERSION 3.8)

project("OpenCL-example")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Require pkg-config tool for finding libraries.
find_package(PkgConfig REQUIRED)
# Find  ocl-icd library with pkg-config tool from the system libraries. This line will add variable PkgConfig::OPENCL.
pkg_check_modules(OPENCL REQUIRED IMPORTED_TARGET OpenCL)

# Add main executable and it's source files.
add_executable(main main.c example.cl)

# Link main executable with ocl-icd library.
target_link_libraries(main PUBLIC PkgConfig::OPENCL)

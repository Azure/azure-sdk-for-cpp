# Generic Service for Samples

This is a helper library for samples that provides a generic service client library.

## How to use

Include the header from the CMake project using `target_include_directories`. For example, use

```cmake
# NOTE: Use shared-code only within .cpp files. DO NEVER consume the shared-code from header files.
target_include_directories(
  cmake-target-name
    PRIVATE
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../path/to/samples/inc>
)
```

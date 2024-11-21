# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# Configure CMake project.
cmake_minimum_required (VERSION 3.13)
project(azure-perf-unit-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(DEFINED ENV{VCPKG-AZURE-CORE-CPP})
   find_package(azure-core-cpp $ENV{VCPKG-AZURE-CORE-CPP} EXACT)
   add_compile_definitions(VCPKG_CORE_VERSION="$ENV{VCPKG-AZURE-CORE-CPP}")
else()
    add_compile_definitions(VCPKG_CORE_VERSION="source")
endif()

# Unit tests
include(GoogleTest)

add_executable (
  azure-perf-unit-test
    src/random_stream_test.cpp
)

if (MSVC)
  # Disable warnings
  # - C6326: Google comparisons 
  target_compile_options(azure-perf-unit-test PUBLIC /wd6326)
endif()

target_link_libraries(azure-perf-unit-test PRIVATE azure-perf gtest gtest_main)

target_compile_definitions(azure-perf-unit-test PRIVATE _azure_BUILDING_TESTS)

gtest_discover_tests(azure-perf-unit-test
     WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
     TEST_PREFIX azure-perf-unittest.
     NO_PRETTY_TYPES
     NO_PRETTY_VALUES
     DISCOVERY_TIMEOUT 600)


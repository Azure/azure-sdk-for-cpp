# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

# Configure CMake project.
cmake_minimum_required (VERSION 3.13)
project(azure-perf-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(
  AZURE_PERF_TEST_HEADER
  inc/azure/perf/test/curl_http_client_get_test.hpp
  inc/azure/perf/test/delay_test.hpp
  inc/azure/perf/test/exception_test.hpp
  inc/azure/perf/test/extended_options_test.hpp
  inc/azure/perf/test/http_client_get_test.hpp
  inc/azure/perf/test/no_op_test.hpp
  inc/azure/perf/test/win_http_client_get_test.hpp
)

set(
  AZURE_PERF_TEST_SOURCE
    src/perf_test.cpp
)

# Name the binary to be created.
add_executable (
  azure-perf-test
     ${AZURE_PERF_TEST_HEADER} ${AZURE_PERF_TEST_SOURCE}
)

create_map_file(azure-perf-test azure-perf-test.map)

# Include the headers from the project.
target_include_directories(
  azure-perf-test
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
)

if(DEFINED ENV{VCPKG-AZURE-CORE-CPP})
   find_package(azure-core-cpp $ENV{VCPKG-AZURE-CORE-CPP} EXACT)
   add_compile_definitions(VCPKG_CORE_VERSION="$ENV{VCPKG-AZURE-CORE-CPP}")
else()
    add_compile_definitions(VCPKG_CORE_VERSION="source")
endif()


# link the `azure-perf` lib together with any other library which will be used for the tests. 
target_link_libraries(azure-perf-test PRIVATE Azure::azure-core azure-perf)
# Make sure the project will appear in the test folder for Visual Studio CMake view
set_target_properties(azure-perf-test PROPERTIES FOLDER "Tests/Core")

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

gtest_discover_tests(azure-perf-unit-test
     TEST_PREFIX azure-perf-unittest.
     NO_PRETTY_TYPES
     NO_PRETTY_VALUES
     DISCOVERY_TIMEOUT 600 )


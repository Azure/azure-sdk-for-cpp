# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules")

# Compile Options
option(WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)
option(BUILD_TRANSPORT_CURL "Build an HTTP transport implementation with CURL" OFF)
option(BUILD_TRANSPORT_WINHTTP "Build an HTTP transport implementation with WIN HTTP" OFF)
option(BUILD_TESTING "Build test cases" OFF)
option(BUILD_CODE_COVERAGE "Build gcov targets for HTML and XML reports. Requires debug build and BUILD_TESTING" OFF)
option(BUILD_DOCUMENTATION "Create HTML based API documentation (requires Doxygen)" OFF)
option(RUN_LONG_UNIT_TESTS "Tests that takes more than 5 minutes to complete. No effect if BUILD_TESTING is OFF" OFF)
option(BUILD_STORAGE_SAMPLES "Build sample application for Azure Storage clients" OFF)

include(DefineTransportAdapter)

# VCPKG Integration
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
elseif(DEFINED ENV{VCPKG_INSTALLATION_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_INSTALLATION_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif()
if(DEFINED ENV{VCPKG_DEFAULT_TRIPLET} AND NOT DEFINED VCPKG_TARGET_TRIPLET)
  set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_DEFAULT_TRIPLET}" CACHE STRING "")
endif()

# Define WINDOWS of POSIX for specific code implementations like FileStream
include(DefinePlatform)

# Project definition
project(az LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(BUILD_TESTING)
    # define a symbol that enables some test hooks in code
    add_compile_definitions(TESTING_BUILD)
    if(RUN_LONG_UNIT_TESTS)
      add_compile_definitions(RUN_LONG_UNIT_TESTS)
    endif()
    
    # tests
    include(AddGoogleTest)
    enable_testing ()
    add_subdirectory(sdk/core/azure-core/test/ut)
endif()

# compiler warning flags globally
include(global_compile_options)

# Documentation automation function
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules/doxygen_common.cmake)

# Functions for library versions
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules/az_version.cmake)

# sub-projects
# add_subdirectory(sdk/core/performance-stress)
add_subdirectory(sdk/core/azure-core)

if(BUILD_TESTING)
    add_subdirectory(sdk/core/azure-core/test/e2e)
endif()

add_subdirectory(sdk/identity/azure-identity)

add_subdirectory(sdk/storage)
add_subdirectory(sdk/template/azure-template)

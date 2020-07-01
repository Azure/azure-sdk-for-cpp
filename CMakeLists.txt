# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.15)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake-modules")

# Compile Options
option(WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)
option(BUILD_CURL_TRANSPORT "Build internal http transport implementation with CURL for HTTP Pipeline" OFF)
option(BUILD_TESTING "Build test cases" OFF)
option(BUILD_DOCUMENTATION "Create HTML based API documentation (requires Doxygen)" OFF)

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

# Project definition
project(az LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(BUILD_TESTING)
    # tests
    include(AddGoogleTest)
    enable_testing ()
    add_subdirectory(sdk/core/azure-core/test)
endif()

# compiler warning flags globally
include(global_compile_options)

# Documentation automation function
include(${CMAKE_SOURCE_DIR}/cmake-modules/doxygen_common.cmake)

# sub-projects
add_subdirectory(sdk/core/azure-core)
add_subdirectory(sdk/samples/http_client/curl) # will work only if BUILD_CURL_TRANSPORT=ON
#add_subdirectory(sdk/storage)

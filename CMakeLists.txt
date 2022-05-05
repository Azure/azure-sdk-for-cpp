# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules")

# Variable to indicate whether the entire SDK is being built, as opposed to an individual library.
set(AZ_ALL_LIBRARIES ON)

# Compile Options
include(FolderList)
SetGlobalOptions()

if(FETCH_SOURCE_DEPS)
  message(FATAL_ERROR, "FETCH_SOURCE_DEPS flag not for global use, instead use when building individual components")
  return()
endif()

include(AzureTransportAdapters)
include(AzureVcpkg)
include(AzureBuildTargetForCI)

az_vcpkg_integrate()

# Project definition
project(azure-sdk LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(MSVC_USE_STATIC_CRT AND MSVC)
  # 1. More about static/shared CRT:
  # https://docs.microsoft.com/cpp/build/reference/md-mt-ld-use-run-time-library?view=msvc-160
  #
  # 2. MSVC_USE_STATIC_CRT build flag approach is used/inspired by libcurl
  # (https://github.com/curl/curl/blob/master/CMakeLists.txt) and some other projects.
  #
  # 3. GTest would emit the following warning:
  # warning LNK4098: defaultlib 'LIBCMTD' conflicts with use of other libs; use /NODEFAULTLIB:library
  # AddGoogleTest.cmake uses gtest_force_shared_crt
  # (see https://github.com/google/googletest/blob/master/googletest/README.md),
  # which respects linker settings that we set below, and our settings below are all in sync.
  #
  # 4. Sometimes, the following approach is recommended instead:
  # +-----------------------------------------------------------------------------------+
  # | # Use the static runtime libraries when building statically                       |
  # | # for consistency with the vcpkg arch-windows-static triplets:                    |
  # | cmake_policy(SET CMP0091 NEW)                                                     |
  # | # see https://cmake.org/cmake/help/v3.15/variable/CMAKE_MSVC_RUNTIME_LIBRARY.html |
  # | if (NOT DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)                                       |
  # |   set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")         |
  # | endif()                                                                           |
  # +-----------------------------------------------------------------------------------+
  # However, it only works when cmake installed is 3.15+;
  # we have to require a minimum of 3.13.
  #
  # 5. We "replace with empty string" (i.e. remove) first, then add, so that '/MT'
  # will be present (and present once) even if '/MD' was not.

  string(REGEX REPLACE "/MD" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MT")
  
  string(REGEX REPLACE "/MD" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")

  string(REGEX REPLACE "/MD" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MT")

  string(REGEX REPLACE "/MD" "" CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL}")
  set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /MT")
  
  string(REGEX REPLACE "/MDd" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()

if(BUILD_TESTING)
  include(AddGoogleTest)
  enable_testing ()  
endif()

# compiler warning flags globally
include(AzureGlobalCompileOptions)

# Add create_map_file function
include(CreateMapFile)

# Documentation automation function
include(AzureDoxygen)

# Functions for library versions
include(AzureVersion)

if(BUILD_SAMPLES)
  add_subdirectory(samples/helpers/get-env)
endif()

# sub-projects
add_subdirectory(sdk/core)
add_subdirectory(sdk/attestation)
add_subdirectory(sdk/identity)
add_subdirectory(sdk/keyvault)
add_subdirectory(sdk/storage)
add_subdirectory(sdk/template)

if(BUILD_SAMPLES)
  add_subdirectory(samples/integration/vcpkg-all-smoke)
endif()

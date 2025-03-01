# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# cspell: words fsanitize

cmake_minimum_required (VERSION 3.13)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules")

# Disable Rust based APIs in C++ SDK.
set(DISABLE_RUST_IN_BUILD OFF CACHE BOOL "Disable Rust based APIs in package")
set(DISABLE_AMQP OFF CACHE BOOL "Disable AMQP based functionality")
set(DISABLE_AZURE_CORE_OPENTELEMETRY OFF CACHE BOOL "Disable OpenTelemetry based functionality")


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
project(azure-sdk LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Enable a build configuration to disable features when building for Windows Store.
message(STATUS "CMAKE_SYSTEM_NAME=" ${CMAKE_SYSTEM_NAME} ";")
message(STATUS "VCPKG_TARGET_TRIPLET=" ${VCPKG_TARGET_TRIPLET} ";")

# In the CI pipeline, for <reasons>, CMAKE_SYSTEM_NAME is set to "Windows" even when building for Windows Store.
# use the CMAKE triplet to detect the windows store build.
if ((${VCPKG_TARGET_TRIPLET} MATCHES ".*-uwp"))
  set(BUILD_WINDOWS_UWP True)
  message(STATUS "Building for Windows Store, CMAKE_SYSTEM_NAME=" ${CMAKE_SYSTEM_NAME})
endif()

if (BUILD_WINDOWS_UWP)
  if (DISABLE_RUST_IN_BUILD)
    message(STATUS "uAMQP is incompatible with UWP builds.")
    set(DISABLE_AMQP ON)
  endif()
  message(STATUS "opentelemetry-cpp package is incompatible with UWP builds.")
  set(DISABLE_AZURE_CORE_OPENTELEMETRY ON)
endif()

# For MSVC, ensure that the compiler is operating in strict compliance mode.
if (MSVC)
# See also: https://learn.microsoft.com/cpp/build/reference/permissive-standards-conformance
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive-")
endif()

if(MSVC_USE_STATIC_CRT AND MSVC)
  # 1. More about static/shared CRT:
  # https://learn.microsoft.com/cpp/build/reference/md-mt-ld-use-run-time-library?view=msvc-160
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

  message(STATUS "Configuring Static Runtime Library.")
  if(${CMAKE_CXX_FLAGS} MATCHES ".*/MD.*")
    string(REGEX REPLACE "/MD" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MT")
  endif()

  if(${CMAKE_CXX_FLAGS_RELEASE} MATCHES ".*/MD.*")
    string(REGEX REPLACE "/MD" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
  endif()

  if(${CMAKE_CXX_FLAGS_RELWITHDEBINFO} MATCHES ".*/MD.*")
    string(REGEX REPLACE "/MD" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MT")
  endif()

  if(${CMAKE_CXX_FLAGS_MINSIZEREL} MATCHES ".*/MD.*")
    string(REGEX REPLACE "/MD" "" CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL}")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /MT")
  endif()
  
  if(${CMAKE_CXX_FLAGS_DEBUG} MATCHES ".*/MD.*")
    string(REGEX REPLACE "/MDd" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
  endif()

  # Now make the same adjustments for .C files as was done for .CPP files.
  if(${CMAKE_C_FLAGS} MATCHES ".*/MD.*")
    string(REGEX REPLACE "/MD" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MT")
  endif()
  
  if(${CMAKE_C_FLAGS_RELEASE} MATCHES ".*/MD.*")
    string(REGEX REPLACE "/MD" "" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
  endif()

  if(${CMAKE_C_FLAGS_RELWITHDEBINFO} MATCHES ".*/MD.*")
    string(REGEX REPLACE "/MD" "" CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} /MT")
  endif()

  if(${CMAKE_C_FLAGS_MINSIZEREL} MATCHES ".*/MD.*")
    string(REGEX REPLACE "/MD" "" CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL}")
    set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} /MT")
  endif()
  
  if(${CMAKE_C_FLAGS_DEBUG} MATCHES ".*/MD.*")
    string(REGEX REPLACE "/MDd" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
  endif()
endif()

# If the cmake presets enable Address Sanitizer, enable it for the project.
if (ENABLE_ADDRESS_SANITIZER)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
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
  add_subdirectory(samples/helpers/service)
endif()

# sub-projects
add_subdirectory(sdk/core)
add_subdirectory(sdk/appconfiguration)
add_subdirectory(sdk/attestation)
# AMQP doesn't work for UWP yet, and eventhubs depends on AMQP, so we cannot include eventhubs on UWP.
if (NOT DISABLE_AMQP)
    add_subdirectory(sdk/eventhubs)
endif()
add_subdirectory(sdk/identity)
add_subdirectory(sdk/keyvault)
add_subdirectory(sdk/storage)
add_subdirectory(sdk/template)
add_subdirectory(sdk/tables)

if(BUILD_SAMPLES)
  add_subdirectory(samples/integration/vcpkg-all-smoke)
endif()

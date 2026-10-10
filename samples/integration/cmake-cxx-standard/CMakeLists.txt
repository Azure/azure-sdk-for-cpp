# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.13)
project(cmake-cxx-standard LANGUAGES CXX)

option(USE_INSTALLED_CORE "Test an installed azure-core package" OFF)
set(EXPECTED_CXX_VERSION 201402 CACHE STRING "Minimum expected __cplusplus value")
set(caller_cxx_standard "${CMAKE_CXX_STANDARD}")
get_filename_component(sdk_root "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)

if(USE_INSTALLED_CORE)
  find_package(azure-core-cpp CONFIG REQUIRED)
else()
  add_subdirectory("${sdk_root}/sdk/core/azure-core" azure-core)
  add_subdirectory("${sdk_root}/samples/helpers/get-env" get-env)
  foreach(target azure-core get-env-helper)
    get_target_property(required ${target} CXX_STANDARD_REQUIRED)
    if(NOT required)
      message(FATAL_ERROR "${target} must require the selected C++ standard")
    endif()
    if(caller_cxx_standard GREATER_EQUAL 14)
      get_target_property(standard ${target} CXX_STANDARD)
      if(NOT "${standard}" STREQUAL "${caller_cxx_standard}")
        message(FATAL_ERROR "${target} changed the caller-selected C++ standard")
      endif()
    endif()
  endforeach()
endif()

if(NOT "${CMAKE_CXX_STANDARD}" STREQUAL "${caller_cxx_standard}")
  message(FATAL_ERROR "The SDK changed the caller's CMAKE_CXX_STANDARD")
endif()

get_target_property(features Azure::azure-core INTERFACE_COMPILE_FEATURES)
if(NOT "cxx_std_14" IN_LIST features)
  message(FATAL_ERROR "azure-core must publish its C++14 requirement")
endif()

# Deliberately omit compile features here to test the SDK's transitive requirement.
add_executable(cxx-standard-consumer main.cpp)
set_target_properties(cxx-standard-consumer PROPERTIES CXX_STANDARD_REQUIRED ON)
target_compile_definitions(cxx-standard-consumer PRIVATE EXPECTED_CXX_VERSION=${EXPECTED_CXX_VERSION})
target_link_libraries(cxx-standard-consumer PRIVATE Azure::azure-core)

enable_testing()
add_test(NAME cxx-standard-consumer COMMAND cxx-standard-consumer)

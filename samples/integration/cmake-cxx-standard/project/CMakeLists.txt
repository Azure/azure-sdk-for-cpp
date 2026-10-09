# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.13)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../../cmake-modules")
include(AzureVcpkg)
az_vcpkg_integrate()
project(cxx-standard-regression LANGUAGES CXX)

if(DEFINED TEST_PARENT_STANDARD)
  set(CMAKE_CXX_STANDARD "${TEST_PARENT_STANDARD}")
endif()

set(DISABLE_AMQP ON)
set(DISABLE_AZURE_CORE_OPENTELEMETRY ON)
set(BUILD_TESTING OFF)
set(BUILD_SAMPLES OFF)
set(BUILD_PERFORMANCE_TESTS OFF)
set(AZ_ALL_LIBRARIES ON)
set(CMAKE_EXPORT_NO_PACKAGE_REGISTRY ON)

if(TEST_SURFACE STREQUAL "service")
  add_subdirectory("../../../../sdk/core" core)
  add_subdirectory("../../../../sdk/identity" identity)
elseif(TEST_SURFACE STREQUAL "package")
  add_subdirectory("../../../../sdk/core/azure-core" core)
  add_subdirectory("../../../../sdk/identity/azure-identity" identity)
else()
  message(FATAL_ERROR "TEST_SURFACE must be service or package.")
endif()

foreach(target azure-core azure-identity)
  get_target_property(standard ${target} CXX_STANDARD)
  get_target_property(required ${target} CXX_STANDARD_REQUIRED)
  if(NOT standard STREQUAL TEST_EXPECTED_STANDARD OR NOT required)
    message(FATAL_ERROR
      "${target}: expected required C++${TEST_EXPECTED_STANDARD}, got C++${standard}, required=${required}")
  endif()
  message(STATUS "${target}: required C++${standard}")
endforeach()

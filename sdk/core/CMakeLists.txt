# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

project (azure-core LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_subdirectory(azure-core)

if (NOT BUILD_WINDOWS_UWP)
  message(STATUS "Including AMQP library")
  add_subdirectory(azure-core-amqp)
endif()

if (NOT DISABLE_AZURE_CORE_OPENTELEMETRY)
  add_subdirectory(azure-core-tracing-opentelemetry)
endif()

#if (BUILD_PERFORMANCE_TESTS) 
  add_subdirectory(perf)
#endif()

# Playback & record framework (a.k.a. azure-core-test)
if (BUILD_TESTING)
  add_subdirectory(azure-core-test)
endif()


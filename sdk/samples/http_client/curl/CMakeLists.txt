# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

if (BUILD_CURL_TRANSPORT)

cmake_minimum_required (VERSION 3.12)
set(TARGET_NAME "azure_core_with_curl")
set(TARGET_NAME_STREAM "azure_core_with_curl_stream")

project(${TARGET_NAME} LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (MSVC)
  add_compile_definitions(Windows)
endif (UNIX)
if (UNIX)
  add_compile_definitions(Posix)
endif (UNIX)

add_executable (
     ${TARGET_NAME}
     src/azure_core_with_curl_bodyBuffer 
)

add_executable (
     ${TARGET_NAME_STREAM}
     src/azure_core_with_curl_bodyStream
)

target_link_libraries(${TARGET_NAME} PRIVATE azure-core)
target_link_libraries(${TARGET_NAME_STREAM} PRIVATE azure-core)

endif()

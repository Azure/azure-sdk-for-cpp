# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

if (BUILD_CURL_TRANSPORT)

cmake_minimum_required (VERSION 3.12)
set(TARGET_NAME "azure_core_with_curl")

project(${TARGET_NAME} LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
     ${TARGET_NAME}
     src/azure_core_with_curl 
)

target_link_libraries(${TARGET_NAME} PRIVATE azure-core azure-transport-curl)

endif()

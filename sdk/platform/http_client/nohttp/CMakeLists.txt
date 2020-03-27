# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.12)
set(TARGET_NAME "azure-transport-nothhp")

project(${TARGET_NAME} LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library (
  ${TARGET_NAME} STATIC
  src/azure_transport_nohttp.cpp
)

target_link_libraries(${TARGET_NAME} PRIVATE az_core)

# make sure that users can consume the project as a library.
add_library (az::nohttp ALIAS ${TARGET_NAME})

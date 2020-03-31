# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

if (BUILD_CURL_TRANSPORT)

cmake_minimum_required (VERSION 3.12)
set(TARGET_NAME "azure-transport-curl")

project(${TARGET_NAME} LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(CURL CONFIG)
if(NOT CURL_FOUND)
  find_package(CURL REQUIRED)
endif()

add_library (
  ${TARGET_NAME} STATIC
  src/azure_transport_curl
  src/curl_client
  )

target_include_directories (${TARGET_NAME} PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc> $<INSTALL_INTERFACE:include/azure_curl>)

# make sure that users can consume the project as a library.
add_library (azure::curl ALIAS ${TARGET_NAME})

target_include_directories(${TARGET_NAME} PUBLIC ${CURL_INCLUDE_DIRS})
target_link_libraries(${TARGET_NAME} PRIVATE azure-core CURL::libcurl)

endif()

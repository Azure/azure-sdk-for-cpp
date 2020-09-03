# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.12)
set(TARGET_NAME "azure-core")
project(${TARGET_NAME} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CURL_MIN_REQUIRED_VERSION 7.4)
find_package(CURL ${CURL_MIN_REQUIRED_VERSION} CONFIG)
if(NOT CURL_FOUND)
  find_package(CURL ${CURL_MIN_REQUIRED_VERSION} REQUIRED)
endif()

add_library (
  ${TARGET_NAME}
  src/context.cpp
  src/credentials/credentials.cpp
  src/credentials/policy/policies.cpp
  src/http/body_stream.cpp
  src/http/curl/curl.cpp
  src/http/logging_policy.cpp
  src/http/policy.cpp
  src/http/request.cpp
  src/http/raw_response.cpp
  src/http/retry_policy.cpp
  src/http/transport_policy.cpp
  src/http/telemetry_policy.cpp
  src/http/url.cpp
  src/http/winhttp/win_http_transport.cpp
  src/logging/logging.cpp
  src/strings.cpp
  src/version.cpp)

target_include_directories (${TARGET_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc> $<INSTALL_INTERFACE:include/az_core>)

# make sure that users can consume the project as a library.
add_library (Azure::Core ALIAS ${TARGET_NAME})

target_include_directories(${TARGET_NAME} PUBLIC ${CURL_INCLUDE_DIRS})
target_link_libraries(${TARGET_NAME} PRIVATE CURL::libcurl)

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/inc/azure/core/version.hpp")
generate_documentation(azure-core ${AZ_LIBRARY_VERSION})

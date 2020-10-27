# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)
set(TARGET_NAME "azure-core")
project(${TARGET_NAME} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CURL_MIN_REQUIRED_VERSION 7.4)
find_package(CURL ${CURL_MIN_REQUIRED_VERSION} CONFIG)
if(NOT CURL_FOUND)
  find_package(CURL ${CURL_MIN_REQUIRED_VERSION} REQUIRED)
endif()

if(BUILD_TRANSPORT_CURL)
  SET(CURL_TRANSPORT_ADAPTER_SRC src/http/curl/curl.cpp)
endif()
if(BUILD_TRANSPORT_WINHTTP)
  SET(WIN_TRANSPORT_ADAPTER_SRC src/http/winhttp/win_http_transport.cpp)
endif()

# CodeCoverage will validate requirements
include(CodeCoverage)

add_library (
  ${TARGET_NAME}
  src/context.cpp
  src/credentials.cpp
  src/datetime.cpp
  src/http/body_stream.cpp
  ${CURL_TRANSPORT_ADAPTER_SRC}
  src/http/http.cpp
  src/http/logging_policy.cpp
  src/http/policy.cpp
  src/http/raw_response.cpp
  src/http/request.cpp
  src/http/retry_policy.cpp
  src/http/telemetry_policy.cpp
  src/http/transport_policy.cpp
  src/http/url.cpp
  ${BUILD_WIN_TRANSPORT}
  src/logging/logging.cpp
  src/strings.cpp
  src/version.cpp
  )

target_include_directories (${TARGET_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc> $<INSTALL_INTERFACE:include/az_core>)

# make sure that users can consume the project as a library.
add_library (Azure::Core ALIAS ${TARGET_NAME})

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(core ${TARGET_NAME} "${TARGET_NAME}-test")

target_include_directories(${TARGET_NAME} PUBLIC ${CURL_INCLUDE_DIRS})
target_link_libraries(${TARGET_NAME} PRIVATE CURL::libcurl)

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/inc/azure/core/version.hpp")
generate_documentation(azure-core ${AZ_LIBRARY_VERSION})

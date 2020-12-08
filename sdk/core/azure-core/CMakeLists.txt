# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)
set(azure-core)
project(azure-core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

# min version for `CURLSSLOPT_NO_REVOKE`
# https://curl.haxx.se/libcurl/c/CURLOPT_SSL_OPTIONS.html
set(CURL_MIN_REQUIRED_VERSION 7.44)
find_package(CURL ${CURL_MIN_REQUIRED_VERSION} CONFIG)
if(NOT CURL_FOUND)
  find_package(CURL ${CURL_MIN_REQUIRED_VERSION} REQUIRED)
endif()
message("Libcurl version ${CURL_VERSION_STRING}")

# Storage requires 3.6.0 version for using `contains` feature
set(NLOHMANN_JSON_MIN_REQUIRED_VERSION 3.6.0)
# Try to find the config cmake file. Tipically for when the integration is made at CMake level
# (CMake project available with a configuration file to consume)
find_package(nlohmann_json ${NLOHMANN_JSON_MIN_REQUIRED_VERSION} CONFIG)
if(NOT nlohmann_json_FOUND)
  # If CMake project/config is not found, last option is to find the library installed. The integration is done by the linker directly. 
  find_package(nlohmann_json ${NLOHMANN_JSON_MIN_REQUIRED_VERSION} REQUIRED)
endif()
message("nlohmann json version ${nlohmann_json_VERSION}")

if(BUILD_TRANSPORT_CURL)
  SET(CURL_TRANSPORT_ADAPTER_SRC src/http/curl/curl.cpp)
endif()
if(BUILD_TRANSPORT_WINHTTP)
  SET(WIN_TRANSPORT_ADAPTER_SRC src/http/winhttp/win_http_transport.cpp)
endif()

# CodeCoverage will validate requirements
include(CodeCoverage)

add_library (
  azure-core
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

target_include_directories(
  azure-core
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include/az_core>
)

# make sure that users can consume the project as a library.
add_library (Azure::Core ALIAS azure-core)

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(core azure-core azure-core-test)

target_include_directories(azure-core PUBLIC ${CURL_INCLUDE_DIRS})
target_link_libraries(azure-core PRIVATE CURL::libcurl Threads::Threads)
target_link_libraries(azure-core INTERFACE nlohmann_json::nlohmann_json)

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/inc/azure/core/version.hpp")
generate_documentation(azure-core ${AZ_LIBRARY_VERSION})

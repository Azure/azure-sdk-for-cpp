# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)
set(TARGET_NAME "azure-core")
project(${TARGET_NAME} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake-modules")

include(az_vcpkg)
include(az_version)
include(CodeCoverage)
include(DefineTransportAdapter)
include(doxygen_common)
include(global_compile_options)

az_vcpkg_integrate()

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
# Try to find the config cmake file. Typically for when the integration is made at CMake level
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

target_include_directories(
  ${TARGET_NAME}
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
  )

# make sure that users can consume the project as a library.
add_library(Azure::${TARGET_NAME} ALIAS ${TARGET_NAME})

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(core ${TARGET_NAME} "${TARGET_NAME}-test")

# Needs Azure::azure-storage-blobs be public as long as we #include<curl.h> in public headers.
target_include_directories(${TARGET_NAME} PUBLIC ${CURL_INCLUDE_DIRS})
target_include_directories(${TARGET_NAME} INTERFACE ${nlohmann_json_INCLUDE_DIRS})
target_link_libraries(${TARGET_NAME} INTERFACE CURL::libcurl Threads::Threads nlohmann_json::nlohmann_json)

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/inc/azure/core/version.hpp")
generate_documentation(${TARGET_NAME} ${AZ_LIBRARY_VERSION})

az_vcpkg_export()

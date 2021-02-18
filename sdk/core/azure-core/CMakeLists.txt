# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)
project(azure-core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake-modules")

include(AzureVcpkg)
include(AzureVersion)
include(AzureCodeCoverage)
include(AzureTransportAdapters)
include(AzureDoxygen)
include(AzureGlobalCompileOptions)

az_vcpkg_integrate()

find_package(Threads REQUIRED)

if(BUILD_TRANSPORT_CURL)
  # min version for `CURLSSLOPT_NO_REVOKE`
  # https://curl.haxx.se/libcurl/c/CURLOPT_SSL_OPTIONS.html
  set(CURL_MIN_REQUIRED_VERSION 7.44)
  find_package(CURL ${CURL_MIN_REQUIRED_VERSION} CONFIG QUIET)
  if(NOT CURL_FOUND)
    find_package(CURL ${CURL_MIN_REQUIRED_VERSION} REQUIRED)
  endif()
  message("Libcurl version ${CURL_VERSION_STRING}")
endif()

if(BUILD_TRANSPORT_CURL)
  SET(CURL_TRANSPORT_ADAPTER_SRC src/http/curl/curl.cpp)
  SET(CURL_TRANSPORT_ADAPTER_INC inc/azure/core/http/curl/curl.hpp)
endif()
if(BUILD_TRANSPORT_WINHTTP)
  SET(WIN_TRANSPORT_ADAPTER_SRC src/http/winhttp/win_http_transport.cpp)
  SET(WIN_TRANSPORT_ADAPTER_INC inc/azure/core/http/winhttp/win_http_client.hpp)
endif()

set(
  AZURE_CORE_HEADER
    ${CURL_TRANSPORT_ADAPTER_INC}
    ${WIN_TRANSPORT_ADAPTER_INC}
    inc/azure/core/cryptography/hash.hpp
    inc/azure/core/http/body_stream.hpp
    inc/azure/core/http/http.hpp
    inc/azure/core/http/policy.hpp
    inc/azure/core/http/transport.hpp
    inc/azure/core/internal/contract.hpp
    inc/azure/core/internal/http/pipeline.hpp
    inc/azure/core/internal/json_serializable.hpp
    inc/azure/core/internal/json.hpp
    inc/azure/core/internal/log.hpp
    inc/azure/core/internal/strings.hpp
    inc/azure/core/logging/logging.hpp
    inc/azure/core/base64.hpp
    inc/azure/core/context.hpp
    inc/azure/core/credentials.hpp
    inc/azure/core/datetime.hpp
    inc/azure/core/dll_import_export.hpp
    inc/azure/core/etag.hpp
    inc/azure/core/exception.hpp
    inc/azure/core/match_conditions.hpp
    inc/azure/core/modified_conditions.hpp
    inc/azure/core/nullable.hpp
    inc/azure/core/operation.hpp
    inc/azure/core/operation_status.hpp
    inc/azure/core/platform.hpp
    inc/azure/core/response.hpp
    inc/azure/core/uuid.hpp
    inc/azure/core/version.hpp
    inc/azure/core.hpp
)

set(
  AZURE_CORE_SOURCE
    ${CURL_TRANSPORT_ADAPTER_SRC}
    ${WIN_TRANSPORT_ADAPTER_SRC}
    src/cryptography/md5.cpp
    src/http/bearer_token_authentication_policy.cpp
    src/http/body_stream.cpp
    src/http/http.cpp
    src/http/logging_policy.cpp
    src/http/policy.cpp
    src/http/raw_response.cpp
    src/http/request.cpp
    src/http/retry_policy.cpp
    src/http/telemetry_policy.cpp
    src/http/transport_policy.cpp
    src/http/url.cpp
    src/logging/logging.cpp
    src/base64.cpp
    src/context.cpp
    src/datetime.cpp
    src/operation_status.cpp
    src/strings.cpp
    src/version.cpp
)

add_library(azure-core ${AZURE_CORE_HEADER} ${AZURE_CORE_SOURCE})

target_include_directories(
  azure-core
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
)

# make sure that users can consume the project as a library.
add_library(Azure::azure-core ALIAS azure-core)

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(core azure-core azure-core-test)

target_link_libraries(azure-core INTERFACE Threads::Threads)

if(WIN32)
    target_link_libraries(azure-core PRIVATE bcrypt crypt32)
else()
    find_package(OpenSSL REQUIRED)
    target_link_libraries(azure-core PRIVATE OpenSSL::SSL)
endif()

if(BUILD_TRANSPORT_CURL)
  target_link_libraries(azure-core PUBLIC CURL::libcurl)
endif()
if(BUILD_TRANSPORT_WINHTTP)
  target_link_libraries(azure-core PRIVATE winhttp)
endif()

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/inc/azure/core/version.hpp")
generate_documentation(azure-core ${AZ_LIBRARY_VERSION})

az_vcpkg_export(
    azure-core
    CORE
    "azure/core/dll_import_export.hpp"
  )

if (BUILD_PERFORMANCE_TESTS)
  add_subdirectory(test/performance)
endif()

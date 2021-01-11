# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)
project(azure-core LANGUAGES CXX)

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

# Storage requires 3.6.0 version for using `contains` feature
set(NLOHMANN_JSON_MIN_REQUIRED_VERSION 3.6.0)
# Try to find the config cmake file. Typically for when the integration is made at CMake level
# (CMake project available with a configuration file to consume)
find_package(nlohmann_json ${NLOHMANN_JSON_MIN_REQUIRED_VERSION} CONFIG QUIET)
if(NOT nlohmann_json_FOUND)
  # If CMake project/config is not found, last option is to find the library installed. The integration is done by the linker directly. 
  find_package(nlohmann_json ${NLOHMANN_JSON_MIN_REQUIRED_VERSION} REQUIRED)
endif()
message("nlohmann json version ${nlohmann_json_VERSION}")

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
    inc/azure/core/http/body_stream.hpp
    inc/azure/core/http/http.hpp
    inc/azure/core/http/pipeline.hpp
    inc/azure/core/http/policy.hpp
    inc/azure/core/http/transport.hpp
    inc/azure/core/internal/contract.hpp
    inc/azure/core/internal/log.hpp
    inc/azure/core/logging/logging.hpp
    inc/azure/core/base64.hpp
    inc/azure/core/context.hpp
    inc/azure/core/credentials.hpp
    inc/azure/core/datetime.hpp
    inc/azure/core/exception.hpp
    inc/azure/core/nullable.hpp
    inc/azure/core/operation.hpp
    inc/azure/core/operation_status.hpp
    inc/azure/core/platform.hpp
    inc/azure/core/response.hpp
    inc/azure/core/strings.hpp
    inc/azure/core/uuid.hpp
    inc/azure/core/version.hpp
    inc/azure/core.hpp
)

set(
  AZURE_CORE_SOURCE
    ${CURL_TRANSPORT_ADAPTER_SRC}
    ${WIN_TRANSPORT_ADAPTER_SRC}
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

# ${CURL_INCLUDE_DIRS} needs to be public as long as we #include<curl.h> in public headers.
target_include_directories(azure-core PUBLIC ${CURL_INCLUDE_DIRS})
target_include_directories(azure-core INTERFACE ${nlohmann_json_INCLUDE_DIRS})
target_link_libraries(azure-core INTERFACE Threads::Threads nlohmann_json::nlohmann_json)

if(MSVC)
    target_link_libraries(azure-core PRIVATE crypt32)
else()
    message("Find the OpenSSL package on MacOS, used for Base64 encoding.")
    find_package(OpenSSL REQUIRED)
    target_link_libraries(azure-core PRIVATE OpenSSL::SSL)
endif()

if(BUILD_TRANSPORT_CURL)
  target_link_libraries(azure-core PRIVATE CURL::libcurl)
endif()
if(BUILD_TRANSPORT_WINHTTP)
  target_link_libraries(azure-core PRIVATE winhttp)
endif()

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/inc/azure/core/version.hpp")
generate_documentation(azure-core ${AZ_LIBRARY_VERSION})

az_vcpkg_export(azure-core)

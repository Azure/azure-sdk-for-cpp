# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

# setting CMAKE_TOOLCHAIN_FILE must happen before creating the project
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake-modules")
include(AzureVcpkg)
az_vcpkg_integrate()

# Azure core is compatible with CMake 3.12
cmake_minimum_required (VERSION 3.12)
project(azure-core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

include(AzureVersion)
include(AzureCodeCoverage)
include(AzureTransportAdapters)
include(AzureDoxygen)
include(AzureGlobalCompileOptions)
include(AzureConfigRTTI)
# Add create_map_file function
include(CreateMapFile)

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
  SET(CURL_TRANSPORT_ADAPTER_SRC
    src/http/curl/curl.cpp
    src/http/curl/curl_connection_pool_private.hpp
    src/http/curl/curl_connection_private.hpp
    src/http/curl/curl_session_private.hpp
  )
  SET(CURL_TRANSPORT_ADAPTER_INC
    inc/azure/core/http/curl_transport.hpp
  )
endif()
if(BUILD_TRANSPORT_WINHTTP)
  SET(WIN_TRANSPORT_ADAPTER_SRC src/http/winhttp/win_http_transport.cpp)
  SET(WIN_TRANSPORT_ADAPTER_INC inc/azure/core/http/win_http_transport.hpp)
endif()

set(
  AZURE_CORE_HEADER
    ${CURL_TRANSPORT_ADAPTER_INC}
    ${WIN_TRANSPORT_ADAPTER_INC}
    inc/azure/core.hpp 
    inc/azure/core/azure_assert.hpp
    inc/azure/core/base64.hpp
    inc/azure/core/case_insensitive_containers.hpp
    inc/azure/core/context.hpp
    inc/azure/core/credentials/credentials.hpp
    inc/azure/core/credentials/token_credential_options.hpp
    inc/azure/core/cryptography/hash.hpp
    inc/azure/core/datetime.hpp
    inc/azure/core/diagnostics/logger.hpp
    inc/azure/core/dll_import_export.hpp
    inc/azure/core/etag.hpp
    inc/azure/core/exception.hpp
    inc/azure/core/http/http.hpp
    inc/azure/core/http/http_status_code.hpp
    inc/azure/core/http/policies/policy.hpp
    inc/azure/core/http/raw_response.hpp
    inc/azure/core/http/transport.hpp
    inc/azure/core/internal/client_options.hpp
    inc/azure/core/internal/contract.hpp
    inc/azure/core/internal/cryptography/sha_hash.hpp
    inc/azure/core/internal/diagnostics/log.hpp
    inc/azure/core/internal/environment.hpp
    inc/azure/core/internal/extendable_enumeration.hpp
    inc/azure/core/internal/http/http_sanitizer.hpp
    inc/azure/core/internal/http/pipeline.hpp
    inc/azure/core/internal/http/user_agent.hpp
    inc/azure/core/internal/io/null_body_stream.hpp
    inc/azure/core/internal/json/json.hpp
    inc/azure/core/internal/json/json_optional.hpp
    inc/azure/core/internal/json/json_serializable.hpp
    inc/azure/core/internal/strings.hpp
    inc/azure/core/internal/tracing/service_tracing.hpp
    inc/azure/core/io/body_stream.hpp
    inc/azure/core/match_conditions.hpp
    inc/azure/core/modified_conditions.hpp
    inc/azure/core/nullable.hpp
    inc/azure/core/operation.hpp
    inc/azure/core/operation_status.hpp
    inc/azure/core/paged_response.hpp
    inc/azure/core/platform.hpp
    inc/azure/core/response.hpp
    inc/azure/core/rtti.hpp
    inc/azure/core/tracing/tracing.hpp
    inc/azure/core/url.hpp
    inc/azure/core/uuid.hpp
    )

set(
  AZURE_CORE_SOURCE
    ${CURL_TRANSPORT_ADAPTER_SRC}
    ${WIN_TRANSPORT_ADAPTER_SRC}
    src/azure_assert.cpp
    src/base64.cpp
    src/context.cpp
    src/cryptography/md5.cpp
    src/cryptography/sha_hash.cpp
    src/datetime.cpp
    src/environment.cpp
    src/environment_log_level_listener.cpp
    src/etag.cpp
    src/exception.cpp
    src/http/bearer_token_authentication_policy.cpp
    src/http/http.cpp
    src/http/http_sanitizer.cpp
    src/http/log_policy.cpp
    src/http/policy.cpp
    src/http/raw_response.cpp
    src/http/request.cpp
    src/http/request_activity_policy.cpp
    src/http/retry_policy.cpp
    src/http/telemetry_policy.cpp
    src/http/transport_policy.cpp
    src/http/url.cpp
    src/http/user_agent.cpp
    src/io/body_stream.cpp
    src/io/random_access_file_body_stream.cpp
    src/logger.cpp
    src/operation_status.cpp
    src/private/environment_log_level_listener.hpp
    src/private/package_version.hpp
    src/strings.cpp
    src/tracing/tracing.cpp
    src/uuid.cpp
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
create_code_coverage(core azure-core azure-core-test "tests?/*;samples?/*;inc/azure/core/internal/json/json.hpp")

target_link_libraries(azure-core INTERFACE Threads::Threads)

if(WIN32)
    target_link_libraries(azure-core PRIVATE bcrypt crypt32)
else()
    # Required for Hashing ( md5 and sha ).
    find_package(OpenSSL REQUIRED)
    target_link_libraries(azure-core PRIVATE OpenSSL::SSL)
endif()

if(BUILD_TRANSPORT_CURL)
  target_link_libraries(azure-core PUBLIC CURL::libcurl)
endif()
if(BUILD_TRANSPORT_WINHTTP)
  target_link_libraries(azure-core PRIVATE winhttp)
endif()

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/src/private/package_version.hpp")
generate_documentation(azure-core ${AZ_LIBRARY_VERSION})

az_vcpkg_export(
    azure-core
    CORE
    "azure/core/dll_import_export.hpp"
  )

az_rtti_setup(
  azure-core
  CORE
  "azure/core/rtti.hpp"
)

if(BUILD_TESTING)
  # define a symbol that enables some test hooks in code
  add_compile_definitions(TESTING_BUILD)
  
  if (NOT AZ_ALL_LIBRARIES)
    include(AddGoogleTest)
    enable_testing ()
  endif()
  
  add_subdirectory(test/ut)
  if(DEFINED ENV{AZURE_CORE_ENABLE_JSON_TESTS})
    add_subdirectory(test/nlohmann-json-test)
  endif()
  add_subdirectory(test/fault-injector)
endif()

if (BUILD_PERFORMANCE_TESTS)
  add_subdirectory(test/perf)
endif()

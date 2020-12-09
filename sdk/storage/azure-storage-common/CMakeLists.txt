# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)
project(azure-storage-common LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake-modules")

include(az_version)
include(CodeCoverage)
include(DefineTransportAdapter)
include(doxygen_common)
include(global_compile_options)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)
find_package(LibXml2 REQUIRED)

add_library(
  azure-storage-common
    src/account_sas_builder.cpp
    src/crypt.cpp
    src/file_io.cpp
    src/reliable_stream.cpp
    src/shared_key_policy.cpp
    src/storage_common.cpp
    src/storage_credential.cpp
    src/storage_exception.cpp
    src/storage_per_retry_policy.cpp
    src/storage_retry_policy.cpp
    src/xml_wrapper.cpp
)

# make sure that users can consume the project as a library.
add_library(azure::storage::common ALIAS azure-storage-common)

target_include_directories(azure-storage-common PUBLIC inc ${LIBXML2_INCLUDE_DIRS})
target_link_libraries(azure-storage-common Threads::Threads azure-core ${LIBXML2_LIBRARIES})
if(MSVC)
    target_link_libraries(azure-storage-common bcrypt)
    # C28020 and C28204 are introduced by nlohmann/json
    target_compile_options(azure-storage-common PUBLIC /wd28204 /wd28020)
else()
    find_package(OpenSSL REQUIRED)
    target_link_libraries(azure-storage-common OpenSSL::SSL OpenSSL::Crypto)
endif()

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/inc/azure/storage/common/version.hpp")
generate_documentation(azure-storage-common ${AZ_LIBRARY_VERSION})

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
# excluding json from coverage report
create_code_coverage(storage azure-storage-common azure-storage-test "inc/azure/storage/common/json*")

if(BUILD_TESTING)
  target_sources(
    azure-storage-test
      PRIVATE
        test/bearer_token_test.cpp
        test/crypt_functions_test.cpp
        test/metadata_test.cpp
        test/storage_credential_test.cpp
        test/test_base.cpp
  )

  if (MSVC)
    target_compile_options(azure-storage-test PUBLIC /wd6326 /wd26495 /wd26812)
  endif()

  target_link_libraries(azure-storage-test PRIVATE azure-identity)
  target_include_directories(azure-storage-test PRIVATE test)
endif()

if(BUILD_STORAGE_SAMPLES)
  target_sources(
    azure-storage-sample
      PRIVATE
        sample/main.cpp
        sample/samples_common.hpp
  )

  target_include_directories(azure-storage-sample PRIVATE sample)
endif()

# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

set(AZURE_STORAGE_COMMON_HEADER
    inc/azure/storage/common/access_conditions.hpp
    inc/azure/storage/common/account_sas_builder.hpp
    inc/azure/storage/common/concurrent_transfer.hpp
    inc/azure/storage/common/constants.hpp
    inc/azure/storage/common/crypt.hpp
    inc/azure/storage/common/file_io.hpp
    inc/azure/storage/common/json.hpp
    inc/azure/storage/common/reliable_stream.hpp
    inc/azure/storage/common/shared_key_policy.hpp
    inc/azure/storage/common/storage_common.hpp
    inc/azure/storage/common/storage_credential.hpp
    inc/azure/storage/common/storage_error.hpp
    inc/azure/storage/common/storage_per_retry_policy.hpp
    inc/azure/storage/common/storage_retry_policy.hpp
    inc/azure/storage/common/version.hpp
    inc/azure/storage/common/xml_wrapper.hpp
)

set(AZURE_STORAGE_COMMON_SOURCE
    src/account_sas_builder.cpp
    src/crypt.cpp
    src/file_io.cpp
    src/reliable_stream.cpp
    src/shared_key_policy.cpp
    src/storage_common.cpp
    src/storage_credential.cpp
    src/storage_error.cpp
    src/storage_per_retry_policy.cpp
    src/storage_retry_policy.cpp
    src/xml_wrapper.cpp
)

# CodeCoverage will validate requirements
include(CodeCoverage)

add_library(azure-storage-common ${AZURE_STORAGE_COMMON_HEADER} ${AZURE_STORAGE_COMMON_SOURCE})

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)
find_package(LibXml2 REQUIRED)

target_include_directories(azure-storage-common PUBLIC inc ${LIBXML2_INCLUDE_DIR})
target_link_libraries(azure-storage-common Threads::Threads azure-core azure-identity ${LIBXML2_LIBRARIES})
if(MSVC)
    target_link_libraries(azure-storage-common bcrypt)
    # C28020 and C28204 are introduced by nlohmann/json
    target_compile_options(azure-storage-common PUBLIC /wd28204 /wd28020)
else()
    find_package(OpenSSL REQUIRED)
    target_link_libraries(azure-storage-common OpenSSL::SSL OpenSSL::Crypto)
endif()

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(storage azure-storage-common azure-storage-test)

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/inc/azure/storage/common/version.hpp")
add_library(azure::storage::common ALIAS azure-storage-common)


target_sources(
    azure-storage-test
    PRIVATE
    test/bearer_token_test.cpp
    test/crypt_functions_test.cpp
    test/test_base.cpp
    test/test_base.hpp
)

target_include_directories(azure-storage-test PUBLIC test)
if (MSVC)
    target_compile_options(azure-storage-test PUBLIC /wd6326 /wd26495 /wd26812)
endif()


target_sources(
    azure-storage-sample
    PRIVATE
    sample/main.cpp
    sample/samples_common.hpp
)
    
target_include_directories(azure-storage-sample PUBLIC sample)

generate_documentation(azure-storage-common ${AZ_LIBRARY_VERSION})

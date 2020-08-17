# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.15)

project (azure-storage LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

option(BUILD_STORAGE_SAMPLES "Build sample codes" ON)

if(MSVC)
    add_compile_definitions(NOMINMAX)
endif()

set(AZURE_STORAGE_COMMON_HEADER
    inc/common/access_conditions.hpp
    inc/common/storage_per_retry_policy.hpp
    inc/common/concurrent_transfer.hpp
    inc/common/constants.hpp
    inc/common/crypt.hpp
    inc/common/file_io.hpp
    inc/common/reliable_stream.hpp
    inc/common/shared_key_policy.hpp
    inc/common/storage_common.hpp
    inc/common/storage_credential.hpp
    inc/common/storage_error.hpp
    inc/common/storage_uri_builder.hpp
    inc/common/storage_version.hpp
    inc/common/xml_wrapper.hpp
    inc/common/account_sas_builder.hpp
)

set(AZURE_STORAGE_COMMON_SOURCE
    src/common/storage_per_retry_policy.cpp
    src/common/crypt.cpp
    src/common/file_io.cpp
    src/common/reliable_stream.cpp
    src/common/shared_key_policy.cpp
    src/common/storage_common.cpp
    src/common/storage_credential.cpp
    src/common/storage_error.cpp
    src/common/storage_uri_builder.cpp
    src/common/xml_wrapper.cpp
    src/common/account_sas_builder.cpp
)

add_library(azure-storage-common ${AZURE_STORAGE_COMMON_HEADER} ${AZURE_STORAGE_COMMON_SOURCE})

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)
find_package(LibXml2 REQUIRED)

target_include_directories(azure-storage-common PUBLIC ${LIBXML2_INCLUDE_DIR} $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc> $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external> $<INSTALL_INTERFACE:include/azure_storage>)
target_link_libraries(azure-storage-common Threads::Threads azure-core ${LIBXML2_LIBRARIES})

if(MSVC)
    target_link_libraries(azure-storage-common bcrypt)
    # C28020 and C28204 are introduced by nlohmann/json
    target_compile_options(azure-storage-common PUBLIC /wd28204 /wd28020)
else()
    find_package(OpenSSL REQUIRED)
    target_link_libraries(azure-storage-common OpenSSL::SSL OpenSSL::Crypto)
endif()

file(READ "inc/common/storage_version.hpp" VERSION_FILE_CONTENT)

string(REGEX MATCH "CommonComponentVersion = \"([^\"]*)" _ ${VERSION_FILE_CONTENT})
if ("${CMAKE_MATCH_1}" STREQUAL "")
    message(FATAL_ERROR "Failed to get Common component version")
endif()
set(COMMON_COMPONENT_VERSION ${CMAKE_MATCH_1})
message("Common component version ${COMMON_COMPONENT_VERSION}")

string(REGEX MATCH "BlobServiceVersion = \"([^\"]*)" _ ${VERSION_FILE_CONTENT})
if ("${CMAKE_MATCH_1}" STREQUAL "")
    message(FATAL_ERROR "Failed to get Blob service version")
endif()
set(BLOB_SERVICE_VERSION ${CMAKE_MATCH_1})
message("Blob service version ${BLOB_SERVICE_VERSION}")

string(REGEX MATCH "DataLakeServiceVersion = \"([^\"]*)" _ ${VERSION_FILE_CONTENT})
if ("${CMAKE_MATCH_1}" STREQUAL "")
    message(FATAL_ERROR "Failed to get DataLake service version")
endif()
set(DATALAKE_SERVICE_VERSION ${CMAKE_MATCH_1})
message("DataLake service version ${DATALAKE_SERVICE_VERSION}")

set (AZURE_STORAGE_BLOB_HEADER
    inc/blobs/blob.hpp
    inc/blobs/blob_service_client.hpp
    inc/blobs/blob_container_client.hpp
    inc/blobs/blob_client.hpp
    inc/blobs/block_blob_client.hpp
    inc/blobs/page_blob_client.hpp
    inc/blobs/append_blob_client.hpp
    inc/blobs/blob_options.hpp
    inc/blobs/blob_responses.hpp
    inc/blobs/blob_sas_builder.hpp
    inc/blobs/protocol/blob_rest_client.hpp
)

set (AZURE_STORAGE_BLOB_SOURCE
    src/blobs/blob_service_client.cpp
    src/blobs/blob_container_client.cpp
    src/blobs/blob_client.cpp
    src/blobs/block_blob_client.cpp
    src/blobs/page_blob_client.cpp
    src/blobs/append_blob_client.cpp
    src/blobs/blob_sas_builder.cpp
)

add_library(azure-storage-blob ${AZURE_STORAGE_BLOB_HEADER} ${AZURE_STORAGE_BLOB_SOURCE})
target_link_libraries(azure-storage-blob azure-storage-common)

set (AZURE_STORAGE_DATALAKE_HEADER
    inc/datalake/datalake.hpp
    inc/datalake/protocol/datalake_rest_client.hpp
    inc/datalake/datalake_responses.hpp
    inc/datalake/datalake_options.hpp
    inc/datalake/datalake_utilities.hpp
    inc/datalake/datalake_service_client.hpp
    inc/datalake/datalake_file_system_client.hpp
    inc/datalake/datalake_path_client.hpp
    inc/datalake/datalake_file_client.hpp
    inc/datalake/datalake_directory_client.hpp
)

set (AZURE_STORAGE_DATALAKE_SOURCE
    src/datalake/datalake_service_client.cpp
    src/datalake/datalake_file_system_client.cpp
    src/datalake/datalake_path_client.cpp
    src/datalake/datalake_file_client.cpp
    src/datalake/datalake_directory_client.cpp
    src/datalake/datalake_utilities.cpp
    src/datalake/datalake_responses.cpp
)

add_library(azure-storage-file-datalake ${AZURE_STORAGE_DATALAKE_HEADER} ${AZURE_STORAGE_DATALAKE_SOURCE})
target_link_libraries(azure-storage-file-datalake azure-storage-blob)

set (AZURE_STORAGE_SHARES_HEADER
    inc/shares/shares.hpp
    inc/shares/protocol/share_rest_client.hpp
    inc/shares/share_file_attribute.hpp
    inc/shares/share_options.hpp
    inc/shares/share_responses.hpp
    inc/shares/share_service_client.hpp
    inc/shares/share_client.hpp
    inc/shares/share_directory_client.hpp
    inc/shares/share_file_client.hpp
)

set (AZURE_STORAGE_SHARES_SOURCE
    src/shares/share_service_client.cpp
    src/shares/share_client.cpp
    src/shares/share_directory_client.cpp
    src/shares/share_file_client.cpp
)

add_library(azure-storage-file-share ${AZURE_STORAGE_SHARES_HEADER} ${AZURE_STORAGE_SHARES_SOURCE})
target_link_libraries(azure-storage-file-share azure-storage-common)

add_library(azure::storage::common ALIAS azure-storage-common)
add_library(azure::storage::blob ALIAS azure-storage-blob)
add_library(azure::storage::file::datalake ALIAS azure-storage-file-datalake)
add_library(azure::storage::file::share ALIAS azure-storage-file-share)

if(BUILD_STORAGE_SAMPLES)
  add_subdirectory(sample)
endif()

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

# TODO: eng/sys teams is going to fix this
generate_documentation(azure-storage 1.0.0-preview.1)

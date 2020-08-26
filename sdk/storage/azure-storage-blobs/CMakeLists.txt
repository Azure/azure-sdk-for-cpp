# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.15)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

set (AZURE_STORAGE_BLOB_HEADER
    inc/azure/storage/blobs/blob.hpp
    inc/azure/storage/blobs/blob_service_client.hpp
    inc/azure/storage/blobs/blob_container_client.hpp
    inc/azure/storage/blobs/blob_client.hpp
    inc/azure/storage/blobs/block_blob_client.hpp
    inc/azure/storage/blobs/page_blob_client.hpp
    inc/azure/storage/blobs/append_blob_client.hpp
    inc/azure/storage/blobs/blob_options.hpp
    inc/azure/storage/blobs/blob_responses.hpp
    inc/azure/storage/blobs/blob_sas_builder.hpp
    inc/azure/storage/blobs/protocol/blob_rest_client.hpp
)

set (AZURE_STORAGE_BLOB_SOURCE
    src/blob_service_client.cpp
    src/blob_container_client.cpp
    src/blob_client.cpp
    src/block_blob_client.cpp
    src/page_blob_client.cpp
    src/append_blob_client.cpp
    src/blob_sas_builder.cpp
)

add_library(azure-storage-blobs ${AZURE_STORAGE_BLOB_HEADER} ${AZURE_STORAGE_BLOB_SOURCE})
target_include_directories(azure-storage-blobs PUBLIC inc)
target_link_libraries(azure-storage-blobs azure::storage::common)

add_library(azure::storage::blobs ALIAS azure-storage-blobs)

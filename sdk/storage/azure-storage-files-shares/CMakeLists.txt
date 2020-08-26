# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.15)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

set (AZURE_STORAGE_SHARES_HEADER
    inc/azure/storage/files/shares/shares.hpp
    inc/azure/storage/files/shares/protocol/share_rest_client.hpp
    inc/azure/storage/files/shares/share_constants.hpp
    inc/azure/storage/files/shares/share_file_attribute.hpp
    inc/azure/storage/files/shares/share_options.hpp
    inc/azure/storage/files/shares/share_responses.hpp
    inc/azure/storage/files/shares/share_service_client.hpp
    inc/azure/storage/files/shares/share_client.hpp
    inc/azure/storage/files/shares/share_directory_client.hpp
    inc/azure/storage/files/shares/share_file_client.hpp
)

set (AZURE_STORAGE_SHARES_SOURCE
    src/share_service_client.cpp
    src/share_client.cpp
    src/share_directory_client.cpp
    src/share_file_client.cpp
)

add_library(azure-storage-files-shares ${AZURE_STORAGE_SHARES_HEADER} ${AZURE_STORAGE_SHARES_SOURCE})
target_include_directories(azure-storage-files-shares PUBLIC inc)
target_link_libraries(azure-storage-files-shares azure-storage-common)

add_library(azure::storage::files::shares ALIAS azure-storage-files-shares)

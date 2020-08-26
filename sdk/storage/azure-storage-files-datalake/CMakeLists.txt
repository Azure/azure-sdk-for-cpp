# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.15)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

set (AZURE_STORAGE_DATALAKE_HEADER
    inc/azure/storage/files/datalake/datalake.hpp
    inc/azure/storage/files/datalake/protocol/datalake_rest_client.hpp
    inc/azure/storage/files/datalake/datalake_responses.hpp
    inc/azure/storage/files/datalake/datalake_options.hpp
    inc/azure/storage/files/datalake/datalake_utilities.hpp
    inc/azure/storage/files/datalake/datalake_service_client.hpp
    inc/azure/storage/files/datalake/datalake_file_system_client.hpp
    inc/azure/storage/files/datalake/datalake_path_client.hpp
    inc/azure/storage/files/datalake/datalake_file_client.hpp
    inc/azure/storage/files/datalake/datalake_directory_client.hpp
)

set (AZURE_STORAGE_DATALAKE_SOURCE
    src/datalake_service_client.cpp
    src/datalake_file_system_client.cpp
    src/datalake_path_client.cpp
    src/datalake_file_client.cpp
    src/datalake_directory_client.cpp
    src/datalake_utilities.cpp
    src/datalake_responses.cpp
)

add_library(azure-storage-files-datalake ${AZURE_STORAGE_DATALAKE_HEADER} ${AZURE_STORAGE_DATALAKE_SOURCE})
target_include_directories(azure-storage-files-datalake PUBLIC inc)
target_link_libraries(azure-storage-files-datalake azure-storage-blobs)

add_library(azure::storage::files::datalake ALIAS azure-storage-files-datalake)

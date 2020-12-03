# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

set (AZURE_STORAGE_DATALAKE_HEADER
    inc/azure/storage/files/datalake.hpp
    inc/azure/storage/files/datalake/datalake_directory_client.hpp
    inc/azure/storage/files/datalake/datalake_file_client.hpp
    inc/azure/storage/files/datalake/datalake_file_system_client.hpp
    inc/azure/storage/files/datalake/datalake_options.hpp
    inc/azure/storage/files/datalake/datalake_path_client.hpp
    inc/azure/storage/files/datalake/datalake_responses.hpp
    inc/azure/storage/files/datalake/datalake_sas_builder.hpp
    inc/azure/storage/files/datalake/datalake_service_client.hpp
    inc/azure/storage/files/datalake/datalake_utilities.hpp
    inc/azure/storage/files/datalake/protocol/datalake_rest_client.hpp
    inc/azure/storage/files/datalake/version.hpp
)

set (AZURE_STORAGE_DATALAKE_SOURCE
    src/datalake_directory_client.cpp
    src/datalake_file_client.cpp
    src/datalake_file_system_client.cpp
    src/datalake_path_client.cpp
    src/datalake_responses.cpp
    src/datalake_sas_builder.cpp
    src/datalake_service_client.cpp
    src/datalake_utilities.cpp
)

add_library(azure-storage-files-datalake ${AZURE_STORAGE_DATALAKE_HEADER} ${AZURE_STORAGE_DATALAKE_SOURCE})
target_include_directories(azure-storage-files-datalake PUBLIC inc)
target_link_libraries(azure-storage-files-datalake azure-storage-blobs)

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/inc/azure/storage/files/datalake/version.hpp")
add_library(azure::storage::files::datalake ALIAS azure-storage-files-datalake)

generate_documentation(azure-storage-files-datalake ${AZ_LIBRARY_VERSION})

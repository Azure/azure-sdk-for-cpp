# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

set (AZURE_STORAGE_SHARES_HEADER
    inc/azure/storage/files/shares.hpp
    inc/azure/storage/files/shares/protocol/share_rest_client.hpp
    inc/azure/storage/files/shares/share_client.hpp
    inc/azure/storage/files/shares/share_constants.hpp
    inc/azure/storage/files/shares/share_directory_client.hpp
    inc/azure/storage/files/shares/share_file_attribute.hpp
    inc/azure/storage/files/shares/share_file_client.hpp
    inc/azure/storage/files/shares/share_options.hpp
    inc/azure/storage/files/shares/share_responses.hpp
    inc/azure/storage/files/shares/share_sas_builder.hpp
    inc/azure/storage/files/shares/share_service_client.hpp
    inc/azure/storage/files/shares/version.hpp
)

set (AZURE_STORAGE_SHARES_SOURCE
    src/share_client.cpp
    src/share_directory_client.cpp
    src/share_file_client.cpp
    src/share_sas_builder.cpp
    src/share_service_client.cpp
)

# CodeCoverage will validate requirements
include(CodeCoverage)

add_library(azure-storage-files-shares ${AZURE_STORAGE_SHARES_HEADER} ${AZURE_STORAGE_SHARES_SOURCE})
target_include_directories(azure-storage-files-shares PUBLIC inc)
target_link_libraries(azure-storage-files-shares azure-storage-common)

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(storage azure-storage-files-shares azure-storage-test)

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/inc/azure/storage/files/shares/version.hpp")
add_library(azure::storage::files::shares ALIAS azure-storage-files-shares)


target_sources(
    azure-storage-test
    PRIVATE
    test/share_client_test.cpp
    test/share_client_test.hpp
    test/share_directory_client_test.cpp
    test/share_directory_client_test.hpp
    test/share_file_client_test.cpp
    test/share_file_client_test.hpp
    test/share_sas_test.cpp
    test/share_service_client_test.cpp
    test/share_service_client_test.hpp
)
target_link_libraries(azure-storage-test PUBLIC azure::storage::files::shares)


target_sources(
    azure-storage-sample
    PRIVATE
    sample/file_share_getting_started.cpp
)
target_link_libraries(azure-storage-sample PUBLIC azure::storage::files::shares)

generate_documentation(azure-storage-files-shares ${AZ_LIBRARY_VERSION})

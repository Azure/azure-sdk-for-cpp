# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

if(MSVC)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

add_executable(blob-getting-started blob_getting_started.cpp)
target_link_libraries(blob-getting-started PRIVATE azure-storage-blobs get-env-helper)
target_compile_definitions(blob-getting-started PRIVATE _azure_BUILDING_SAMPLES)
create_per_service_target_build_for_sample(storage blob-getting-started)

add_executable(blob-list-operation blob_list_operation.cpp)
target_link_libraries(blob-list-operation  PRIVATE azure-storage-blobs get-env-helper)
target_compile_definitions(blob-list-operation PRIVATE _azure_BUILDING_SAMPLES)
create_per_service_target_build_for_sample(storage blob-list-operation)

add_executable(blob-sas blob_sas.cpp)
target_link_libraries(blob-sas PRIVATE azure-storage-blobs get-env-helper)
target_compile_definitions(blob-sas PRIVATE _azure_BUILDING_SAMPLES)
create_per_service_target_build_for_sample(storage blob-sas)

add_executable(transactional-checksum transactional_checksum.cpp)
target_link_libraries(transactional-checksum PRIVATE azure-storage-blobs get-env-helper)
target_compile_definitions(transactional-checksum PRIVATE _azure_BUILDING_SAMPLES)
create_per_service_target_build_for_sample(storage transactional-checksum)

add_executable(blob-query blob_query.cpp)
target_link_libraries(blob-query PRIVATE azure-storage-blobs get-env-helper)
target_compile_definitions(blob-query PRIVATE _azure_BUILDING_SAMPLES)
create_per_service_target_build_for_sample(storage blob-query)

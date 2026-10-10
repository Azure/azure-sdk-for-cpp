# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

if(MSVC)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

add_executable(blob-getting-started blob_getting_started.cpp)
target_compile_features(blob-getting-started PRIVATE cxx_std_14)
set_target_properties(blob-getting-started PROPERTIES CXX_STANDARD_REQUIRED ON)
target_link_libraries(blob-getting-started PRIVATE azure-storage-blobs get-env-helper)
target_compile_definitions(blob-getting-started PRIVATE _azure_BUILDING_SAMPLES)
create_per_service_target_build_for_sample(storage blob-getting-started)

add_executable(blob-list-operation blob_list_operation.cpp)
target_compile_features(blob-list-operation PRIVATE cxx_std_14)
set_target_properties(blob-list-operation PROPERTIES CXX_STANDARD_REQUIRED ON)
target_link_libraries(blob-list-operation  PRIVATE azure-storage-blobs get-env-helper)
target_compile_definitions(blob-list-operation PRIVATE _azure_BUILDING_SAMPLES)
create_per_service_target_build_for_sample(storage blob-list-operation)

add_executable(blob-sas blob_sas.cpp)
target_compile_features(blob-sas PRIVATE cxx_std_14)
set_target_properties(blob-sas PROPERTIES CXX_STANDARD_REQUIRED ON)
target_link_libraries(blob-sas PRIVATE azure-storage-blobs get-env-helper)
target_compile_definitions(blob-sas PRIVATE _azure_BUILDING_SAMPLES)
create_per_service_target_build_for_sample(storage blob-sas)

add_executable(transactional-checksum transactional_checksum.cpp)
target_compile_features(transactional-checksum PRIVATE cxx_std_14)
set_target_properties(transactional-checksum PROPERTIES CXX_STANDARD_REQUIRED ON)
target_link_libraries(transactional-checksum PRIVATE azure-storage-blobs get-env-helper)
target_compile_definitions(transactional-checksum PRIVATE _azure_BUILDING_SAMPLES)
create_per_service_target_build_for_sample(storage transactional-checksum)

add_executable(blob-query blob_query.cpp)
target_compile_features(blob-query PRIVATE cxx_std_14)
set_target_properties(blob-query PROPERTIES CXX_STANDARD_REQUIRED ON)
target_link_libraries(blob-query PRIVATE azure-storage-blobs get-env-helper)
target_compile_definitions(blob-query PRIVATE _azure_BUILDING_SAMPLES)
create_per_service_target_build_for_sample(storage blob-query)

add_executable(blob_list_operation_with_timeout blob_list_operation_with_timeout.cpp)
target_compile_features(blob_list_operation_with_timeout PRIVATE cxx_std_14)
set_target_properties(blob_list_operation_with_timeout PROPERTIES CXX_STANDARD_REQUIRED ON)
target_link_libraries(blob_list_operation_with_timeout PRIVATE azure-storage-blobs get-env-helper)
create_per_service_target_build_for_sample(storage blob_list_operation_with_timeout)

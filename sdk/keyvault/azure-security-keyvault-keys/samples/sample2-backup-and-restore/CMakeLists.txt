# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

project (sample2-backup-and-restore LANGUAGES CXX)
if(NOT DEFINED CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD LESS 14 OR CMAKE_CXX_STANDARD EQUAL 98)
  set(CMAKE_CXX_STANDARD 14)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  sample2-backup-and-restore
  sample2_backup_and_restore.cpp
)

target_compile_definitions(sample2-backup-and-restore PRIVATE _azure_BUILDING_SAMPLES)

create_per_service_target_build_for_sample(keyvault sample2-backup-and-restore)

target_link_libraries(sample2-backup-and-restore PRIVATE azure-security-keyvault-keys azure-identity get-env-helper)

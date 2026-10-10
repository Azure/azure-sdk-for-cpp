# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

project (sample2-backup-and-restore LANGUAGES CXX)

add_executable (
  sample2-backup-and-restore
  sample2_backup_and_restore.cpp
)
target_compile_features(sample2-backup-and-restore PRIVATE cxx_std_14)
set_target_properties(sample2-backup-and-restore PROPERTIES CXX_STANDARD_REQUIRED ON)

target_compile_definitions(sample2-backup-and-restore PRIVATE _azure_BUILDING_SAMPLES)

create_per_service_target_build_for_sample(keyvault sample2-backup-and-restore)

target_link_libraries(sample2-backup-and-restore PRIVATE azure-security-keyvault-keys azure-identity get-env-helper)

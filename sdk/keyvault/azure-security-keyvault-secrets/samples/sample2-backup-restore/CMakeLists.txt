# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

project (sample2-backup-restore LANGUAGES CXX)

add_executable (
  sample2-backup-restore
  sample2_backup_restore.cpp
)
target_compile_features(sample2-backup-restore PRIVATE cxx_std_14)
set_target_properties(sample2-backup-restore PROPERTIES CXX_STANDARD_REQUIRED ON)

target_compile_definitions(sample2-backup-restore PRIVATE _azure_BUILDING_SAMPLES)

create_per_service_target_build_for_sample(keyvault sample2-backup-restore)

target_link_libraries(sample2-backup-restore PRIVATE azure-security-keyvault-secrets azure-identity get-env-helper)

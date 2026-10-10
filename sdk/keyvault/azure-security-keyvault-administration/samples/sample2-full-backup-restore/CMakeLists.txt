# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

project (sample2-full-backup-restore LANGUAGES CXX)

add_executable (
  sample2-full-backup-restore
  sample2_full_backup_restore.cpp
)
target_compile_features(sample2-full-backup-restore PRIVATE cxx_std_14)
set_target_properties(sample2-full-backup-restore PROPERTIES CXX_STANDARD_REQUIRED ON)
create_per_service_target_build_for_sample(keyvault sample2-full-backup-restore DISABLE_RUN)

target_link_libraries(sample2-full-backup-restore PRIVATE azure-security-keyvault-administration azure-identity get-env-helper)

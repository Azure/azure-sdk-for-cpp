# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

project (sample3-backup-selective-restore LANGUAGES CXX)

add_executable (
  sample3-backup-selective-restore
  sample3_backup_selective_restore.cpp
)
create_per_service_target_build_for_sample(keyvault sample3-backup-selective-restore DISABLE_RUN)

target_link_libraries(sample3-backup-selective-restore PRIVATE azure-security-keyvault-administration azure-identity get-env-helper)

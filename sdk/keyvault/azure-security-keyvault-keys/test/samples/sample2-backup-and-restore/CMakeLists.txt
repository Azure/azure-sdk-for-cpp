# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (sample2-backup-and-restore LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  sample2-backup-and-restore
  sample2_backup_and_restore.cpp
)
create_per_service_target_build_for_sample(keyvault sample2-backup-and-restore)

target_link_libraries(sample2-backup-and-restore PRIVATE azure-security-keyvault-keys azure-identity get-env-helper)
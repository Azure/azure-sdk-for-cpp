# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (sample2-backup-restore LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  sample2-backup-restore
  sample2_backup_restore.cpp
)

target_link_libraries(sample2-backup-restore PRIVATE azure-security-keyvault-secrets azure-identity)

# Add the sample to be run on CI. 
# CI pipeline reads the {service}-samples.txt and runs the binaries listed there.
file(APPEND ${CMAKE_BINARY_DIR}/keyvault-samples.txt "${CMAKE_CURRENT_BINARY_DIR}/sample2-backup-restore\n")

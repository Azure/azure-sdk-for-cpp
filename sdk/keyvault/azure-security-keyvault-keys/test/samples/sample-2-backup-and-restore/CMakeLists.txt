# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (sample-2-backup-and-restore LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  sample-2-backup-and-restore
  main.cpp
)

target_link_libraries(sample-2-backup-and-restore PRIVATE azure-security-keyvault-keys azure-identity)

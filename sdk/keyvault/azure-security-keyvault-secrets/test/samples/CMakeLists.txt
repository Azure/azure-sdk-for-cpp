# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

if(MSVC)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

add_subdirectory(sample1-basic-operations)
add_subdirectory(sample2-backup-restore)
add_subdirectory(sample3-delete-recover)
add_subdirectory(sample4-get-secrets-deleted)

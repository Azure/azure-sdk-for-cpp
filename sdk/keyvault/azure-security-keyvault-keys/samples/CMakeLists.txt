# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

if(MSVC)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

add_subdirectory(sample1-hello-world)
add_subdirectory(sample2-backup-and-restore)
add_subdirectory(sample3-get-keys)
add_subdirectory(sample4-encrypt-decrypt)
add_subdirectory(sample5-sign-verify)
add_subdirectory(sample6-wrap-unwrap)
add_subdirectory(sample7-key-rotation)


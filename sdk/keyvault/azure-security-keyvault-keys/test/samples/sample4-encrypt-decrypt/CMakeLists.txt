# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (sample4-encrypt-decrypt LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  sample4-encrypt-decrypt
  sample4_encrypt_decrypt.cpp
)

file(APPEND ${CMAKE_BINARY_DIR}/keyvault-samples.txt " ${CMAKE_CURRENT_BINARY_DIR}/sample4-encrypt-decrypt")

target_link_libraries(sample4-encrypt-decrypt PRIVATE azure-security-keyvault-keys azure-identity)

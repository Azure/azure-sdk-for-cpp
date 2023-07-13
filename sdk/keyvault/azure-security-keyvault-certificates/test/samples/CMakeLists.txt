# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

if(MSVC)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

add_subdirectory(certificate-basic-operations)
add_subdirectory(certificate-get-certificates)
add_subdirectory(certificate-import-certificate)

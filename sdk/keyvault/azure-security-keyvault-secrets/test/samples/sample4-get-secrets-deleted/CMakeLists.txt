# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (sample4-get-secrets-deleted LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  sample4-get-secrets-deleted
  sample4_get_secrets_deleted.cpp
)

create_per_service_target_build_for_sample(keyvault sample4-get-secrets-deleted)

target_link_libraries(sample4-get-secrets-deleted PRIVATE azure-security-keyvault-secrets azure-identity get-env-helper)
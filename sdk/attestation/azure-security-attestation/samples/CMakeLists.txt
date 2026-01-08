# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

az_build_shared_lib_test(azure-security-attestation ATTESTATION)

add_subdirectory(basic-operations)
add_subdirectory(attestation)
add_subdirectory(policy)
add_subdirectory(policy-certificates)

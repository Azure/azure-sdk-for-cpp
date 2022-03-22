# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (attestation-policy LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

macro (define_sample samplename)
add_executable (
  attestation-${samplename}
  ${samplename}.cpp)

CREATE_PER_SERVICE_TARGET_BUILD_FOR_SAMPLE(attestation-${samplename} attestation)

target_link_libraries(attestation-${samplename} PRIVATE azure-security-attestation azure-identity get-env-helper)

# Add the sample to be run on CI. 
# CI pipeline reads the {service}-samples.txt and runs the binaries listed there.
if (WIN32)
set(exe_suffix ".exe")
else()
set(exe_suffix "")
endif()
file(APPEND ${CMAKE_BINARY_DIR}/attestation-samples.txt "${CMAKE_CURRENT_BINARY_DIR}/attestation-${samplename}\n")

endmacro()

DEFINE_SAMPLE(get_policy)
DEFINE_SAMPLE(set_policy)
DEFINE_SAMPLE(reset_policy)
DEFINE_SAMPLE(set_sealed_policy)
DEFINE_SAMPLE(reset_sealed_policy)

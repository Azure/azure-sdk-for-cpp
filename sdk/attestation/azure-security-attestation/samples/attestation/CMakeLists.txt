# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (attestation-attestation LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

macro (define_sample samplename)
add_executable (
  attestation-${samplename}
  ${samplename}.cpp
  attestation_collateral.cpp 
  attestation_collateral.hpp)

create_per_service_target_build(attestation-${samplename} attestation)

target_link_libraries(attestation-${samplename} PRIVATE azure-security-attestation get-env-helper)

message(STATUS "Adding sample ${CMAKE_CURRENT_BINARY_DIR}/attestation-${samplename} to attestation-samples.txt")

# Add the sample to be run on CI. 
# CI pipeline reads the {service}-samples.txt and runs the binaries listed there.
file(APPEND ${CMAKE_BINARY_DIR}/attestation-samples.txt "${CMAKE_CURRENT_BINARY_DIR}/attestation-${samplename}\n")

endmacro()


define_sample(attestsgxenclave)
define_sample(attestsgxenclavewithruntimejson)
define_sample(attestsgxenclavewithruntimebinary)
define_sample(attestopenenclavewithdraftpolicy)
define_sample(attestopenenclaveshared)


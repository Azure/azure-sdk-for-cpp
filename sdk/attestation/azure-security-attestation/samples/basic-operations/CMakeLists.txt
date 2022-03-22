# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (attestation-basic-operations LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

macro (define_sample samplename)
add_executable (
  attestation-${samplename}
  ${samplename}.cpp)

CREATE_PER_SERVICE_TARGET_BUILD_FOR_SAMPLE(attestation-${samplename} attestation)

target_link_libraries(attestation-${samplename} PRIVATE azure-security-attestation azure-identity get-env-helper)

endmacro()

define_sample(createclient)
define_sample(createadminclient)
define_sample(getopenidmetadata)
define_sample(getsigningcertificates)

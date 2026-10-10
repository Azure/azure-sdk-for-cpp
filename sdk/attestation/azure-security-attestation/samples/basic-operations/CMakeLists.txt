# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

project (attestation-basic-operations LANGUAGES CXX)

macro (define_sample samplename)
add_executable (
  attestation-${samplename}
  ${samplename}.cpp)
target_compile_features(attestation-${samplename} PRIVATE cxx_std_14)
set_target_properties(attestation-${samplename} PROPERTIES CXX_STANDARD_REQUIRED ON)

CREATE_PER_SERVICE_TARGET_BUILD_FOR_SAMPLE(attestation attestation-${samplename})

target_link_libraries(attestation-${samplename} PRIVATE azure-security-attestation azure-identity get-env-helper)

target_compile_definitions(attestation-${samplename} PRIVATE _azure_BUILDING_SAMPLES)

endmacro()

define_sample(create_client)
define_sample(create_admin_client)
define_sample(get_open-id_metadata)
define_sample(get_signing_certificates)

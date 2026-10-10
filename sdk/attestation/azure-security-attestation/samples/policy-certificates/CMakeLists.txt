# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

project (attestation-policy LANGUAGES CXX)

macro (define_sample samplename)
add_executable (
  attestation-${samplename}
  ${samplename}.cpp
  cryptohelpers.hpp
)
target_compile_features(attestation-${samplename} PRIVATE cxx_std_14)
set_target_properties(attestation-${samplename} PROPERTIES CXX_STANDARD_REQUIRED ON)

CREATE_PER_SERVICE_TARGET_BUILD_FOR_SAMPLE(attestation-${samplename} attestation)

target_link_libraries(attestation-${samplename} PRIVATE azure-security-attestation azure-identity OpenSSL::Crypto get-env-helper)

target_compile_definitions(attestation-${samplename} PRIVATE _azure_BUILDING_SAMPLES)

endmacro()

DEFINE_SAMPLE(get_policy_certificates)
DEFINE_SAMPLE(add_policy_certificate)

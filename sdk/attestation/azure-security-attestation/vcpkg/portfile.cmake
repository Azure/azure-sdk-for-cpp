# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO Azure/azure-sdk-for-cpp
    REF azure-security-attestation_@AZ_LIBRARY_VERSION@
    SHA512 0
)

file(RENAME ${SOURCE_PATH}/sdk/attestation/azure-security-attestation ${SOURCE_PATH}/sdk/attestation/_)
file(RENAME ${SOURCE_PATH}/sdk/attestation ${SOURCE_PATH}/sdk/_)
file(RENAME ${SOURCE_PATH}/sdk ${SOURCE_PATH}/_)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}/_/_/_/"
    OPTIONS
        -DWARNINGS_AS_ERRORS=OFF
        -DBUILD_TESTING=OFF
)

vcpkg_cmake_install()
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
vcpkg_cmake_config_fixup()
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")
vcpkg_copy_pdbs()

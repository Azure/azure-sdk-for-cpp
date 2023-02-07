# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT
#

macro(SetUpTestProxy subDir)
    # assets.json dir
    add_compile_definitions(AZURE_TEST_ASSETS_DIR="${AZ_ROOT_DIR}/${subDir}/")
    #copy start stop scripts to the bin folder
    file(COPY ${AZ_ROOT_DIR}/eng/scripts/Start-TestProxy.ps1
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(COPY ${AZ_ROOT_DIR}/eng/scripts/Stop-TestProxy.ps1
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endmacro()

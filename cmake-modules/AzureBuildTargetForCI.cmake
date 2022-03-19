# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT
#
# Defines utility functions to create build targets for CI.
#

macro(create_per_service_target_build target service)

    file(APPEND ${CMAKE_BINARY_DIR}/${service}-targets-build.txt "${target}\n")

endmacro()

macro(add_sample_for_ci_run service sample_binary)
    SET(binary "${sample_binary}")
    if(MSVC)
        SET(binary "${sample_binary}/Release/${sample_binary}.exe")
    endif()
    file(APPEND ${CMAKE_BINARY_DIR}/${service}-samples.txt "${CMAKE_CURRENT_BINARY_DIR}/${binary}\n")

endmacro()

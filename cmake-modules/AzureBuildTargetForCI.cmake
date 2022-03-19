# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT
#
# Defines utility functions to create build targets for CI.
#

macro(create_per_service_target_build target service runAsSample)

    file(APPEND ${CMAKE_BINARY_DIR}/${service}-targets-build.txt "${target}\n")

    # the binary should be run as a sample as well
    if(${runAsSample})
        SET(binary "${target}")
        if(MSVC)
            SET(binary "${target}.exe")
        endif()
        # Samples are run on Release mode.
        if(CMAKE_GENERATOR MATCHES "Visual Studio.*")
            SET(binary "Release/${binary}")
        endif()
        file(APPEND ${CMAKE_BINARY_DIR}/${service}-samples.txt "${CMAKE_CURRENT_BINARY_DIR}/${binary}\n")
    endif()

endmacro()

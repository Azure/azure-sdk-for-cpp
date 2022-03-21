# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT
#
# Defines utility functions to create build targets for CI.
#

##
# Adds a target to be built as part of a CI run
# - param service: The Azure SDK service group
# - param target:  The name of the cmake target to be built
macro(create_per_service_target_build service target)

    file(APPEND ${CMAKE_BINARY_DIR}/${service}-targets-build.txt "${target}\n")

endmacro()

##
# Adds a target to be built and also run during a CI run
# - param service: The Azure SDK service group
# - param target:  The name of the cmake target to be built
macro(create_per_service_target_build_for_sample  service target)
    
    # Create the built target
    create_per_service_target_build(${service} ${target})

    # Assume the sample to be run on Release mode
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

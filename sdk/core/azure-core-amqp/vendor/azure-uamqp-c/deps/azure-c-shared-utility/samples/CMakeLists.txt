#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required (VERSION 3.5)

function(add_sample_directory whatIsBuilding)
    add_subdirectory(${whatIsBuilding})

    set_target_properties(${whatIsBuilding}
               PROPERTIES
               FOLDER "C-Utility_Samples")
endfunction()

add_sample_directory(iot_c_utility)

if (NOT ("${ARCHITECTURE}" STREQUAL "ARM"))
    add_sample_directory(socketio_connect)
    add_sample_directory(tlsio_connect)
endif()
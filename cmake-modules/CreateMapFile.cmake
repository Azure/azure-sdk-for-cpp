# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
#
# Instructs linker to generate map files and optimize build for minimal size
# Requires CMake version >= 3.13 to use add_link_options

function(create_map_file TARGET_NAME MAP_FILE_NAME)
    if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.13")
        if(MSVC)
            target_link_options(${TARGET_NAME} PRIVATE /MAP)
        elseif(CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
            target_link_options(${TARGET_NAME} PRIVATE LINKER:SHELL:-Wl,-map,${MAP_FILE_NAME})
            target_link_options(${TARGET_NAME} PRIVATE -Os)
        else()
            target_link_options(${TARGET_NAME} PRIVATE -Xlinker -Map=${MAP_FILE_NAME})
            target_link_options(${TARGET_NAME} PRIVATE -Os)
        endif()
    else()
        message("Skipping map file generation because CMake version does not support target_link_options")
    endif()
endfunction()

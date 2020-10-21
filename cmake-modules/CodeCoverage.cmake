# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT
#
# Defines utility functions to create code coverage targets with gcov.
# gcov html and xml report.
#

## SET UP only when option is set. 
if(BUILD_CODE_COVERAGE)
    if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(FATAL_ERROR "Code coverage requires GNU compiler (gcc) and debug build type. ${CMAKE_CXX_COMPILER_ID}")
    endif()

    ## Check if gcov and gcovr are available (gcovr generates html and xml from gcov files)
    find_program(GCOV gcov)
    find_program(GCOVR_PATH gcovr PATHS ${CMAKE_SOURCE_DIR}/scripts/test)
    if(NOT GCOV)
        message(FATAL_ERROR "gcov is required for code coverage reports.")
    endif()
    link_libraries(gcov)
endif()

# Set the compiler flags for the CMake project from where this is called (PARENT_SCOPE)
function(append_code_coverage_for_current_project)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fprofile-arcs -ftest-coverage" PARENT_SCOPE)
endfunction()


# gcovr - html
function(add_gcovr_html)

    set(options NONE)
    set(oneValueArgs TARGET_NAME EXECUTABLE_NAME)
    set(multiValueArgs NA)
    cmake_parse_arguments(args "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(BASEDIR ${PROJECT_SOURCE_DIR})

    add_custom_target(${args_TARGET_NAME}
        # Run tests
        ${args_EXECUTABLE_NAME}

        # Create folder
        COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/${args_TARGET_NAME}

        # Running gcovr
        COMMAND ${GCOVR_PATH} --html --html-details
            -r ${BASEDIR}
            --object-directory=${PROJECT_BINARY_DIR}
            -o ${args_TARGET_NAME}/index.html

        BYPRODUCTS ${PROJECT_BINARY_DIR}/${args_TARGET_NAME}  # report directory
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        DEPENDS
        VERBATIM
        COMMENT "Running gcovr to produce HTML code coverage report."
    )

    # Show info where to find the report
    add_custom_command(TARGET ${args_TARGET_NAME} POST_BUILD
        COMMAND ;
        COMMENT "Open ./${args_TARGET_NAME}/index.html in your browser to view the coverage report."
    )

endfunction()

# gcovr - xml
function(add_gcovr_xml)

    set(options NONE)
    set(oneValueArgs TARGET_NAME EXECUTABLE_NAME)
    set(multiValueArgs NA)
    cmake_parse_arguments(args "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(BASEDIR ${PROJECT_SOURCE_DIR})

    if (NOT DEFINED ENV{AZURE_CI_TEST})
        set(RUN_EXE ${args_EXECUTABLE_NAME})
    endif()
    
    add_custom_target(${args_TARGET_NAME}
        # Running on CI won't require to run tests exe since it was run on previous step
        ${RUN_EXE}

        # Running gcovr
        COMMAND ${GCOVR_PATH} --xml
            -r ${BASEDIR}
            --object-directory=${PROJECT_BINARY_DIR}
            -o ${args_TARGET_NAME}.xml
        BYPRODUCTS ${args_TARGET_NAME}.xml
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        DEPENDS
        VERBATIM
        COMMENT "Running gcovr to produce Cobertura code coverage report."
    )

    # Show info where to find the report
    add_custom_command(TARGET ${args_TARGET_NAME} POST_BUILD
        COMMAND ;
        COMMENT "Cobertura code coverage report saved in ${args_TARGET_NAME}.xml."
    )

endfunction()

# codeCoverage macro to be used from CMake lib definition
macro(create_code_coverage service target_prefix exe_name)
    if(BUILD_CODE_COVERAGE)
        APPEND_CODE_COVERAGE_FOR_CURRENT_PROJECT()

        # HTML and XML - Coverage using gcovr
        add_gcovr_html(TARGET_NAME ${target_prefix}_cov_html EXECUTABLE_NAME ${exe_name})
        # xml is used on CI
        add_gcovr_xml(TARGET_NAME ${target_prefix}_cov_xml EXECUTABLE_NAME ${exe_name})

        # add xml target to `coverage_targets.txt` which is used by CI to generate coverage reports
        file(APPEND ${CMAKE_BINARY_DIR}/${service}-targets-coverage.txt " ${target_prefix}_cov_xml")
    endif()
endmacro()

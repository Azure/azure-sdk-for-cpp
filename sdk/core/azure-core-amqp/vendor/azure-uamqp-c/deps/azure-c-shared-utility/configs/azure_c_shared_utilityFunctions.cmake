#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

function(target_link_libraries_with_arg_prefix arg_prefix whatIsBuilding lib)
    if(${arg_prefix} STREQUAL "debug")
        target_link_libraries(${whatIsBuilding} debug ${lib})
    elseif(${arg_prefix} STREQUAL "optimized")
        target_link_libraries(${whatIsBuilding} optimized ${lib})
    elseif(${arg_prefix} STREQUAL "general")
        target_link_libraries(${whatIsBuilding} general ${lib})
    else()
        target_link_libraries(${whatIsBuilding} ${lib})
    endif()
endfunction()

function(set_test_target_folder whatIsBuilding ext)
    if("${whatIsBuilding}" MATCHES ".*e2e.*")
        set_target_properties(${whatIsBuilding}_${ext}
                   PROPERTIES
                   FOLDER "tests/E2ETests")
    else()
        set_target_properties(${whatIsBuilding}_${ext}
                   PROPERTIES
                   FOLDER "tests/UnitTests")
    endif()
endfunction()

function(windows_unittests_add_dll whatIsBuilding)
    link_directories(${whatIsBuilding}_dll $ENV{VCInstallDir}UnitTest/lib)

    add_library(${whatIsBuilding}_dll SHARED
        ${${whatIsBuilding}_cpp_files}
        ${${whatIsBuilding}_h_files}
        ${${whatIsBuilding}_c_files}
        ${logging_files}
    )

    set_test_target_folder(${whatIsBuilding} "dll")

    set(PARSING_ADDITIONAL_LIBS OFF)
    set(PARSING_VALGRIND_SUPPRESSIONS_FILE OFF)
    set(VALGRIND_SUPPRESSIONS_FILE_EXTRA_PARAMETER)
    set(ARG_PREFIX "none")
    foreach(f ${ARGN})
        set(skip_to_next FALSE)
        if(${f} STREQUAL "ADDITIONAL_LIBS")
            SET(PARSING_ADDITIONAL_LIBS ON)
            SET(PARSING_VALGRIND_SUPPRESSIONS_FILE OFF)
            set(ARG_PREFIX "none")
            #also unset all the other states
            set(skip_to_next TRUE)
        elseif(${f} STREQUAL "VALGRIND_SUPPRESSIONS_FILE")
            SET(PARSING_ADDITIONAL_LIBS OFF)
            SET(PARSING_VALGRIND_SUPPRESSIONS_FILE ON)
            set(skip_to_next TRUE)
        endif()

        if(NOT skip_to_next)
            if(PARSING_ADDITIONAL_LIBS)
                if((${f} STREQUAL "debug") OR (${f} STREQUAL "optimized") OR (${f} STREQUAL "general"))
                    SET(ARG_PREFIX ${f})
                else()
                    target_link_libraries_with_arg_prefix(${ARG_PREFIX} ${whatIsBuilding}_dll ${f})
                    set(ARG_PREFIX "none")
                endif()
            endif()
        endif()

    endforeach()

    target_include_directories(${whatIsBuilding}_dll PUBLIC ${sharedutil_include_directories} $ENV{VCInstallDir}UnitTest/include)
    target_compile_definitions(${whatIsBuilding}_dll PUBLIC -DCPP_UNITTEST)
    target_link_libraries(${whatIsBuilding}_dll micromock_cpp_unittest umock_c ctest testrunnerswitcher)
endfunction()

function(windows_unittests_add_exe whatIsBuilding)
    add_executable(${whatIsBuilding}_exe
        ${${whatIsBuilding}_cpp_files}
        ${${whatIsBuilding}_h_files}
        ${${whatIsBuilding}_c_files}
        ${CMAKE_CURRENT_LIST_DIR}/main.c
        ${logging_files}
    )

    set_test_target_folder(${whatIsBuilding} "exe")

    set(PARSING_ADDITIONAL_LIBS OFF)
    set(PARSING_VALGRIND_SUPPRESSIONS_FILE OFF)
    set(VALGRIND_SUPPRESSIONS_FILE_EXTRA_PARAMETER)
    set(ARG_PREFIX "none")
    foreach(f ${ARGN})
        set(skip_to_next FALSE)
        if(${f} STREQUAL "ADDITIONAL_LIBS")
            SET(PARSING_ADDITIONAL_LIBS ON)
            SET(PARSING_VALGRIND_SUPPRESSIONS_FILE OFF)
            set(ARG_PREFIX "none")
            #also unset all the other states
            set(skip_to_next TRUE)
        elseif(${f} STREQUAL "VALGRIND_SUPPRESSIONS_FILE")
            SET(PARSING_ADDITIONAL_LIBS OFF)
            SET(PARSING_VALGRIND_SUPPRESSIONS_FILE ON)
            set(skip_to_next TRUE)
        endif()

        if(NOT skip_to_next)
            if(PARSING_ADDITIONAL_LIBS)
                if((${f} STREQUAL "debug") OR (${f} STREQUAL "optimized") OR (${f} STREQUAL "general"))
                    SET(ARG_PREFIX ${f})
                else()
                    target_link_libraries_with_arg_prefix(${ARG_PREFIX} ${whatIsBuilding}_exe ${f})
                    set(ARG_PREFIX "none")
                endif()
            endif()
        endif()

    endforeach()

    target_compile_definitions(${whatIsBuilding}_exe PUBLIC -DUSE_CTEST)
    target_include_directories(${whatIsBuilding}_exe PUBLIC ${sharedutil_include_directories})
    target_link_libraries(${whatIsBuilding}_exe micromock_ctest umock_c ctest testrunnerswitcher)
    add_test(NAME ${whatIsBuilding} COMMAND ${whatIsBuilding}_exe)
endfunction()

#this function takes more than the 1 mandatory argument (whatIsBuilding)
#the parameters are separated by "known" separators
#for example, ADDITIONAL_LIBS starts a list of needed libraries
function(linux_unittests_add_exe whatIsBuilding)
    add_executable(${whatIsBuilding}_exe
        ${${whatIsBuilding}_cpp_files}
        ${${whatIsBuilding}_h_files}
        ${${whatIsBuilding}_c_files}
        ${CMAKE_CURRENT_LIST_DIR}/main.c
        ${logging_files}
    )

    set_test_target_folder(${whatIsBuilding} "exe")

    target_compile_definitions(${whatIsBuilding}_exe PUBLIC -DUSE_CTEST)
    target_include_directories(${whatIsBuilding}_exe PUBLIC ${sharedutil_include_directories})

    #this part detects
    #   - the additional libraries that might be needed.
    #     additional libraries are started by ADDITIONAL_LIBS parameter and ended by any other known parameter (or end of variable arguments)
    #   - a valgrind suppression file (VALGRIND_SUPPRESSIONS_FILE) for memcheck
    #     the file name follows immediately after

    set(PARSING_ADDITIONAL_LIBS OFF)
    set(PARSING_VALGRIND_SUPPRESSIONS_FILE OFF)
    set(VALGRIND_SUPPRESSIONS_FILE_EXTRA_PARAMETER)
    set(ARG_PREFIX "none")
    foreach(f ${ARGN})
        set(skip_to_next FALSE)
        if(${f} STREQUAL "ADDITIONAL_LIBS")
            SET(PARSING_ADDITIONAL_LIBS ON)
            SET(PARSING_VALGRIND_SUPPRESSIONS_FILE OFF)
            set(ARG_PREFIX "none")
            set(skip_to_next TRUE)
            #also unset all the other states

        elseif(${f} STREQUAL "VALGRIND_SUPPRESSIONS_FILE")
            SET(PARSING_ADDITIONAL_LIBS OFF)
            SET(PARSING_VALGRIND_SUPPRESSIONS_FILE ON)
            set(skip_to_next TRUE)
        endif()

        if(NOT skip_to_next)
            if(PARSING_ADDITIONAL_LIBS)
                if((${f} STREQUAL "debug") OR (${f} STREQUAL "optimized") OR (${f} STREQUAL "general"))
                    SET(ARG_PREFIX ${f})
                else()
                    target_link_libraries_with_arg_prefix(${ARG_PREFIX} ${whatIsBuilding}_exe ${f})
                    set(ARG_PREFIX "none")
                endif()
            elseif(PARSING_VALGRIND_SUPPRESSIONS_FILE)
                set(VALGRIND_SUPPRESSIONS_FILE_EXTRA_PARAMETER "--suppressions=${f}")
            endif()
        endif()

    endforeach()

    target_link_libraries(${whatIsBuilding}_exe micromock_ctest umock_c ctest)

    add_test(NAME ${whatIsBuilding} COMMAND $<TARGET_FILE:${whatIsBuilding}_exe>)

    if(${run_valgrind})
        find_program(VALGRIND_FOUND NAMES valgrind)
        if(${VALGRIND_FOUND} STREQUAL VALGRIND_FOUND-NOTFOUND)
            message(WARNING "run_valgrind was TRUE, but valgrind was not found - there will be no tests run under valgrind")
        else()
            add_test(NAME ${whatIsBuilding}_valgrind COMMAND valgrind                 --num-callers=100 --error-exitcode=1 --leak-check=full --track-origins=yes ${VALGRIND_SUPPRESSIONS_FILE_EXTRA_PARAMETER} $<TARGET_FILE:${whatIsBuilding}_exe>)
            add_test(NAME ${whatIsBuilding}_helgrind COMMAND valgrind --tool=helgrind --num-callers=100 --error-exitcode=1 ${VALGRIND_SUPPRESSIONS_FILE_EXTRA_PARAMETER} $<TARGET_FILE:${whatIsBuilding}_exe>)
            add_test(NAME ${whatIsBuilding}_drd      COMMAND valgrind --tool=drd      --num-callers=100 --error-exitcode=1 ${VALGRIND_SUPPRESSIONS_FILE_EXTRA_PARAMETER} $<TARGET_FILE:${whatIsBuilding}_exe>)
        endif()
    endif()
endfunction()

function(build_test_artifacts whatIsBuilding use_gballoc)

    #the first argument is what is building
    #the second argument is whether the tests should be build with gballoc #defines or not
    #the following arguments are a list of libraries to link with

    if(${use_gballoc})
        add_definitions(-DGB_MEASURE_MEMORY_FOR_THIS -DGB_DEBUG_ALLOC)
    else()
    endif()

    #setting #defines
    if(WIN32)
        add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    else()
    endif()

    #setting includes
    set(sharedutil_include_directories ${sharedutil_include_directories} ${TESTRUNNERSWITCHER_INCLUDES} ${CTEST_INCLUDES} ${UMOCK_C_INCLUDES} ${AZURE_C_SHARED_UTILITY_INCLUDES})
    set(sharedutil_include_directories ${sharedutil_include_directories} ${MICROMOCK_INC_FOLDER} ${TESTRUNNERSWITCHER_INC_FOLDER} ${CTEST_INC_FOLDER} ${SAL_INC_FOLDER} ${SHARED_UTIL_INC_FOLDER} ${SHARED_UTIL_SRC_FOLDER})
    if(WIN32)
    else()
        include_directories(${sharedutil_include_directories})
    endif()

    #setting logging_files
    add_definitions(-DLOGGER_DISABLE_PAL)
    if(DEFINED SHARED_UTIL_SRC_FOLDER)
        set(logging_files ${XLOGGING_C_FILE} ${LOGGING_C_FILE})
    elseif(DEFINED SHARED_UTIL_FOLDER)
        set(logging_files ${XLOGGING_C_FILE} ${LOGGING_C_FILE})
    else()
        message(FATAL_ERROR "No Shared Utility folder defined for src.")
    endif()

    #setting output type
    if(WIN32)
        if(
            (("${whatIsBuilding}" MATCHES ".*ut.*") AND ${run_unittests}) OR
            (("${whatIsBuilding}" MATCHES ".*e2e.*") AND ${run_e2e_tests}) OR
            (("${whatIsBuilding}" MATCHES ".*int.*") AND ${run_int_tests})
        )
                windows_unittests_add_exe(${whatIsBuilding} ${ARGN})
                if (${use_cppunittest})
                    windows_unittests_add_dll(${whatIsBuilding} ${ARGN})
                endif()
        else()
            if(
                (("${whatIsBuilding}" MATCHES ".*e2e.*") AND ${nuget_e2e_tests})
            )
                windows_unittests_add_exe(${whatIsBuilding}_nuget ${ARGN})
            else()
                #do nothing
            endif()
        endif()
    else()
        if(
            (("${whatIsBuilding}" MATCHES ".*ut.*") AND ${run_unittests}) OR
            (("${whatIsBuilding}" MATCHES ".*e2e.*") AND ${run_e2e_tests}) OR
            (("${whatIsBuilding}" MATCHES ".*int.*") AND ${run_int_tests})
        )
            linux_unittests_add_exe(${whatIsBuilding} ${ARGN})
        endif()
    endif()
endfunction(build_test_artifacts)


function(c_windows_unittests_add_dll whatIsBuilding folder)
    link_directories(${whatIsBuilding}_dll $ENV{VCInstallDir}UnitTest/lib)

    add_library(${whatIsBuilding}_testsonly_lib STATIC
            ${${whatIsBuilding}_test_files}
    )
    SET(VAR 1)
    foreach(file ${${whatIsBuilding}_test_files})
        set_source_files_properties(${file} PROPERTIES COMPILE_FLAGS -DCPPUNITTEST_SYMBOL=some_symbol_for_cppunittest_${VAR})
        MATH(EXPR VAR "${VAR}+1")
    endforeach()

    set_target_properties(${whatIsBuilding}_testsonly_lib
               PROPERTIES
               FOLDER ${folder} )

    target_include_directories(${whatIsBuilding}_testsonly_lib PUBLIC ${sharedutil_include_directories} $ENV{VCInstallDir}UnitTest/include)
    target_compile_definitions(${whatIsBuilding}_testsonly_lib PUBLIC -DCPP_UNITTEST)
    target_compile_options(${whatIsBuilding}_testsonly_lib PUBLIC /TP /EHsc)

    add_library(${whatIsBuilding}_dll SHARED
        ${${whatIsBuilding}_cpp_files}
        ${${whatIsBuilding}_h_files}
        ${${whatIsBuilding}_c_files}
        ${logging_files}
    )

    set_target_properties(${whatIsBuilding}_dll
               PROPERTIES
               FOLDER ${folder})

    set_source_files_properties(${${whatIsBuilding}_c_files} ${logging_files}
               PROPERTIES
               COMPILE_FLAGS /TC)

    set_source_files_properties(${${whatIsBuilding}_cpp_files}
               PROPERTIES
               COMPILE_FLAGS /TP)

    target_link_libraries(${whatIsBuilding}_dll umock_c ctest testrunnerswitcher ${whatIsBuilding}_testsonly_lib )

    set(PARSING_ADDITIONAL_LIBS OFF)
    set(PARSING_VALGRIND_SUPPRESSIONS_FILE OFF)
    set(VALGRIND_SUPPRESSIONS_FILE_EXTRA_PARAMETER)
    set(ARG_PREFIX "none")
    foreach(f ${ARGN})
        set(skip_to_next FALSE)
        if(${f} STREQUAL "ADDITIONAL_LIBS")
            SET(PARSING_ADDITIONAL_LIBS ON)
            SET(PARSING_VALGRIND_SUPPRESSIONS_FILE OFF)
            set(ARG_PREFIX "none")
            #also unset all the other states
            set(skip_to_next TRUE)
        elseif(${f} STREQUAL "VALGRIND_SUPPRESSIONS_FILE")
            SET(PARSING_ADDITIONAL_LIBS OFF)
            SET(PARSING_VALGRIND_SUPPRESSIONS_FILE ON)
            set(skip_to_next TRUE)
        endif()

        if(NOT skip_to_next)
            if(PARSING_ADDITIONAL_LIBS)
                if((${f} STREQUAL "debug") OR (${f} STREQUAL "optimized") OR (${f} STREQUAL "general"))
                    SET(ARG_PREFIX ${f})
                else()
                    target_link_libraries_with_arg_prefix(${ARG_PREFIX} ${whatIsBuilding}_dll ${f})
                    target_link_libraries_with_arg_prefix(${ARG_PREFIX} ${whatIsBuilding}_testsonly_lib ${f})
                    set(ARG_PREFIX "none")
                endif()
            endif()
        endif()

    endforeach()

    SET(SPACES " ")
    SET(VAR 1)
    foreach(file ${${whatIsBuilding}_test_files})
        # for x64 the underscore is not needed
        if (ARCHITECTURE STREQUAL "x86_64" OR ARCHITECTURE STREQUAL "ARM")
            set_property(TARGET ${whatIsBuilding}_dll APPEND_STRING PROPERTY LINK_FLAGS ${SPACES}/INCLUDE:"some_symbol_for_cppunittest_${VAR}")
        else()
            set_property(TARGET ${whatIsBuilding}_dll APPEND_STRING PROPERTY LINK_FLAGS ${SPACES}/INCLUDE:"_some_symbol_for_cppunittest_${VAR}")
        endif()
        MATH(EXPR VAR "${VAR}+1")
    endforeach()
endfunction()

function(c_windows_unittests_add_exe whatIsBuilding folder)
    add_executable(${whatIsBuilding}_exe
        ${${whatIsBuilding}_test_files}
        ${${whatIsBuilding}_cpp_files}
        ${${whatIsBuilding}_h_files}
        ${${whatIsBuilding}_c_files}
        ${CMAKE_CURRENT_LIST_DIR}/main.c
        ${logging_files}
    )
    set_target_properties(${whatIsBuilding}_exe
               PROPERTIES
               FOLDER ${folder})

    target_compile_definitions(${whatIsBuilding}_exe PUBLIC -DUSE_CTEST)
    target_include_directories(${whatIsBuilding}_exe PUBLIC ${sharedutil_include_directories})
    target_link_libraries(${whatIsBuilding}_exe umock_c ctest testrunnerswitcher)

    set(PARSING_ADDITIONAL_LIBS OFF)
    set(PARSING_VALGRIND_SUPPRESSIONS_FILE OFF)
    set(VALGRIND_SUPPRESSIONS_FILE_EXTRA_PARAMETER)
    set(ARG_PREFIX "none")
    foreach(f ${ARGN})
        set(skip_to_next FALSE)
        if(${f} STREQUAL "ADDITIONAL_LIBS")
            SET(PARSING_ADDITIONAL_LIBS ON)
            SET(PARSING_VALGRIND_SUPPRESSIONS_FILE OFF)
            set(ARG_PREFIX "none")
            #also unset all the other states
            set(skip_to_next TRUE)
        elseif(${f} STREQUAL "VALGRIND_SUPPRESSIONS_FILE")
            SET(PARSING_ADDITIONAL_LIBS OFF)
            SET(PARSING_VALGRIND_SUPPRESSIONS_FILE ON)
            set(skip_to_next TRUE)
        endif()

        if(NOT skip_to_next)
            if(PARSING_ADDITIONAL_LIBS)
                if((${f} STREQUAL "debug") OR (${f} STREQUAL "optimized") OR (${f} STREQUAL "general"))
                    SET(ARG_PREFIX ${f})
                else()
                    target_link_libraries_with_arg_prefix(${ARG_PREFIX} ${whatIsBuilding}_exe ${f})
                    set(ARG_PREFIX "none")
                endif()
            endif()
        endif()

    endforeach()

    add_test(NAME ${whatIsBuilding} COMMAND ${whatIsBuilding}_exe)
endfunction()

#this function takes more than the 1 mandatory argument (whatIsBuilding)
#the parameters are separated by "known" separators
#for example, ADDITIONAL_LIBS starts a list of needed libraries
function(c_linux_unittests_add_exe whatIsBuilding folder)

    add_executable(${whatIsBuilding}_exe
        ${${whatIsBuilding}_test_files}
        ${${whatIsBuilding}_cpp_files}
        ${${whatIsBuilding}_h_files}
        ${${whatIsBuilding}_c_files}
        ${CMAKE_CURRENT_LIST_DIR}/main.c
        ${logging_files}
    )
    set_target_properties(${whatIsBuilding}_exe
               PROPERTIES
               FOLDER ${folder})

    target_compile_definitions(${whatIsBuilding}_exe PUBLIC -DUSE_CTEST)
    target_include_directories(${whatIsBuilding}_exe PUBLIC ${sharedutil_include_directories})

    #this part detects
    #       - the additional libraries that might be needed.
    #     additional libraries are started by ADDITIONAL_LIBS parameter and ended by any other known parameter (or end of variable arguments)
    #   - a valgrind suppression file (VALGRIND_SUPPRESSIONS_FILE) for memcheck
    #     the file name follows immediately after

    set(PARSING_ADDITIONAL_LIBS OFF)
    set(PARSING_VALGRIND_SUPPRESSIONS_FILE OFF)
    set(VALGRIND_SUPPRESSIONS_FILE_EXTRA_PARAMETER)
    set(ARG_PREFIX "none")
    foreach(f ${ARGN})
        set(skip_to_next FALSE)
        if(${f} STREQUAL "ADDITIONAL_LIBS")
            SET(PARSING_ADDITIONAL_LIBS ON)
            SET(PARSING_VALGRIND_SUPPRESSIONS_FILE OFF)
            set(ARG_PREFIX "none")
            #also unset all the other states
            set(skip_to_next TRUE)
        elseif(${f} STREQUAL "VALGRIND_SUPPRESSIONS_FILE")
            SET(PARSING_ADDITIONAL_LIBS OFF)
            SET(PARSING_VALGRIND_SUPPRESSIONS_FILE ON)
            set(skip_to_next TRUE)
        endif()

        if(NOT skip_to_next)
            if(PARSING_ADDITIONAL_LIBS)
                if((${f} STREQUAL "debug") OR (${f} STREQUAL "optimized") OR (${f} STREQUAL "general"))
                    SET(ARG_PREFIX ${f})
                else()
                    target_link_libraries_with_arg_prefix(${ARG_PREFIX} ${whatIsBuilding}_exe ${f})
                    set(ARG_PREFIX "none")
                endif()
            elseif(PARSING_VALGRIND_SUPPRESSIONS_FILE)
                set(VALGRIND_SUPPRESSIONS_FILE_EXTRA_PARAMETER "--suppressions=${f}")
            endif()
        endif()

    endforeach()

    target_link_libraries(${whatIsBuilding}_exe umock_c ctest m)

    add_test(NAME ${whatIsBuilding} COMMAND $<TARGET_FILE:${whatIsBuilding}_exe>)

    if(${run_valgrind})
        find_program(VALGRIND_FOUND NAMES valgrind)
        if(${VALGRIND_FOUND} STREQUAL VALGRIND_FOUND-NOTFOUND)
            message(WARNING "run_valgrind was TRUE, but valgrind was not found - there will be no tests run under valgrind")
        else()
            add_test(NAME ${whatIsBuilding}_valgrind COMMAND valgrind                 --gen-suppressions=all --num-callers=100 --error-exitcode=1 --leak-check=full --track-origins=yes ${VALGRIND_SUPPRESSIONS_FILE_EXTRA_PARAMETER} $<TARGET_FILE:${whatIsBuilding}_exe>)
            add_test(NAME ${whatIsBuilding}_helgrind COMMAND valgrind --tool=helgrind --gen-suppressions=all --num-callers=100 --error-exitcode=1 ${VALGRIND_SUPPRESSIONS_FILE_EXTRA_PARAMETER} $<TARGET_FILE:${whatIsBuilding}_exe>)
            add_test(NAME ${whatIsBuilding}_drd      COMMAND valgrind --tool=drd      --gen-suppressions=all --num-callers=100 --error-exitcode=1 ${VALGRIND_SUPPRESSIONS_FILE_EXTRA_PARAMETER} $<TARGET_FILE:${whatIsBuilding}_exe>)
        endif()
    endif()
endfunction()

function(build_c_test_artifacts whatIsBuilding use_gballoc folder)

    #the first argument is what is building
    #the second argument is whether the tests should be build with gballoc #defines or not
    #the following arguments are a list of libraries to link with

    if(${use_gballoc})
        add_definitions(-DGB_MEASURE_MEMORY_FOR_THIS -DGB_DEBUG_ALLOC)
    else()
    endif()

    #setting #defines
    if(WIN32)
        add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    else()
    endif()

    #setting includes
    set(sharedutil_include_directories ${sharedutil_include_directories} ${TESTRUNNERSWITCHER_INCLUDES} ${CTEST_INCLUDES} ${UMOCK_C_INCLUDES} ${AZURE_C_SHARED_UTILITY_INCLUDES})
    set(sharedutil_include_directories ${sharedutil_include_directories} ${MICROMOCK_INC_FOLDER} ${UMOCK_C_INC_FOLDER} ${TESTRUNNERSWITCHER_INC_FOLDER} ${CTEST_INC_FOLDER} ${SAL_INC_FOLDER} ${SHARED_UTIL_INC_FOLDER} ${SHARED_UTIL_SRC_FOLDER})
    if(WIN32)
    else()
        include_directories(${sharedutil_include_directories})
    endif()

    add_definitions(-DLOGGER_DISABLE_PAL)

    #setting logging_files
    if(DEFINED SHARED_UTIL_SRC_FOLDER)
        set(logging_files ${XLOGGING_C_FILE} ${LOGGING_C_FILE})
    elseif(DEFINED SHARED_UTIL_FOLDER)
        set(logging_files ${XLOGGING_C_FILE} ${LOGGING_C_FILE})
    else()
        message(FATAL_ERROR "No Shared Utility folder defined for includes/src.")
    endif()

    #setting output type
    if(WIN32)
        if(
            (("${whatIsBuilding}" MATCHES ".*ut.*") AND ${run_unittests}) OR
            (("${whatIsBuilding}" MATCHES ".*e2e.*") AND ${run_e2e_tests}) OR
            (("${whatIsBuilding}" MATCHES ".*int.*") AND ${run_int_tests})
        )
            if (${use_cppunittest})
                c_windows_unittests_add_dll(${whatIsBuilding} ${folder} ${ARGN})
            endif()
            c_windows_unittests_add_exe(${whatIsBuilding} ${folder} ${ARGN})
        else()
            if(
                (("${whatIsBuilding}" MATCHES ".*e2e.*") AND ${nuget_e2e_tests})
            )
                c_windows_unittests_add_exe(${whatIsBuilding}_nuget ${folder} ${ARGN})
            else()
                #do nothing
            endif()
        endif()
    else()
        if(
            (("${whatIsBuilding}" MATCHES ".*ut.*") AND ${run_unittests}) OR
            (("${whatIsBuilding}" MATCHES ".*e2e.*") AND ${run_e2e_tests}) OR
            (("${whatIsBuilding}" MATCHES ".*int.*") AND ${run_int_tests})
        )
            c_linux_unittests_add_exe(${whatIsBuilding} ${folder} ${ARGN})
        endif()
    endif()
endfunction()

function(compile_c_test_artifacts_as whatIsBuilding compileAsWhat)

    if(WIN32)
        if(
            (("${whatIsBuilding}" MATCHES ".*ut.*") AND ${run_unittests}) OR
            (("${whatIsBuilding}" MATCHES ".*e2e.*") AND ${run_e2e_tests}) OR
            (("${whatIsBuilding}" MATCHES ".*int.*") AND ${run_int_tests})
        )
            if (${use_cppunittest})
                if(${compileAsWhat} STREQUAL "C99")
                    compileTargetAsC99(${whatIsBuilding}_dll)
                    compileTargetAsC99(${whatIsBuilding}_testsonly_lib)
                endif()
                if(${compileAsWhat} STREQUAL "C11")
                    compileTargetAsC11(${whatIsBuilding}_dll)
                    compileTargetAsC11(${whatIsBuilding}_testsonly_lib)
                endif()
            endif()
            if(${compileAsWhat} STREQUAL "C99")
                compileTargetAsC99(${whatIsBuilding}_exe)
            endif()
            if(${compileAsWhat} STREQUAL "C11")
                compileTargetAsC11(${whatIsBuilding}_exe)
            endif()
        else()
            if(
                (("${whatIsBuilding}" MATCHES ".*e2e.*") AND ${nuget_e2e_tests})
            )
                if(${compileAsWhat} STREQUAL "C99")
                    compileTargetAsC99(${whatIsBuilding}_exe)
                endif()
                if(${compileAsWhat} STREQUAL "C11")
                    compileTargetAsC11(${whatIsBuilding}_exe)
                endif()
            else()
                #do nothing
            endif()
        endif()
    else()
        if(
            (("${whatIsBuilding}" MATCHES ".*ut.*") AND ${run_unittests}) OR
            (("${whatIsBuilding}" MATCHES ".*e2e.*") AND ${run_e2e_tests}) OR
            (("${whatIsBuilding}" MATCHES ".*int.*") AND ${run_int_tests})
        )
            if(${compileAsWhat} STREQUAL "C99")
                compileTargetAsC99(${whatIsBuilding}_exe)
            endif()
            if(${compileAsWhat} STREQUAL "C11")
                compileTargetAsC11(${whatIsBuilding}_exe)
            endif()
        endif()
    endif()
endfunction()

function(build_c_test_longhaul_test test_name test_c_files test_h_files)
    set(test_c_files
        ${CMAKE_CURRENT_LIST_DIR}/../common_longhaul/iothub_client_statistics.c
        ${CMAKE_CURRENT_LIST_DIR}/../common_longhaul/iothub_client_common_longhaul.c
        ${test_c_files}
    )

    set(test_h_files
        ${CMAKE_CURRENT_LIST_DIR}/../common_longhaul/iothub_client_statistics.h
        ${CMAKE_CURRENT_LIST_DIR}/../common_longhaul/iothub_client_common_longhaul.h
        ${test_h_files}
    )

    IF(WIN32)
        #windows needs this define
        add_definitions(-D_CRT_SECURE_NO_WARNINGS)
        add_definitions(-DGB_MEASURE_MEMORY_FOR_THIS -DGB_DEBUG_ALLOC)
    ENDIF(WIN32)

    #Conditionally use the SDK trusted certs in the samples
    if(${set_trusted_cert_in_samples})
        add_definitions(-DSET_TRUSTED_CERT_IN_SAMPLES)
        include_directories(${PROJECT_SOURCE_DIR}/certs)
        set(samples_cert_file ${PROJECT_SOURCE_DIR}/certs/certs.c)
    endif()

    include_directories(${IOTHUB_TEST_INC_FOLDER})
    include_directories(${IOTHUB_SERVICE_CLIENT_INC_FOLDER})
    include_directories(.)

    add_executable(${test_name} ${test_c_files} ${test_h_files} ${samples_cert_file})

    set(PARSING_VALGRIND_SUPPRESSIONS_FILE OFF)
    set(VALGRIND_SUPPRESSIONS_FILE_EXTRA_PARAMETER)
    foreach(f ${ARGN})
        set(skip_to_next FALSE)
        if(${f} STREQUAL "VALGRIND_SUPPRESSIONS_FILE")
            SET(PARSING_VALGRIND_SUPPRESSIONS_FILE ON)
            set(skip_to_next TRUE)
        endif()

        if(NOT skip_to_next)
            if(PARSING_VALGRIND_SUPPRESSIONS_FILE)
                set(VALGRIND_SUPPRESSIONS_FILE_EXTRA_PARAMETER "--suppressions=${f}")
            endif()
        endif()
    endforeach()

    if(${run_valgrind})
        find_program(VALGRIND_FOUND NAMES valgrind)
        if(${VALGRIND_FOUND} STREQUAL VALGRIND_FOUND-NOTFOUND)
            message(WARNING "run_valgrind was TRUE, but valgrind was not found - there will be no tests run under valgrind")
        else()
            # below are the longhaul tests running under valgrind
            # These are currently disabled because the longhaul containers are still Ubuntu 16 andd need s to upgraded to Ubuntu 18

            #add_test(NAME ${test_name}_valgrind COMMAND valgrind                 --gen-suppressions=all --num-callers=100 --error-exitcode=1 --leak-check=full --track-origins=yes ${VALGRIND_SUPPRESSIONS_FILE_EXTRA_PARAMETER} $<TARGET_FILE:${test_name}>)
            #add_test(NAME ${test_name}_helgrind COMMAND valgrind --tool=helgrind --gen-suppressions=all --num-callers=100 --error-exitcode=1 ${VALGRIND_SUPPRESSIONS_FILE_EXTRA_PARAMETER} $<TARGET_FILE:${test_name}>)
            #add_test(NAME ${test_name}_drd      COMMAND valgrind --tool=drd      --gen-suppressions=all --num-callers=100 --error-exitcode=1 ${VALGRIND_SUPPRESSIONS_FILE_EXTRA_PARAMETER} $<TARGET_FILE:${test_name}>)
            #set_tests_properties(${test_name}_valgrind PROPERTIES TIMEOUT 1296000)
            #set_tests_properties(${test_name}_helgrind PROPERTIES TIMEOUT 1296000)
            #set_tests_properties(${test_name}_drd PROPERTIES TIMEOUT 1296000)

            # run the lonhaul without using valgrind
            add_test(NAME ${test_name} COMMAND $<TARGET_FILE:${test_name}>)
            set_tests_properties(${test_name} PROPERTIES TIMEOUT 1296000)
        endif()
    else()
        add_test(NAME ${test_name} COMMAND $<TARGET_FILE:${test_name}>)
        set_tests_properties(${test_name} PROPERTIES TIMEOUT 1296000)
    endif()

    target_link_libraries(${test_name}
            iothub_test
            iothub_client)

    set_target_properties(${test_name} PROPERTIES FOLDER "tests/LongHaul")
endfunction()

# This function focuses on setting files which are unique to a given hardware platform.
# The choice of tlsio is not unique per-platform, and is set in the main CMakeLists.txt
function(set_platform_files c_shared_dir)
    if(WIN32)
        if(${use_condition})
            set(CONDITION_C_FILE ${c_shared_dir}/adapters/condition_win32.c PARENT_SCOPE)
        endif()
        
        if(${use_etw} STREQUAL "OFF")
            set(XLOGGING_C_FILE ${c_shared_dir}/src/xlogging.c PARENT_SCOPE)
            set(LOGGING_C_FILE ${c_shared_dir}/src/consolelogger.c PARENT_SCOPE)
            set(LOGGING_H_FILE ${c_shared_dir}/inc/azure_c_shared_utility/consolelogger.h PARENT_SCOPE)
        elseif(${use_etw} STREQUAL "TRACELOGGING")
            set(XLOGGING_C_FILE ${c_shared_dir}/src/etwxlogging.c PARENT_SCOPE)
            set(LOGGING_C_FILE ${c_shared_dir}/src/etwlogger_driver.c PARENT_SCOPE)
            set(LOGGING_H_FILE ${c_shared_dir}/inc/azure_c_shared_utility/etwlogger_driver.h PARENT_SCOPE)
        elseif(${use_etw} STREQUAL "TRACELOGGING_WITH_CONSOLE")
            set(XLOGGING_C_FILE ${c_shared_dir}/src/etwxlogging.c PARENT_SCOPE)
            set(LOGGING_C_FILE ${c_shared_dir}/src/etwlogger_driver.c ${c_shared_dir}/src/consolelogger.c PARENT_SCOPE)
            set(LOGGING_H_FILE ${c_shared_dir}/inc/azure_c_shared_utility/etwlogger_driver.h ${c_shared_dir}/inc/azure_c_shared_utility/consolelogger.h PARENT_SCOPE)
        endif()

        set(THREAD_C_FILE ${c_shared_dir}/adapters/threadapi_win32.c PARENT_SCOPE)
        set(LOCK_C_FILE ${c_shared_dir}/adapters/lock_win32.c PARENT_SCOPE)
        set(SRW_LOCK_C_FILE ${c_shared_dir}/adapters/srw_lock.c PARENT_SCOPE)
        set(TIMER_C_FILE ${c_shared_dir}/adapters/timer.c PARENT_SCOPE)
        set(STRING_UTILS_C_FILE ${c_shared_dir}/adapters/string_utils.c PARENT_SCOPE)
        
        if (${use_builtin_httpapi})
            set(HTTP_C_FILE ${c_shared_dir}/adapters/httpapi_compact.c PARENT_SCOPE)
        else()
            set(HTTP_C_FILE ${c_shared_dir}/adapters/httpapi_winhttp.c PARENT_SCOPE)
        endif()
        set(PLATFORM_C_FILE ${c_shared_dir}/adapters/platform_win32.c PARENT_SCOPE)
        if (${use_socketio})
            set(SOCKETIO_C_FILE ${c_shared_dir}/adapters/socketio_win32.c PARENT_SCOPE)
        endif()
        set(TICKCOUTER_C_FILE ${c_shared_dir}/adapters/tickcounter_win32.c PARENT_SCOPE)
        if (${use_default_uuid})
            set(UNIQUEID_C_FILE ${c_shared_dir}/adapters/uniqueid_stub.c PARENT_SCOPE)
        else()
            set(UNIQUEID_C_FILE ${c_shared_dir}/adapters/uniqueid_win32.c PARENT_SCOPE)
        endif()
        set(DNS_C_FILE ${c_shared_dir}/src/dns_resolver_sync.c PARENT_SCOPE)
    else()
        set(XLOGGING_C_FILE ${c_shared_dir}/src/xlogging.c PARENT_SCOPE)
        set(LOGGING_C_FILE ${c_shared_dir}/src/consolelogger.c PARENT_SCOPE)
        set(LOGGING_H_FILE ${c_shared_dir}/inc/azure_c_shared_utility/consolelogger.h PARENT_SCOPE)
        
        if(${use_condition})
            set(CONDITION_C_FILE ${c_shared_dir}/adapters/condition_pthreads.c PARENT_SCOPE)
        endif()

        if (${use_builtin_httpapi})
            set(HTTP_C_FILE ${c_shared_dir}/adapters/httpapi_compact.c PARENT_SCOPE)
        else()
            set(HTTP_C_FILE ${c_shared_dir}/adapters/httpapi_curl.c PARENT_SCOPE)
        endif()
        set(LOCK_C_FILE ${c_shared_dir}/adapters/lock_pthreads.c PARENT_SCOPE)
        if (use_applessl)
            set(PLATFORM_C_FILE ${c_shared_dir}/pal/ios-osx/platform_appleios.c PARENT_SCOPE)
        else()
            set(PLATFORM_C_FILE ${c_shared_dir}/adapters/platform_linux.c PARENT_SCOPE)
        endif()
        if (${use_socketio})
            set(SOCKETIO_C_FILE ${c_shared_dir}/adapters/socketio_berkeley.c PARENT_SCOPE)
        endif()
        set(THREAD_C_FILE ${c_shared_dir}/adapters/threadapi_pthreads.c PARENT_SCOPE)
        set(TICKCOUTER_C_FILE ${c_shared_dir}/adapters/tickcounter_linux.c PARENT_SCOPE)
        if (${use_default_uuid})
            set(UNIQUEID_C_FILE ${c_shared_dir}/adapters/uniqueid_stub.c PARENT_SCOPE)
        elseif(AZSPHERE)
            set(UNIQUEID_C_FILE ${c_shared_dir}/adapters/uniqueid_azsphere.c PARENT_SCOPE)
        else()
            set(UNIQUEID_C_FILE ${c_shared_dir}/adapters/uniqueid_linux.c PARENT_SCOPE)
        endif()
        if(${use_c_ares})
            set(DNS_C_FILE ${c_shared_dir}/src/dns_resolver_ares.c PARENT_SCOPE)
        else()
            set(DNS_C_FILE ${c_shared_dir}/src/dns_resolver_sync.c PARENT_SCOPE)
        endif()
    endif()
    
    if(WIN32 OR MACOSX OR LINUX)
        set (ENVIRONMENT_VARIABLE_C_FILE ${c_shared_dir}/adapters/envvariable.c PARENT_SCOPE)
    endif()
endfunction(set_platform_files)

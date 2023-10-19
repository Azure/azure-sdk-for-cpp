#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

set(exeName "min_nolog_${no_logging_value}_stk_${logerror_captures_stack_traces_value}_etw_${use_etw_value}")

if(${no_logging_value})
    add_definitions(-DNO_LOGGING)
endif()

if(${logerror_captures_stack_traces_value})
    #set a preprocessor option (LOGERROR_CAPTURES_STACK_TRACES) and add the files that provide the functionality
    add_definitions(-DLOGERROR_CAPTURES_STACK_TRACES)
    set(LOGGING_STACKTRACE_C_FILE  ../../../src/logging_stacktrace.c)
    set(LOGGING_STACKTRACE_H_FILE  ../../../inc/azure_c_shared_utility/logging_stacktrace.h)
else()
    set(LOGGING_STACKTRACE_C_FILE)
    set(LOGGING_STACKTRACE_H_FILE)
endif()

    add_definitions(-DLOGGER_DISABLE_PAL)
if (${use_etw} STREQUAL "TRACELOGGING")
    add_definitions(-DUSE_TRACELOGGING)
endif()

if (${use_etw} STREQUAL "TRACELOGGING_WITH_CONSOLE")
    add_definitions(-DUSE_TRACELOGGING)
    add_definitions(-DCALL_CONSOLE_LOGGER)
endif()

if(${use_etw} STREQUAL "OFF")
        set(XLOGGING_C_FILE ../../../src/xlogging.c PARENT_SCOPE)
        set(LOGGING_C_FILE ../../../src/consolelogger.c PARENT_SCOPE)
        set(LOGGING_H_FILE ../../../inc/azure_c_shared_utility/consolelogger.h PARENT_SCOPE)
    elseif(${use_etw} STREQUAL "TRACELOGGING")
        set(XLOGGING_C_FILE ../../../src/etwxlogging.c PARENT_SCOPE)
        set(LOGGING_C_FILE ../../../src/etwlogger_driver.c PARENT_SCOPE)
        set(LOGGING_H_FILE ../../../inc/azure_c_shared_utility/etwlogger_driver.h PARENT_SCOPE)
    elseif(${use_etw} STREQUAL "TRACELOGGING_WITH_CONSOLE")
        set(XLOGGING_C_FILE ../../../src/etwxlogging.c PARENT_SCOPE)
        set(LOGGING_C_FILE ../../../src/etwlogger_driver.c ../../../src/consolelogger.c PARENT_SCOPE)
        set(LOGGING_H_FILE ../../../inc/azure_c_shared_utility/etwlogger_driver.h ../../../inc/azure_c_shared_utility/consolelogger.h PARENT_SCOPE)
    endif()


add_executable(${exeName} 
    ${LOGGING_STACKTRACE_C_FILE} 
    ${LOGGING_STACKTRACE_H_FILE} 
    ${XLOGGING_C_FILE}
    ${LOGGING_C_FILE}
    ${LOGGING_H_FILE}
    ${LOGGING_RC_FILE}
    minimal.c)

if(${logerror_captures_stack_traces_value})
    target_link_libraries(${exeName} Dbghelp.lib)
endif()

set_target_properties(${exeName} PROPERTIES FOLDER "tests/azure_c_shared_utility_tests")
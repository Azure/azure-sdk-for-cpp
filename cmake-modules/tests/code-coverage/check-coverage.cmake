# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
# cspell: words gcda

set(OBJECT_DIR "${TEST_BINARY_DIR}/CMakeFiles/coverage-test.dir")
file(REMOVE "${OBJECT_DIR}/coverage_test.cpp.gcda")

execute_process(COMMAND "${TEST_EXECUTABLE}" RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "The multithreaded coverage executable failed: ${result}")
endif()

execute_process(
    COMMAND "${GCOV}" -b -c -o "${OBJECT_DIR}/coverage_test.cpp.o" "${TEST_SOURCE_DIR}/coverage_test.cpp"
    WORKING_DIRECTORY "${TEST_BINARY_DIR}"
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "gcov failed: ${result}")
endif()

file(READ "${TEST_BINARY_DIR}/coverage_test.cpp.gcov" coverage)
string(REGEX MATCH "function CountedBranch called ([0-9]+)" function_count "${coverage}")
if(NOT function_count OR NOT CMAKE_MATCH_1 STREQUAL "1600000")
    message(FATAL_ERROR "Expected 1600000 recorded calls; got '${function_count}'.")
endif()

execute_process(
    COMMAND "${GCOVR}" -r "${TEST_SOURCE_DIR}" --object-directory "${TEST_BINARY_DIR}"
        --xml -o "${TEST_BINARY_DIR}/coverage.xml"
    WORKING_DIRECTORY "${TEST_BINARY_DIR}"
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "gcovr could not parse the coverage data: ${result}")
endif()

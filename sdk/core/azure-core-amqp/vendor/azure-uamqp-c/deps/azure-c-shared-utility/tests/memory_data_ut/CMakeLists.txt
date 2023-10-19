#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

set(theseTestsName memory_data_ut)

generate_cppunittest_wrapper(${theseTestsName})

set(${theseTestsName}_c_files
../../src/memory_data.c
)

set(${theseTestsName}_h_files
)

file(COPY ../valgrind_suppressions.supp DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
build_c_test_artifacts(${theseTestsName} ON "tests/azure_c_shared_utility_tests"
  ADDITIONAL_LIBS
    aziotsharedutil
  VALGRIND_SUPPRESSIONS_FILE
    valgrind_suppressions.supp
)

compile_c_test_artifacts_as(${theseTestsName} C99)

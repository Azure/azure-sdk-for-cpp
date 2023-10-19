#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

set(theseTestsName local_client_server_tcp_e2e)
set(${theseTestsName}_test_files
${theseTestsName}.c
)

set(${theseTestsName}_c_files
)

set(${theseTestsName}_h_files
)

file(COPY ../valgrind_suppressions.supp DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
build_c_test_artifacts(${theseTestsName} ON "tests/uamqp_tests"
  ADDITIONAL_LIBS
    uamqp
  VALGRIND_SUPPRESSIONS_FILE
    valgrind_suppressions.supp
)

compile_c_test_artifacts_as(${theseTestsName} C99)

#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

set(theseTestsName sasl_server_mechanism_ut)

set(${theseTestsName}_test_files
${theseTestsName}.c
)

set(${theseTestsName}_c_files
../../src/sasl_server_mechanism.c
)

set(${theseTestsName}_h_files
)

build_c_test_artifacts(${theseTestsName} ON "tests/uamqp_tests")

compile_c_test_artifacts_as(${theseTestsName} C99)

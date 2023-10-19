#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required (VERSION 3.5)

set(theseTestsName x509_schannel_int)

generate_cppunittest_wrapper(${theseTestsName})

set(${theseTestsName}_c_files
)

set(${theseTestsName}_h_files
)

build_c_test_artifacts(${theseTestsName} ON "tests/azure_c_shared_utility_tests")

compile_c_test_artifacts_as(${theseTestsName} C99)

if(TARGET ${theseTestsName}_dll)
    target_link_libraries(${theseTestsName}_dll crypt32 Ncrypt aziotsharedutil)
endif()

if(TARGET ${theseTestsName}_exe)
    target_link_libraries(${theseTestsName}_exe crypt32 Ncrypt aziotsharedutil)
endif()



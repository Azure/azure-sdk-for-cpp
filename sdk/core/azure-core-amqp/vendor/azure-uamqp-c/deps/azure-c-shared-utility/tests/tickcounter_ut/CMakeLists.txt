#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required (VERSION 3.5)

set(theseTestsName tickcounter_ut)

generate_cppunittest_wrapper(${theseTestsName})

set(${theseTestsName}_c_files
	${TICKCOUTER_C_FILE}
)

if(UNIX) # linux & apple
    set(${theseTestsName}_c_files ${${theseTestsName}_c_files}
        ../../adapters/linux_time.c
    )
endif()

set(${theseTestsName}_h_files
)

build_c_test_artifacts(${theseTestsName} ON "tests/azure_c_shared_utility_tests")

compile_c_test_artifacts_as(${theseTestsName} C99)

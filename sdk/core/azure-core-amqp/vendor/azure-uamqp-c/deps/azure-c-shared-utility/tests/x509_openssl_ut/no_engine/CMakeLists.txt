#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required (VERSION 3.5)

set(theseTestsName x509_openssl_ut_no_engine)

set(${theseTestsName}_test_files
../x509_openssl_ut.c
)

set(${theseTestsName}_c_files
../../../adapters/x509_openssl.c
)

set(${theseTestsName}_h_files
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DOPENSSL_NO_ENGINE")

build_c_test_artifacts(${theseTestsName} ON "tests/azure_c_shared_utility_tests")

compile_c_test_artifacts_as(${theseTestsName} C99)

#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required (VERSION 3.5)

set(theseTestsName x509_openssl_ut)

set(${theseTestsName}_test_files
${theseTestsName}.c
)

if(LINUX)
  set_property(
    SOURCE
      ../../adapters/x509_openssl.c
    PROPERTY COMPILE_OPTIONS
      -Wno-deprecated-declarations
  )
endif()

set(${theseTestsName}_c_files
../../adapters/x509_openssl.c
)

set(${theseTestsName}_h_files
)

build_c_test_artifacts(${theseTestsName} ON "tests/azure_c_shared_utility_tests")

compile_c_test_artifacts_as(${theseTestsName} C99)

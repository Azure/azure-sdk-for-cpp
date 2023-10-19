#Copyright (c) Microsoft. All rights reserved.

set(theseTestsName string_utils_int)

generate_cppunittest_wrapper(${theseTestsName})

set(${theseTestsName}_c_files
)

set(${theseTestsName}_cpp_files
    nothing.cpp
)

set(${theseTestsName}_h_files
)

build_c_test_artifacts(${theseTestsName} ON "tests/azure_c_shared_utility_tests" ADDITIONAL_LIBS aziotsharedutil)

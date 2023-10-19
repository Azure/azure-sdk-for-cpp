#Copyright (c) Microsoft. All rights reserved.

#this is CMakeLists.txt for macro_utils_int_tests
#since every single other project (testrunnerswitcher, ctest, umock_c etc) uses macro_utils
#it means that macro_utils has its own test infrastructure (cannot call into build_c_test_artifacts)

include_directories(${MACRO_UTILS_INC_FOLDER})

set(macro_utils_int_tests_c_files
    mu_define_enum_2_without_invalid_test.c
    define_struct_test.c
    for_each_1_keep_2_test.c
    mu_eat_empty_args_test.c
    mu_count_array_items_test.c
    mu_define_enum_test.c
    mu_define_local_enum_test.c
    mu_define_enum_2_test.c
    mu_pri_enum_test.c
    main.c
)

set(macro_utils_int_tests_h_files
    test_helper.h
    ${MACRO_UTILS_INC_FOLDER}/azure_macro_utils/macro_utils.h
    ${MACRO_UTILS_INC_FOLDER}/azure_macro_utils/macro_utils_generated.h
    mu_eat_empty_args_test.h
    mu_define_enum_2_without_invalid_test.h
    define_struct_test.h
    for_each_1_keep_2_test.h
    mu_count_array_items_test.h
    mu_define_enum_test.h
    mu_define_local_enum_test.h
    mu_define_enum_2_test.h
    mu_pri_enum_test.h
)
    
add_executable(macro_utils_int_tests ${macro_utils_int_tests_c_files} ${macro_utils_int_tests_h_files})

add_test(NAME macro_utils_int_tests COMMAND macro_utils_int_tests)

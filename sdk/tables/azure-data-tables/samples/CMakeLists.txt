# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

if(MSVC)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

add_executable(tables-getting-started tables_getting_started.cpp )
target_link_libraries(tables-getting-started PRIVATE azure-data-tables get-env-helper)
create_per_service_target_build_for_sample(tables tables-getting-started)

add_executable(tables_entity_operations tables_entity_operations.cpp)
target_link_libraries(tables_entity_operations PRIVATE azure-data-tables get-env-helper)
create_per_service_target_build_for_sample(tables tables_entity_operations)

add_executable(tables_service_operations tables_service_operations.cpp)
target_link_libraries(tables_service_operations PRIVATE azure-data-tables get-env-helper)
create_per_service_target_build_for_sample(tables tables_service_operations)

add_executable(tables_transactions_ok tables_transactions_ok.cpp)
target_link_libraries(tables_transactions_ok PRIVATE azure-data-tables get-env-helper)
create_per_service_target_build_for_sample(tables tables_transactions_ok)

add_executable(tables_transactions_fail tables_transactions_fail.cpp)
target_link_libraries(tables_transactions_fail PRIVATE azure-data-tables get-env-helper)
create_per_service_target_build_for_sample(tables tables_transactions_fail)

# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

az_build_shared_lib_test(azure-core CORE)

# build information for test projects
if(BUILD_TESTING)

 
  if (NOT AZ_ALL_LIBRARIES)
    include(AddGoogleTest)
    enable_testing ()
  endif()
  
  add_subdirectory(ut)
  if(DEFINED ENV{AZURE_CORE_ENABLE_JSON_TESTS})
    add_subdirectory(nlohmann-json-test)
  endif()
  add_subdirectory(fault-injector)
  
  if(BUILD_TRANSPORT_CURL)
    add_subdirectory(libcurl-stress-test)
  endif()
endif()

if (BUILD_PERFORMANCE_TESTS)
  add_subdirectory(perf)
endif()

#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#this is CMakeLists for micromock
cmake_minimum_required (VERSION 3.5)

set(micromock_cpp_files
./src/micromockcharstararenullterminatedstrings.cpp
./src/micromockexception.cpp
./src/micromocktestmutex.cpp
./src/mockcallrecorder.cpp
./src/mockmethodcallbase.cpp
./src/mockvaluebase.cpp
./src/timediscretemicromock.cpp
)

set(micromock_h_files
./inc/globalmock.h
./inc/micromock.h
./inc/micromockcallmacros.h
./inc/micromockcharstararenullterminatedstrings.h
./inc/micromockcommon.h
./inc/micromockenumtostring.h
./inc/micromockexception.h
./inc/micromocktestmutex.h
./inc/micromocktestrunnerhooks.h
./inc/mock.h
./inc/mockcallargument.h
./inc/mockcallargumentbase.h
./inc/mockcallcomparer.h
./inc/mockcallrecorder.h
./inc/mockmethodcall.h
./inc/mockmethodcallbase.h
./inc/mockresultvalue.h
./inc/mockvalue.h
./inc/mockvaluebase.h
./inc/nicecallcomparer.h
./inc/runtimemock.h
./inc/stdafx.h
./inc/strictorderedcallcomparer.h
./inc/strictunorderedcallcomparer.h
./inc/targetver.h
./inc/threadsafeglobalmock.h
./inc/timediscretemicromock.h
./inc/timediscretemicromockcallmacros.h
)

string(REPLACE "./inc/" ";./testtools/micromock/inc/" FILES_TO_INSTALL ${micromock_h_files})
set(micromock_h_files_full_path "${FILES_TO_INSTALL}" CACHE INTERNAL "")
add_files_to_install("${FILES_TO_INSTALL}")

include_directories(${SHARED_UTIL_INC_FOLDER})

IF(WIN32)
    #windows needs this define
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)

    if (${use_cppunittest})
        #windows needs to be build 2 flavor of micromock, one for CTEST reporting, one for CPP_UNITTEST
        add_library(micromock_cpp_unittest ${micromock_cpp_files} ${micromock_h_files})
        target_compile_definitions(micromock_cpp_unittest PUBLIC CPP_UNITTEST)
        target_compile_options(micromock_cpp_unittest PUBLIC /EHsc)

        if(EXISTS "$ENV{VCInstallDir}/UnitTest/include")
            #flip VCInstallDir path
            file(TO_CMAKE_PATH $ENV{VCInstallDir} VCInstallDir)
            #pre-VS2017 build environment doesn't appear to include the path to unit
            #test headers, so add it here
            target_include_directories(micromock_cpp_unittest PUBLIC ${VCInstallDir}/UnitTest/include)
        endif()

        set_target_properties(micromock_cpp_unittest
           PROPERTIES
           FOLDER "test_tools")
    endif()

    add_library(micromock_ctest ${micromock_cpp_files} ${micromock_h_files})
    target_compile_definitions(micromock_ctest PUBLIC USE_CTEST)
    target_compile_options(micromock_ctest PUBLIC /EHsc)
else()
    add_library(micromock_ctest ${micromock_cpp_files} ${micromock_h_files})
    target_compile_definitions(micromock_ctest PUBLIC USE_CTEST)
ENDIF()

set_target_properties(micromock_ctest
               PROPERTIES
               FOLDER "test_tools")

#these are the include folders
#the following "set" statetement exports across the project a global variable called MICROMOCK_INC_FOLDER that expands to whatever needs to included when using micromock library
set(MICROMOCK_INC_FOLDER ${CMAKE_CURRENT_LIST_DIR}/inc CACHE INTERNAL "this is what needs to be included if using micromock lib" FORCE)

include_directories(${TESTRUNNERSWITCHER_INC_FOLDER} ${CTEST_INC_FOLDER} ${SAL_INC_FOLDER} ${MICROMOCK_INC_FOLDER} )
include_directories(${TESTRUNNERSWITCHER_INCLUDES} ${CTEST_INCLUDES} ${UMOCK_C_INCLUDES})

if (${run_unittests})
     add_subdirectory(unittests)
endif()

if(${use_installed_dependencies})
    if(WIN32)
        install (TARGETS micromock_cpp_unittest EXPORT aziotsharedutilTargets DESTINATION lib)
    endif()
    install (TARGETS micromock_ctest EXPORT aziotsharedutilTargets DESTINATION lib)
endif()

if(WIN32)
    add_executable(micromock_generator ./tools/micromockgenerator/micromockgenerator.cpp)

    target_compile_options(micromock_generator PUBLIC /EHsc)

    set_target_properties(micromock_generator
               PROPERTIES
               FOLDER "test_tools")
endif()

if (CMAKE_VERSION VERSION_LESS "3.1")
  if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    if (CXX_FLAG_CXX11)
      set_source_files_properties(${micromock_cpp_files} PROPERTIES COMPILE_FLAGS "-std=c++11")
    else()
      set_source_files_properties(${micromock_cpp_files} PROPERTIES COMPILE_FLAGS "-std=c++0x")
    endif()
  endif()
else()
  if (TARGET micromock_cpp_unittest)
    compileTargetAsC11(micromock_cpp_unittest)
  endif()
  if (TARGET micromock_ctest)
    compileTargetAsC11(micromock_ctest)
  endif()
endif()

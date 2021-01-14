# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT
#
#

if(MSVC)
  if(WARNINGS_AS_ERRORS)
    set(WARNINGS_AS_ERRORS_FLAG "/WX")
  endif()

  # https://stackoverflow.com/questions/58708772/cmake-project-in-visual-studio-gives-flag-override-warnings-command-line-warnin
  string(REGEX REPLACE "/W3" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

  #https://stackoverflow.com/questions/37527946/warning-unreferenced-inline-function-has-been-removed
  add_compile_options(/W4 ${WARNINGS_AS_ERRORS_FLAG} /wd5031 /wd4668 /wd4820 /wd4255 /wd4710)

  # Since we could be built as either DLL or a static library, when we are built as static library,
  # linker does generate a bunch of warnings related to the use of dllimport attribute,
  # basically, suggesting that its use is unneccessary. We know that.
  add_link_options(/ignore:4217 /ignore:4286)
  
  # NOTE: Static analysis will slow building time considerably and it is run during CI gates.
  # It is better to turn in on to debug errors reported by CI than have it ON all the time. 
  if (DEFINED ENV{AZURE_ENABLE_STATIC_ANALYSIS})
    add_compile_options(/analyze)
  endif()
elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
  if(WARNINGS_AS_ERRORS)
    set(WARNINGS_AS_ERRORS_FLAG "-Werror")
  endif()

  add_compile_options(-Xclang -Wall -Wextra -pedantic  ${WARNINGS_AS_ERRORS_FLAG} -Wdocumentation -Wdocumentation-unknown-command -Wcast-qual)
else()
  if(WARNINGS_AS_ERRORS)
    set(WARNINGS_AS_ERRORS_FLAG "-Werror")
  endif()

  add_compile_options(-Wall -Wextra -pedantic  ${WARNINGS_AS_ERRORS_FLAG})
endif()

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)

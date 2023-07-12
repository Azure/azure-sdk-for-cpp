// Copyright (c) Microsoft Corporation.
// SPDX-Licence-Identifier: MIT

#pragma once

/** clang diagnostic push and pop macros
 *
 * These macros are used to suppress warnings from clang and gcc compilers.
 *
 * Some versions of clang don't understand the doxygen commands @cond and @endcond used to suppress
 * documentation warnings.
 *
 * To work around this, add
 * BEGIN_UNKNOWN_DOCUMENTATION_DIAGNOSTIC_IGNORE/END_UNKNOWN_DOCUMENTATION_DIAGNOSTIC_IGNORE around
 * the lines which generate	an error.
 */
#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
#define BEGIN_UNKNOWN_DOCUMENTATION_DIAGNOSTIC_IGNORE _Pragma("GCC diagnostic push")
#define END_UNKNOWN_DOCUMENTATION_DIAGNOSTIC_IGNORE _Pragma("GCC diagnostic pop")
#elif defined(__clang__) // !__clang__
#define BEGIN_UNKNOWN_DOCUMENTATION_DIAGNOSTIC_IGNORE _Pragma("clang diagnostic push")
_Pragma("clang diagnostic ignored \"-Wdocumentation-unknown-command\"")
#define END_UNKNOWN_DOCUMENTATION_DIAGNOSTIC_IGNORE _Pragma("clang diagnostic pop")
#else
#define BEGIN_UNKNOWN_DOCUMENTATION_DIAGNOSTIC_IGNORE
#define END_UNKNOWN_DOCUMENTATION_DIAGNOSTIC_IGNORE
#endif // _MSC_VER

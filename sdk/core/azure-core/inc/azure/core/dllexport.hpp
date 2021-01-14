// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief This file defines a macro for DLL export.
 */

// We use CMAKE_WINDOWS_DLLEXPORT_ALL_SYMBOLS which does most of the job for us, but static data
// members do still need to be declared as __declspec(dllimport) for the client code. (See
// https://cmake.org/cmake/help/v3.13/prop_tgt/WINDOWS_DLLEXPORT_ALL_SYMBOLS.html)
// The way it works is this: each library has its own AZ_xxx_DLLEXPORT macro, which is used as
// prefix for public(*) static variables(**), this way: class Class { public:
//   AZ_xxx_DLLEXPORT static bool IsSomething;
// }
// And also each cmake file adds a corresponding build definition:
// add_compile_build_definitions(AZ_xxx_BEING_BUILT), so it IS defined at the moment the specific
// library (xxx) is being built, and is not defined at all other times. The AZ_xxx_DLLEXPORT macro
// makes the static variable to be prefixed with dllexport attribute at a time when the (xxx) is
// being built, and all other code (other libraries, customer code) see it as dllimport attribute,
// when they include the header and consume the static veriable from the (xxx) library.
// For that reason, each library should have its own AZ_xxx_DLLEXPORT macro: when we build (yyy)
// library which consumes (xxx) library, (xxx)'s symbols are dllimport, while, from (yyy)'s point of
// view, (yyy)'s symbols are dllexport. Do not reuse dllexport.hpp file from other libraries, do not
// reuse AZ_CORE_DLLEXPORT or AZ_CORE_BEING_BUILT names.
// --
// (*) - could be private, but if a public inline class member function uses it, it is effectively
// public and the export attribute should be used.
// (**) - mutable or immutable (const) static variables. But not constexprs. Constexprs don't need
// the export attribute.

#pragma once

#if defined(_MSC_VER)
#if defined(AZ_CORE_BEING_BUILT)
#define AZ_CORE_DLLEXPORT __declspec(dllexport)
#else
#define AZ_CORE_DLLEXPORT __declspec(dllimport)
#endif
#else
#define AZ_CORE_DLLEXPORT
#endif

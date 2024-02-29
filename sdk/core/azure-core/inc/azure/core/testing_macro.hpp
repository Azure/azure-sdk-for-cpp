// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief This file is used to define internal-only macros that are used to control the behavior of
 * the Azure SDK when running tests, to allow mocking within tests.
 * The macros in this file should NOT be used by anyone outside this repo.
 */

#pragma once

// When testing is enabled, we want to make sure that certain classes are not final, so that we can
// mock it.
#if defined(_azure_TESTING_BUILD)

/**
 * @brief If we are testing, we want to make sure that classes are not final, by default.
 */
#if !defined(_azure_NON_FINAL_FOR_TESTS)
#define _azure_NON_FINAL_FOR_TESTS
#endif

/**
 * @brief If we are testing, we want to make sure methods can be made virtual, for mocking.
 */
#if !defined(_azure_VIRTUAL_FOR_TESTS)
#define _azure_VIRTUAL_FOR_TESTS virtual
#endif

#else

/**
 * @brief If we are not testing, we want to make sure that classes are final, by default.
 */
#if !defined(_azure_NON_FINAL_FOR_TESTS)
#define _azure_NON_FINAL_FOR_TESTS final
#endif

/**
 * @brief If we are not testing, we don't need to make methods virtual for mocking.
 */
#if !defined(_azure_VIRTUAL_FOR_TESTS)
#define _azure_VIRTUAL_FOR_TESTS
#endif

#endif

//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

// Define `min` and `max` as function-like macros before including all public
// headers to ensure that uses of those identifiers are defended against
// expansion as function-like macros. Define `small` as an object-like macro to
// ensure that identifier isn't used at all. Windows.h is badly behaved and
// defines similar macros with these names and we want to ensure the SDK headers
// function even when a naive user includes Windows.h first.
//
#define small FAIL><TO][COMPILE)(VERY{{{LOUDLY!!!
#define max(x, y) small
#define min(x, y) small

#include <azure/storage/files/shares.hpp>

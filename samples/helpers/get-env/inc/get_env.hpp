// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

// Since `std::getenv()` may generate warnings on MSVC, and is not available on UWP, sample code
// gets cluttered with insignificant nuances. `GetEnv()` function hides all that.

#pragma once

#include <string>

std::string GetEnv(const char* name);

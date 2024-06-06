//     __ _____ _____ _____
//  __|  |   __|     |   | |  JSON for Modern C++ (supporting code)
// |  |  |__   |  |  | | | |  version 3.11.3
// |_____|_____|_____|_|___|  https://github.com/nlohmann/json
//
// SPDX-FileCopyrightText: 2013-2023 Niels Lohmann <https://nlohmann.me>
// SPDX-License-Identifier: MIT

#include "doctest_compatibility.h"
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX

#ifdef _WIN32
#include <windows.h>
#endif

#include <azure/core/internal/json/json.hpp>
using Azure::Core::Json::_internal::json;

TEST_CASE("include windows.h") { CHECK(true); }

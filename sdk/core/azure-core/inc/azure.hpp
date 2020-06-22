// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <internal/contract.hpp>

#define AZURE_UNREFERENCED_PARAMETER(x) ((void) (x));

namespace Azure { namespace Core {

bool LocaleInvariantCaseInsensitiveEqual(const std::string& lhs, const std::string& rhs) noexcept;

}} // namespace Azure::Core

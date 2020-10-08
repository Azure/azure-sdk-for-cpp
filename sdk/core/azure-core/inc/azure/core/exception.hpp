// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Define RequestFailedException. It is used by HTTP exceptions.
 */

#pragma once

#include <stdexcept>

namespace Azure { namespace Core {
  struct RequestFailedException : public std::runtime_error
  {
    explicit RequestFailedException(std::string const& msg) : std::runtime_error(msg) {}
  };
}} // namespace Azure::Core

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "logging/logging.hpp"

namespace Azure { namespace Core { namespace Logging { namespace Details {
  bool ShouldWrite(Azure::Core::Logging::LogClassification classification) noexcept;

  void Write(
      Azure::Core::Logging::LogClassification classification,
      std::string const& message) noexcept;
}}}} // namespace Azure::Core::Logging::Details

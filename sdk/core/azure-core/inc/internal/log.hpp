// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "logging/logging.hpp"

namespace Azure { namespace Core { namespace Logging { namespace Details {
  bool ShouldWrite(LogClassification const& classification);
  void Write(LogClassification const& classification, std::string const& message);
}}}} // namespace Azure::Core::Logging::Details

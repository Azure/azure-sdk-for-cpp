// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "logging/logging.hpp"

namespace Azure { namespace Core { namespace Logging { namespace Details {
  bool ShouldWrite(LogClassification classification);
  void Write(LogClassification classification, std::string const& message);
}}}} // namespace Azure::Core::Logging::Details

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once

#include <azure/core/base64.hpp>

class AttestationCollateral {
public:
  static std::vector<uint8_t> OpenEnclaveReport();
  static std::vector<uint8_t> SgxQuote();
  static std::vector<uint8_t> RuntimeData();

  static std::string GetMinimalPolicy();
};

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once

#include <azure/core/base64.hpp>

namespace Azure { namespace Security { namespace Attestation { namespace Samples {

  class AttestationCollateral {
  public:
    static std::vector<uint8_t> OpenEnclaveReport();
    static std::vector<uint8_t> SgxQuote();
    static std::vector<uint8_t> RuntimeData();

    static std::string GetMinimalPolicy();
  };
}}}} // namespace Azure::Security::Attestation::Samples

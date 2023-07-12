// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

#include <azure/core/base64.hpp>

namespace Azure { namespace Security { namespace Attestation { namespace Test {

  class AttestationCollateral {
  public:
    static std::vector<uint8_t> OpenEnclaveReport();
    static std::vector<uint8_t> SgxQuote();
    static std::vector<uint8_t> RunTimeData();

    static std::string GetMinimalPolicy();
  };
}}}} // namespace Azure::Security::Attestation::Test

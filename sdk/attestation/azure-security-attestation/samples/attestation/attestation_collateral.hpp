// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once

#include <azure/core/base64.hpp>

/** @brief Collateral used by Attestation APIs
 *
 * @details this hides the details of the actual base64url encoded values from sample
 * implementations.
 */
class AttestationCollateral {
public:
  /// An Open Enclave report which can be used to test.
  static std::vector<uint8_t> OpenEnclaveReport();

  /// An Sgx Enclave report which can be used to test.
  static std::vector<uint8_t> SgxQuote();

  /// RunTime data which is encapsulated in the quote/reports returned.
  static std::vector<uint8_t> RunTimeData();

  /// Minimal Attestation Policy.
  static std::string GetMinimalPolicy();
};

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once

#include <string>

namespace Azure { namespace Security { namespace Attestation { namespace Test {

  class CryptoTestCollateral {
  public:
    static std::string TestEcdsPrivateKey();
    static std::string TestEcdsPublicKey();
    static std::string TestRsaPrivateKey();
    static std::string TestRsaPublicKey();
  };
}}}} // namespace Azure::Security::Attestation::Test

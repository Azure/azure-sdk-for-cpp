// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
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

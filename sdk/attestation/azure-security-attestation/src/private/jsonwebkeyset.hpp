// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Provides a wrapper class for the Azure Core Pipeline for all Attestation services where
 * common functionality is set up.
 *
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Security { namespace Attestation{ namespace _detail {

  struct JsonWebKey final
  {
    std::string kid;
    std::string kty;
    std::string alg;
    std::vector<std::string> x5c;
    std::string x5t;
    std::string use;
    std::string x5t256;
    std::string x5u;
    std::vector<std::string> keyops;

    // RSA Public Keys (alg == 'RS256' | 'RS384' | 'RS512').
    std::string n;
    std::string e;
    // Private key
    std::string p;
    std::string q;
    std::string dp;
    std::string dq;
    std::string qi;
    std::string oth;
    // ECDS Public Keys (alg == 'ES256' | 'ES384' | 'ES512').
    std::string crv;
    std::string x;
    std::string y;
    // Private key
    std::string d; // Shared with RSA Public Key.
    
  };

  struct JsonWebKeySet final
  {
    std::vector<JsonWebKey> Keys;
  };

}}}} // namespace Azure::Security::Attestation::_detail

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Attestation client model support classes and functions.
 *
 * This file contains private classes used to support public model types.
 *
 */

#pragma once

#include "azure/attestation/attestation_client_models.hpp"
#include "azure/attestation/attestation_client_options.hpp"
#include "azure/core/nullable.hpp"
#include <memory>
#include <string>
#include <vector>

namespace Azure {
  namespace Security {
    namespace Attestation {
      namespace Models {
        namespace _detail {

  struct JsonWebKey final
  {
    Azure::Nullable<std::string> kid;
    Azure::Nullable<std::string> kty;
    Azure::Nullable<std::string> alg;
    Azure::Nullable<std::vector<std::string>> x5c;
    Azure::Nullable<std::string> x5t;
    Azure::Nullable<std::string> use;
    Azure::Nullable<std::string> x5t256;
    Azure::Nullable<std::string> x5u;
    Azure::Nullable<std::vector<std::string>> keyops;

    // RSA Public Keys (alg == 'RS256' | 'RS384' | 'RS512').
    Azure::Nullable<std::string> n;
    Azure::Nullable<std::string> e;
    // Private key
    Azure::Nullable<std::string> p;
    Azure::Nullable<std::string> q;
    Azure::Nullable<std::string> dp;
    Azure::Nullable<std::string> dq;
    Azure::Nullable<std::string> qi;
    Azure::Nullable<std::string> oth;
    // ECDSA Public Keys (alg == 'ES256' | 'ES384' | 'ES512').
    Azure::Nullable<std::string> crv;
    Azure::Nullable<std::string> x;
    Azure::Nullable<std::string> y;
    // Private key
    Azure::Nullable<std::string> d; // Shared with RSA Public Key.
  };

  struct JsonWebKeySet final
  {
    std::vector<JsonWebKey> Keys;
  };

  /**
   * @brief Internal implementation class implementing private methods for public model type
   * AttestationSigner.
   */
  class AttestationSignerInternal {
    AttestationSigner m_signer;

  public:
    AttestationSignerInternal(JsonWebKey const& jwk);

    static std::string PemFromX5c(std::string const& x5c);
    operator AttestationSigner&&() { return std::move(m_signer); }
    static std::string SerializeToJson(AttestationSigner const& signer);
  };

  struct AttestSgxEnclaveRequest
  {
    std::vector<uint8_t> Quote;
    Azure::Nullable<AttestationData> InitTimeData;
    Azure::Nullable<AttestationData> RunTimeData;
    Azure::Nullable<std::string> DraftPolicyForAttestation;
    Azure::Nullable<std::string> Nonce;
  };
  struct AttestOpenEnclaveRequest
  {
    std::vector<uint8_t> Report;
    Azure::Nullable<AttestationData> InitTimeData;
    Azure::Nullable<AttestationData> RunTimeData;
    Azure::Nullable<std::string> DraftPolicyForAttestation;
    Azure::Nullable<std::string> Nonce;
  };

}}}}} // namespace Azure::Security::Attestation::Models::_detail

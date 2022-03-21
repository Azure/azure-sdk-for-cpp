// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Attestation client model support classes and functions.
 *
 * This file contains private classes used to support public model types.
 *
 */
#include "attestation_client_models_private.hpp"
#include "crypto/inc/crypto.hpp"
#include <azure/core/internal/json/json.hpp>
#include <chrono>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
using namespace Azure::Security::Attestation::_detail;

namespace Azure { namespace Security { namespace Attestation { namespace _detail {

}}}} // namespace Azure::Security::Attestation::_detail

namespace Azure {
  namespace Security {
    namespace Attestation {
      namespace Models {
        namespace _detail {

  AttestationSignerInternal::AttestationSignerInternal(JsonWebKey const& jwk)
  {
    m_signer.KeyId = jwk.Kid;
    if (jwk.X5c)
    {
      m_signer.CertificateChain = std::vector<std::string>();
      for (const auto& X5c : *jwk.X5c)
      {
        m_signer.CertificateChain->push_back(Cryptography::PemFromBase64(X5c, "CERTIFICATE"));
      }
    }
  }

  std::string AttestationSignerInternal::SerializeToJson(AttestationSigner const& signer)
  {
    Azure::Core::Json::_internal::json rv;

    if (signer.KeyId)
    {
      rv["Kid"] = *signer.KeyId;
    }
    if (signer.CertificateChain)
    {
      rv["X5c"] = *signer.CertificateChain;
    }
    return rv.dump();
  }
}}}}} // namespace Azure::Security::Attestation::Models::_detail

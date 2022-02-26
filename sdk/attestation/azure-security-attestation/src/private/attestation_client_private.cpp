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

namespace Azure { namespace Security { namespace Attestation { namespace _detail {

}}}} // namespace Azure::Security::Attestation::_detail

namespace Azure {
  namespace Security {
    namespace Attestation {
      namespace Models {
        namespace _detail {

  AttestationSignerInternal::AttestationSignerInternal(JsonWebKey const& jwk)
  {
    m_signer.KeyId = jwk.kid;
    if (jwk.x5c)
    {
      m_signer.CertificateChain = std::vector<std::string>();
      for (const auto& x5c : jwk.x5c.Value())
      {
        m_signer.CertificateChain.Value().push_back(
            Azure::Security::Attestation::_detail::Cryptography::PemFromBase64(x5c, "CERTIFICATE"));
      }
    }
  }

  std::string AttestationSignerInternal::SerializeToJson(AttestationSigner const& signer)
  {
    Azure::Core::Json::_internal::json rv;

    if (signer.KeyId)
    {
      rv["kid"] = signer.KeyId.Value();
    }
    if (signer.CertificateChain)
    {
      rv["x5c"] = signer.CertificateChain.Value();
    }
    return rv.dump();
  }
}}}}} // namespace Azure::Security::Attestation::Models::_detail

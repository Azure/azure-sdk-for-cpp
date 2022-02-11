// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Attestation client model support classes and functions.
 *
 * This file contains private classes used to support public model types.
 *
 */
#include "attestation_client_models_private.hpp"
#include "attestation_deserializers_private.hpp"
#include "azure/attestation/attestation_client.hpp"
#include "azure/attestation/attestation_client_models.hpp"
#include "azure/attestation/attestation_client_options.hpp"
#include "crypto/inc/crypto.hpp"
#include "jsonhelpers_private.hpp"
#include <azure/core/base64.hpp>
#include <azure/core/context.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_optional.hpp>
#include <azure/core/internal/json/json_serializable.hpp>
#include <azure/core/response.hpp>
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
    if (jwk.x5c.HasValue())
    {
      m_signer.CertificateChain = std::vector<std::string>();
      for (const auto& x5c : jwk.x5c.Value())
      {
        m_signer.CertificateChain.Value().push_back(PemFromX5c(x5c));
      }
    }
  }

  std::string AttestationSignerInternal::PemFromX5c(std::string const& x5c)
  {
    std::string rv;
    rv += "-----BEGIN CERTIFICATE-----\r\n";
    std::string encodedKey(x5c);

    // Insert crlf characters every 80 characters into the base64 encoded key to make it
    // prettier.
    size_t insertPos = 80;
    while (insertPos < encodedKey.length())
    {
      encodedKey.insert(insertPos, "\r\n");
      insertPos += 82; /* 80 characters plus the \r\n we just inserted */
    }

    rv += encodedKey;
    rv += "\r\n-----END CERTIFICATE-----\r\n";
    return rv;
  }
  std::string AttestationSignerInternal::SerializeToJson(AttestationSigner const& signer)
  {
    Azure::Core::Json::_internal::json rv;

    if (signer.KeyId.HasValue())
    {
      rv["kid"] = signer.KeyId.Value();
    }
    if (signer.CertificateChain.HasValue())
    {
      rv["x5c"] = signer.CertificateChain.Value();
    }
    return rv.dump();
  }
}}}}} // namespace Azure::Security::Attestation::Models::_detail
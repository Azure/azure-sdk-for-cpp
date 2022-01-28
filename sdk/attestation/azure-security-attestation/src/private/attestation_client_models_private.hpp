// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Attestation client model support classes and functions.
 *
 * This file contains private classes used to support public model types.
 *
 */

#pragma once

#include <azure/core/context.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_serializable.hpp>
#include <azure/core/response.hpp>

#include "crypto/inc/crypto.hpp"
#include "jsonwebkeyset.hpp"
#include <azure/attestation/attestation_client_models.hpp>
#include <azure/attestation/attestation_client_options.hpp>
#include <azure/core/base64.hpp>
#include <memory>
#include <string>
#include <vector>

namespace Azure {
  namespace Security {
    namespace Attestation {
      namespace Models {
        namespace _detail {

  using namespace Azure::Core::Json::_internal;
  using namespace Azure::Core::Http;
  using namespace Azure::Security::Attestation::_detail;
  /**
   * @brief Internal implementation class implementing private methods for public model type
   * AttestationSigner.
   */
  class AttestationSignerInternal {
    AttestationSigner m_signer;

  public:
    AttestationSignerInternal(Azure::Security::Attestation::_detail::JsonWebKey const& jwk)
    {
      m_signer.KeyId = jwk.kid;
      for (const auto& x5c : jwk.x5c)
      {
        m_signer.CertificateChain.push_back(PemFromX5c(x5c));
      }
    }

    std::string PemFromX5c(std::string const& x5c)
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
    operator AttestationSigner&&() { return std::move(m_signer); }
  };

  template <class T, typename TDeserializer> class AttestationTokenInternal {
  private:
    AttestationToken<T> m_token;

    void ParseRawToken()
    {
      std::string token(m_token.RawToken);
      size_t headerIndex = token.find('.');
      if (headerIndex == std::string::npos)
      {
        throw Azure::Core::RequestFailedException("Could not find required . in token.");
      }
      std::string header(token);
      header.erase(headerIndex);
      token.erase(0, headerIndex + 1);
      size_t bodyIndex = token.find('.');
      if (headerIndex == std::string::npos)
      {
        throw Azure::Core::RequestFailedException("Could not find required second . in token.");
      }
      m_token.RawHeader = header;

      std::string body(token);
      body.erase(bodyIndex);
      token.erase(0, bodyIndex + 1);
      std::string signature(token);
      m_token.RawBody = body;

      auto jsonHeader(json::parse(Azure::Core::_internal::Base64Url::Base64UrlDecode(header)));
      auto jsonBody(json::parse(Azure::Core::_internal::Base64Url::Base64UrlDecode(body)));
      m_token.Body = TDeserializer::Deserialize(jsonBody);
    }

  public:
    AttestationTokenInternal(std::string& jwt)
    {
      m_token.RawToken = std::string(jwt);
      ParseRawToken();
    }

    void ValidateToken(std::vector<AttestationSigner> const&) {}
    operator AttestationToken<T>&&() { return std::move(m_token); }
  };

  /// <summary>
  /// Private Model types used for interoperability with the attestation service.
  /// </summary>

  // Implementation Model types.
  struct AttestationData
  {
    std::vector<uint8_t> Data;
    Azure::Security::Attestation::AttestationDataType DataType;
  };

  struct AttestSgxEnclaveRequest
  {
    std::vector<uint8_t> Quote;
    AttestationData InitTimeData;
    AttestationData RunTimeData;
    std::string DraftPolicyForAttestation;
  };
  struct AttestOpenEnclaveRequest
  {
    std::vector<uint8_t> Report;
    AttestationData InitTimeData;
    AttestationData RunTimeData;
    std::string DraftPolicyForAttestation;
  };

}}}}} // namespace Azure::Security::Attestation::Models::_detail

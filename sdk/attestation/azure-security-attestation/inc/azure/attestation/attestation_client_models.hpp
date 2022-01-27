// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the Azure Attestatoin API types.
 *
 */

#pragma once

#include "azure/attestation/dll_import_export.hpp"

#include <azure/core/context.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/nullable.hpp>
#include <azure/core/paged_response.hpp>
#include <azure/core/response.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Azure { namespace Security { namespace Attestation {
  class AttestationClient;
  /**
   * @brief Contains identity and other basic properties of a Certificate.
   *
   */
  struct AttestationOpenIdMetadata final
  {
    /**
     */
    ::std::string Issuer;
    ::std::string JsonWebKeySetUrl;
    ::std::vector<::std::string> SupportedResponseTypes;
    ::std::vector<::std::string> SupportedTokenSigningAlgorithms;
    ::std::vector<::std::string> SupportedClaims;

    AttestationOpenIdMetadata() {}
    ~AttestationOpenIdMetadata() {}
    AttestationOpenIdMetadata(AttestationOpenIdMetadata&& that) noexcept
        : Issuer(::std::move(that.Issuer)), JsonWebKeySetUrl(::std::move(that.JsonWebKeySetUrl)),
          SupportedResponseTypes(::std::move(that.SupportedResponseTypes)),
          SupportedTokenSigningAlgorithms(::std::move(that.SupportedTokenSigningAlgorithms)),
          SupportedClaims(::std::move(that.SupportedClaims))
    {
    }
  };

  struct AttestationSigner final
  {
    std::string KeyId;
    std::vector<std::string> CertificateChain;
  };

  struct AttestationResult final
  {
    std::string Issuer;
    std::string UniqueIdentifier;
    std::string Nonce;
    std::string Version;
    std::string RuntimeClaims;
    std::string InitTimeClaims;
    std::string PolicyClaims;
    std::vector<uint8_t> EnclaveHeldData;
    std::string VerifierType;
    AttestationSigner PolicySigner;
    std::vector<uint8_t> PolicyHash;
    Azure::Nullable<bool> IsDebuggable;
    Azure::Nullable<int> ProductId{0};
    std::vector<uint8_t> MrEnclave;
    std::vector<uint8_t> MrSigner;
    Azure::Nullable<int> Svn;
    std::string SgxCollateral;
  };

  /// An AttestationTokenHeader represents common properties in an the RFC 7515 JSON Web Token.

  struct AttestationTokenHeader
  {
    std::string Algorithm;
    std::string KeyId;

    Azure::DateTime ExpiresOn;
    Azure::DateTime NotBefore;
    Azure::DateTime IssuedOn;
    std::string ContentType;
    std::string KeyURL;
    Azure::Nullable<bool> Critical;
    std::string X509Url;
    std::string Type;
    std::string CertificateThumbprint;
    std::string CertificateSha256Thumbprint;
    std::string Issuer;
    std::vector<std::string> X509CertificateChain;
  };

  /// An AttestationToken represents an RFC 7515 JSON Web Token returned from the attestation
  /// service.
  /// <typeparam name="T"></typeparam>
  template <typename T> struct AttestationToken
  {
    std::string RawToken;
    std::string RawHeader;
    std::string RawBody;
    T Body;
    AttestationTokenHeader Header;
  };

  /// An AttestationSigningKey represents a pair of signing keys and certificates.
  struct AttestationSigningKey
  {
    std::string SigningPrivateKey; /// A PEM encoded RSA or ECDSA private key which will be used to
                                   /// sign an attestation token.
    std::string SigningCertificate; /// A PEM encoded X.509 certificate which will be sent to the
                                    /// attestation service to validate an attestation token. The
                                    /// public key embedded in the certificate MUST be the public
                                    /// key of the SigningPrivateKey.
  };
}}} // namespace Azure::Security::Attestation
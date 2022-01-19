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
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>

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
          SupportedClaims(::std::move(that.SupportedClaims)),
          SupportedTokenSigningAlgorithms(::std::move(that.SupportedTokenSigningAlgorithms)),
          SupportedResponseTypes(::std::move(that.SupportedResponseTypes))
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

  template <typename T>
  struct AttestationToken
  {
    std::string RawToken;
    std::string RawHeader;
    std::string RawBody;
    T Body;
  };
}}} // namespace Azure::Security::Attestation
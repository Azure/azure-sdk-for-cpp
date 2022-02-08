// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Provides a wrapper class for the Azure Core Pipeline for all Attestation services where
 * common functionality is set up.
 *
 */

#pragma once

#include "jsonhelpers.hpp"
#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Security { namespace Attestation { namespace _detail {

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

  struct JsonWebKeySerializer final
  {
    static JsonWebKey Deserialize(const Azure::Core::Json::_internal::json& jwk)
    {
      JsonWebKey returnValue;
      if (jwk.contains("kty"))
      {
        returnValue.kty
            = Azure::Security::Attestation::_detail::JsonHelpers::ParseStringField(jwk, "kty")
                  .Value();
      }
      else
      {
        throw std::runtime_error("Could not find required field 'kty' in JSON Web Key");
      }
      returnValue.alg
          = Azure::Security::Attestation::_detail::JsonHelpers::ParseStringField(jwk, "alg");
      returnValue.kid
          = Azure::Security::Attestation::_detail::JsonHelpers::ParseStringField(jwk, "kid");
      returnValue.use
          = Azure::Security::Attestation::_detail::JsonHelpers::ParseStringField(jwk, "use");
      returnValue.keyops
          = Azure::Security::Attestation::_detail::JsonHelpers::ParseStringArrayField(
              jwk, "key_ops");
      returnValue.x5t
          = Azure::Security::Attestation::_detail::JsonHelpers::ParseStringField(jwk, "x5t");
      returnValue.x5t256
          = Azure::Security::Attestation::_detail::JsonHelpers::ParseStringField(jwk, "x5t#S256");
      returnValue.x5u
          = Azure::Security::Attestation::_detail::JsonHelpers::ParseStringField(jwk, "x5u");
      returnValue.x5c
          = Azure::Security::Attestation::_detail::JsonHelpers::ParseStringArrayField(jwk, "x5c");

      // ECDSA key values.
      returnValue.crv
          = Azure::Security::Attestation::_detail::JsonHelpers::ParseStringField(jwk, "crv");
      returnValue.x
          = Azure::Security::Attestation::_detail::JsonHelpers::ParseStringField(jwk, "x");
      returnValue.y
          = Azure::Security::Attestation::_detail::JsonHelpers::ParseStringField(jwk, "y");
      returnValue.d
          = Azure::Security::Attestation::_detail::JsonHelpers::ParseStringField(jwk, "d");

      // RSA key values.
      returnValue.n
          = Azure::Security::Attestation::_detail::JsonHelpers::ParseStringField(jwk, "n");
      returnValue.e
          = Azure::Security::Attestation::_detail::JsonHelpers::ParseStringField(jwk, "e");
      returnValue.q
          = Azure::Security::Attestation::_detail::JsonHelpers::ParseStringField(jwk, "p");
      returnValue.dp
          = Azure::Security::Attestation::_detail::JsonHelpers::ParseStringField(jwk, "dp");
      returnValue.dq
          = Azure::Security::Attestation::_detail::JsonHelpers::ParseStringField(jwk, "dq");
      returnValue.qi
          = Azure::Security::Attestation::_detail::JsonHelpers::ParseStringField(jwk, "qi");

      return returnValue;
    }
  };

  // cspell: words jwks
  struct JsonWebKeySetSerializer final
  {
    static JsonWebKeySet Deserialize(std::unique_ptr<Azure::Core::Http::RawResponse>& response)
    {
      auto parsedBody = Azure::Core::Json::_internal::json::parse(response->GetBody());
      return Deserialize(parsedBody);
    }
    static JsonWebKeySet Deserialize(Azure::Core::Json::_internal::json const& parsedBody)
    {
      JsonWebKeySet returnValue;
      if (!parsedBody.contains("keys"))
      {
        throw std::runtime_error("Field 'keys' not found in JWKS.");
      }
      if (!parsedBody["keys"].is_array())
      {
        throw std::runtime_error("Field 'keys' is not an array.");
      }
      for (const auto& key : parsedBody["keys"])
      {
        returnValue.Keys.push_back(JsonWebKeySerializer::Deserialize(key));
      }
      return returnValue;
    };
  };

}}}} // namespace Azure::Security::Attestation::_detail

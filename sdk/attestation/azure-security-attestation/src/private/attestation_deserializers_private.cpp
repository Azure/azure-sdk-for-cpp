// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Azure::Core::Json::_internal::json Deserialization support functions.
 *
 * This file contains a set of support functions to aid in serializing and deserializing JSON
 * objects. It contains Serializer classes, one for each model type which each support a
 * static Serialize and Deserialize function which serialize and deserialize the specified model
 * types from and to JSON objects.
 *
 */

#include "attestation_deserializers_private.hpp"
#include "attestation_client_models_private.hpp"
#include "attestation_client_private.hpp"
#include "azure/attestation/attestation_client_models.hpp"
#include "jsonhelpers_private.hpp"
#include <azure/core/base64.hpp>
#include <azure/core/context.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_optional.hpp>
#include <azure/core/internal/json/json_serializable.hpp>
#include <azure/core/nullable.hpp>
#include <azure/core/response.hpp>
#include <memory>
#include <string>
#include <vector>

// cspell: words jwks MrSigner MrEnclave
namespace Azure { namespace Security { namespace Attestation { namespace _detail {
  using namespace Azure::Security::Attestation::Models;
  using namespace Azure::Security::Attestation::Models::_detail;
  using namespace Azure::Security::Attestation;

  /***************************************
   * A quick note on the naming convention for the Serialize/Deserialize classes.
   *
   * Serialization classes are named "XxxxSerializer". They contain one or two static methods
   * named "Serialize" and "Deserialize".
   *
   * The Serialize method takes an instance of an "Xxxx" object and returns a std::string which
   * represents the Xxxx object serialized into JSON.
   *
   * The Deserialize method takes an instance of a json object and returns an instance of the Xxxx
   * type.
   *
   */

  AttestationOpenIdMetadata OpenIdMetadataSerializer::Deserialize(
      std::unique_ptr<Azure::Core::Http::RawResponse>& response)
  {
    Models::AttestationOpenIdMetadata returnValue;
    auto parsedBody = Azure::Core::Json::_internal::json::parse(response->GetBody());
    returnValue.Issuer = JsonHelpers::ParseStringField(parsedBody, "issuer").Value();
    returnValue.JsonWebKeySetUrl = JsonHelpers::ParseStringField(parsedBody, "jwks_uri").Value();
    returnValue.SupportedClaims
        = JsonHelpers::ParseStringArrayField(parsedBody, "claims_supported").Value();
    returnValue.SupportedTokenSigningAlgorithms
        = JsonHelpers::ParseStringArrayField(parsedBody, "id_token_signing_alg_values_supported")
              .Value();
    returnValue.SupportedResponseTypes
        = JsonHelpers::ParseStringArrayField(parsedBody, "response_types_supported").Value();
    return returnValue;
  }

  std::string AttestSgxEnclaveRequestSerializer::Serialize(AttestSgxEnclaveRequest const& request)
  {
    Azure::Core::Json::_internal::json serializedRequest;
    serializedRequest["quote"] = Azure::Core::_internal::Base64Url::Base64UrlEncode(request.Quote);
    if (request.RunTimeData.HasValue())
    {
      serializedRequest["runtimeData"]
          = {{"data",
              Azure::Core::_internal::Base64Url::Base64UrlEncode(request.RunTimeData.Value().Data)},
             {"dataType", request.RunTimeData.Value().DataType.ToString()}};
    }

    if (request.InitTimeData.HasValue())
    {
      serializedRequest["inittimeData"] = {
          {"data",
           Azure::Core::_internal::Base64Url::Base64UrlEncode(request.InitTimeData.Value().Data)},
          {"dataType", request.InitTimeData.Value().DataType.ToString()}};
    }

    JsonHelpers::SetField(
        serializedRequest, request.DraftPolicyForAttestation, "draftPolicyForAttestation");
    JsonHelpers::SetField(serializedRequest, request.Nonce, "nonce");
    return serializedRequest.dump();
  }

  std::string AttestOpenEnclaveRequestSerializer::Serialize(AttestOpenEnclaveRequest const& request)
  {
    Azure::Core::Json::_internal::json serializedRequest;
    serializedRequest["report"]
        = Azure::Core::_internal::Base64Url::Base64UrlEncode(request.Report);

    if (request.RunTimeData.HasValue())
    {
      serializedRequest["runtimeData"]
          = {{"data",
              Azure::Core::_internal::Base64Url::Base64UrlEncode(request.RunTimeData.Value().Data)},
             {"dataType", request.RunTimeData.Value().DataType.ToString()}};
    }

    if (request.InitTimeData.HasValue())
    {
      serializedRequest["inittimeData"] = {
          {"data",
           Azure::Core::_internal::Base64Url::Base64UrlEncode(request.InitTimeData.Value().Data)},
          {"dataType", request.InitTimeData.Value().DataType.ToString()}};
    }

    JsonHelpers::SetField(
        serializedRequest, request.DraftPolicyForAttestation, "draftPolicyForAttestation");
    JsonHelpers::SetField(serializedRequest, request.Nonce, "nonce");
    return serializedRequest.dump();
  }

  std::string AttestationServiceTokenResponseSerializer::Deserialize(
      Azure::Core::Json::_internal::json const& parsedBody)
  {
    if (!parsedBody.contains("token"))
    {
      throw std::runtime_error("Field 'token' not found in Attestation Service Response");
    }
    if (!parsedBody["token"].is_string())
    {
      throw std::runtime_error("Field 'token' is not a string");
    }
    return parsedBody["token"].get<std::string>();
  }

  std::string AttestationServiceTokenResponseSerializer::Deserialize(
      std::unique_ptr<Azure::Core::Http::RawResponse>& response)
  {
    auto parsedBody = Azure::Core::Json::_internal::json::parse(response->GetBody());
    return Deserialize(parsedBody);
  }

  Models::AttestationResult AttestationResultSerializer::Deserialize(
      Azure::Core::Json::_internal::json const& parsedJson)
  {
    Models::AttestationResult result;

    result.Nonce = JsonHelpers::ParseStringField(parsedJson, "nonce");
    result.Version = JsonHelpers::ParseStringField(parsedJson, "x-ms-ver").Value();
    result.RuntimeClaims = JsonHelpers::ParseStringJsonField(parsedJson, "x-ms-runtime");
    result.InitTimeClaims = JsonHelpers::ParseStringJsonField(parsedJson, "x-ms-inittime");
    result.PolicyClaims = JsonHelpers::ParseStringJsonField(parsedJson, "x-ms-policy");
    result.VerifierType
        = JsonHelpers::ParseStringField(parsedJson, "x-ms-attestation-type").Value();
    if (parsedJson.contains("x-ms-policy-signer"))
    {
      result.PolicySigner = AttestationSignerInternal(
          JsonWebKeySerializer::Deserialize(parsedJson["x-ms-policy-signer"]));
    }
    result.PolicyHash = JsonHelpers::ParseBase64UrlField(parsedJson, "x-ms-policy-hash");
    result.IsDebuggable = JsonHelpers::ParseBooleanField(parsedJson, "x-ms-sgx-is-debuggable");
    result.ProductId = JsonHelpers::ParseIntNumberField(parsedJson, "x-ms-sgx-product-id");
    result.MrEnclave = JsonHelpers::ParseBase64UrlField(parsedJson, "x-ms-sgx-mrenclave");
    result.MrSigner = JsonHelpers::ParseBase64UrlField(parsedJson, "x-ms-sgx-mrsigner");
    result.EnclaveHeldData = JsonHelpers::ParseBase64UrlField(parsedJson, "x-ms-sgx-ehd");
    result.SgxCollateral = JsonHelpers::ParseStringJsonField(parsedJson, "x-ms-sgx-collateral");
    return result;
  }

  AttestationTokenHeader AttestationTokenHeaderSerializer::Deserialize(
      Azure::Core::Json::_internal::json const& jsonHeader)
  {
    AttestationTokenHeader returnValue;

    returnValue.Algorithm = JsonHelpers::ParseStringField(jsonHeader, "alg");
    returnValue.KeyURL = JsonHelpers::ParseStringField(jsonHeader, "jku");

    if (jsonHeader.contains("jwk"))
    {
      auto jwk = JsonWebKeySerializer::Deserialize(jsonHeader["jwk"]);
      returnValue.Key = AttestationSignerInternal(jwk);
    }
    returnValue.KeyId = JsonHelpers::ParseStringField(jsonHeader, "kid");
    returnValue.X509Url = JsonHelpers::ParseStringField(jsonHeader, "x5u");
    returnValue.X509CertificateChain = JsonHelpers::ParseStringArrayField(jsonHeader, "x5c");
    returnValue.CertificateSha256Thumbprint = JsonHelpers::ParseStringField(jsonHeader, "x5t#S256");
    returnValue.CertificateThumbprint = JsonHelpers::ParseStringField(jsonHeader, "x5t");
    returnValue.Type = JsonHelpers::ParseStringField(jsonHeader, "typ");
    returnValue.ContentType = JsonHelpers::ParseStringField(jsonHeader, "cty");
    returnValue.Critical = JsonHelpers::ParseStringArrayField(jsonHeader, "crit");
    return returnValue;
  }
  std::string AttestationTokenHeaderSerializer::Serialize(AttestationTokenHeader const& tokenHeader)
  {
    Azure::Core::Json::_internal::json serializedHeader;
    JsonHelpers::SetField(serializedHeader, tokenHeader.Algorithm, "alg");
    JsonHelpers::SetField(serializedHeader, tokenHeader.KeyURL, "jku");
    if (tokenHeader.Key.HasValue())
    {
      auto jsonSigner(AttestationSignerInternal::SerializeToJson(tokenHeader.Key.Value()));
      JsonHelpers::SetField(serializedHeader, jsonSigner, "jwk");
    }
    JsonHelpers::SetField(serializedHeader, tokenHeader.ContentType, "cty");
    JsonHelpers::SetField(serializedHeader, tokenHeader.Critical, "crit");
    JsonHelpers::SetField(serializedHeader, tokenHeader.KeyId, "kid");
    JsonHelpers::SetField(serializedHeader, tokenHeader.Type, "typ");

    JsonHelpers::SetField(serializedHeader, tokenHeader.X509CertificateChain, "x5c");
    JsonHelpers::SetField(serializedHeader, tokenHeader.X509Url, "x5u");
    JsonHelpers::SetField(serializedHeader, tokenHeader.CertificateSha256Thumbprint, "x5t#S256");
    JsonHelpers::SetField(serializedHeader, tokenHeader.CertificateThumbprint, "x5t");

    return serializedHeader.dump();
  }

  JsonWebKey JsonWebKeySerializer::Deserialize(const Azure::Core::Json::_internal::json& jwk)
  {
    JsonWebKey returnValue;
    if (jwk.contains("kty"))
    {
      returnValue.kty = JsonHelpers::ParseStringField(jwk, "kty").Value();
    }
    else
    {
      throw std::runtime_error("Could not find required field 'kty' in JSON Web Key");
    }
    returnValue.alg = JsonHelpers::ParseStringField(jwk, "alg");
    returnValue.kid = JsonHelpers::ParseStringField(jwk, "kid");
    returnValue.use = JsonHelpers::ParseStringField(jwk, "use");
    returnValue.keyops = JsonHelpers::ParseStringArrayField(jwk, "key_ops");
    returnValue.x5t = JsonHelpers::ParseStringField(jwk, "x5t");
    returnValue.x5t256 = JsonHelpers::ParseStringField(jwk, "x5t#S256");
    returnValue.x5u = JsonHelpers::ParseStringField(jwk, "x5u");
    returnValue.x5c = JsonHelpers::ParseStringArrayField(jwk, "x5c");

    // ECDSA key values.
    returnValue.crv = JsonHelpers::ParseStringField(jwk, "crv");
    returnValue.x = JsonHelpers::ParseStringField(jwk, "x");
    returnValue.y = JsonHelpers::ParseStringField(jwk, "y");
    returnValue.d = JsonHelpers::ParseStringField(jwk, "d");

    // RSA key values.
    returnValue.n = JsonHelpers::ParseStringField(jwk, "n");
    returnValue.e = JsonHelpers::ParseStringField(jwk, "e");
    returnValue.q = JsonHelpers::ParseStringField(jwk, "p");
    returnValue.dp = JsonHelpers::ParseStringField(jwk, "dp");
    returnValue.dq = JsonHelpers::ParseStringField(jwk, "dq");
    returnValue.qi = JsonHelpers::ParseStringField(jwk, "qi");

    return returnValue;
  }

  // cspell: words jwks
  JsonWebKeySet JsonWebKeySetSerializer::Deserialize(
      std::unique_ptr<Azure::Core::Http::RawResponse>& response)
  {
    auto parsedBody = Azure::Core::Json::_internal::json::parse(response->GetBody());
    return Deserialize(parsedBody);
  }
  JsonWebKeySet JsonWebKeySetSerializer::Deserialize(
      Azure::Core::Json::_internal::json const& parsedBody)
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
  }
}}}} // namespace Azure::Security::Attestation::_detail

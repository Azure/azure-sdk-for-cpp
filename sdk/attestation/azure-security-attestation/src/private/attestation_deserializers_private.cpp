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
  using namespace Azure::Core::Json::_internal;
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
    auto const parsedBody = Azure::Core::Json::_internal::json::parse(response->GetBody());

    JsonOptional::SetIfExists(returnValue.Issuer, parsedBody, "issuer");
    JsonOptional::SetIfExists(returnValue.JsonWebKeySetUrl, parsedBody, "jwks_uri");
    JsonOptional::SetIfExists(returnValue.SupportedClaims, parsedBody, "claims_supported");
    JsonOptional::SetIfExists(
        returnValue.SupportedTokenSigningAlgorithms,
        parsedBody,
        "id_token_signing_alg_values_supported");
    JsonOptional::SetIfExists(
        returnValue.SupportedResponseTypes, parsedBody, "response_types_supported");
    return returnValue;
  }

  std::string AttestSgxEnclaveRequestSerializer::Serialize(AttestSgxEnclaveRequest const& request)
  {
    Azure::Core::Json::_internal::json serializedRequest;
    serializedRequest["quote"] = Azure::Core::_internal::Base64Url::Base64UrlEncode(request.Quote);

    JsonOptional::SetFromNullable<AttestationData, std::map<std::string, std::string>>(
        request.RunTimeData,
        serializedRequest,
        "runtimeData",
        JsonHelpers::DecorateAttestationData);

    JsonOptional::SetFromNullable<AttestationData, std::map<std::string, std::string>>(
        request.InitTimeData,
        serializedRequest,
        "inittimeData",
        JsonHelpers::DecorateAttestationData);

    JsonOptional::SetFromNullable(request.Nonce, serializedRequest, "nonce");
    JsonOptional::SetFromNullable(
        request.DraftPolicyForAttestation, serializedRequest, "draftPolicyForAttestation");
    return serializedRequest.dump();
  }

  std::string AttestOpenEnclaveRequestSerializer::Serialize(AttestOpenEnclaveRequest const& request)
  {
    Azure::Core::Json::_internal::json serializedRequest;
    serializedRequest["report"]
        = Azure::Core::_internal::Base64Url::Base64UrlEncode(request.Report);

    JsonOptional::SetFromNullable<AttestationData, std::map<std::string, std::string>>(
        request.RunTimeData,
        serializedRequest,
        "runtimeData",
        JsonHelpers::DecorateAttestationData);

    JsonOptional::SetFromNullable<AttestationData, std::map<std::string, std::string>>(
        request.InitTimeData,
        serializedRequest,
        "inittimeData",
        JsonHelpers::DecorateAttestationData);

    JsonOptional::SetFromNullable(request.Nonce, serializedRequest, "nonce");
    JsonOptional::SetFromNullable(
        request.DraftPolicyForAttestation, serializedRequest, "draftPolicyForAttestation");
    return serializedRequest.dump();
  }

  std::string AttestationServiceTokenResponseSerializer::Deserialize(
      Azure::Core::Json::_internal::json const& parsedBody)
  {
    Azure::Nullable<std::string> token;
    JsonOptional::SetIfExists(token, parsedBody, "token");
    if (token)
    {
      return *token;
    }

    throw std::runtime_error("Field 'token' not found in Attestation Service Response");
  }

  std::string AttestationServiceTokenResponseSerializer::Deserialize(
      std::unique_ptr<Azure::Core::Http::RawResponse> const& response)
  {
    auto parsedBody = Azure::Core::Json::_internal::json::parse(response->GetBody());
    return Deserialize(parsedBody);
  }

  Models::AttestationResult AttestationResultSerializer::Deserialize(
      Azure::Core::Json::_internal::json const& parsedJson)
  {
    Models::AttestationResult result;

    JsonOptional::SetIfExists(result.Nonce, parsedJson, "nonce");
    JsonOptional::SetIfExists(result.Version, parsedJson, "x-ms-ver");
    JsonHelpers::SetIfExistsJson(result.RuntimeClaims, parsedJson, "x-ms-runtime");
    JsonHelpers::SetIfExistsJson(result.InitTimeClaims, parsedJson, "x-ms-inittime");
    JsonHelpers::SetIfExistsJson(result.PolicyClaims, parsedJson, "x-ms-policy");
    JsonOptional::SetIfExists(result.VerifierType, parsedJson, "x-ms-attestation-type");
    if (parsedJson.contains("x-ms-policy-signer"))
    {
      result.PolicySigner = AttestationSignerInternal(
          JsonWebKeySerializer::Deserialize(parsedJson["x-ms-policy-signer"]));
    }

    JsonOptional::SetIfExists<std::string, std::vector<uint8_t>>(
        result.PolicyHash,
        parsedJson,
        "x-ms-policy-hash",
        Azure::Core::_internal::Base64Url::Base64UrlDecode);

    JsonOptional::SetIfExists(result.SgxIsDebuggable, parsedJson, "x-ms-sgx-is-debuggable");
    JsonOptional::SetIfExists(result.SgxProductId, parsedJson, "x-ms-sgx-product-id");
    JsonOptional::SetIfExists(result.SgxSvn, parsedJson, "x-ms-sgx-svn");
    JsonOptional::SetIfExists<std::string, std::vector<uint8_t>>(
        result.SgxMrEnclave, parsedJson, "x-ms-sgx-mrenclave", JsonHelpers::HexStringToBinary);
    JsonOptional::SetIfExists<std::string, std::vector<uint8_t>>(
        result.SgxMrSigner, parsedJson, "x-ms-sgx-mrsigner", JsonHelpers::HexStringToBinary);
    JsonOptional::SetIfExists<std::string, std::vector<uint8_t>>(
        result.EnclaveHeldData,
        parsedJson,
        "x-ms-sgx-ehd",
        Azure::Core::_internal::Base64Url::Base64UrlDecode);
    JsonHelpers::SetIfExistsJson(result.SgxCollateral, parsedJson, "x-ms-sgx-collateral");
    return result;
  }

  AttestationTokenHeader AttestationTokenHeaderSerializer::Deserialize(
      Azure::Core::Json::_internal::json const& jsonHeader)
  {
    AttestationTokenHeader returnValue;

    JsonOptional::SetIfExists(returnValue.Algorithm, jsonHeader, "alg");
    JsonOptional::SetIfExists(returnValue.KeyURL, jsonHeader, "jku");

    if (jsonHeader.contains("jwk"))
    {
      auto jwk = JsonWebKeySerializer::Deserialize(jsonHeader["jwk"]);
      returnValue.Key = AttestationSignerInternal(jwk);
    }
    JsonOptional::SetIfExists(returnValue.KeyId, jsonHeader, "kid");
    JsonOptional::SetIfExists(returnValue.X509Url, jsonHeader, "x5u");
    JsonOptional::SetIfExists(returnValue.X509CertificateChain, jsonHeader, "x5c");
    JsonOptional::SetIfExists(returnValue.CertificateSha256Thumbprint, jsonHeader, "x5t#S256");
    JsonOptional::SetIfExists(returnValue.CertificateThumbprint, jsonHeader, "x5t");
    JsonOptional::SetIfExists(returnValue.Type, jsonHeader, "typ");
    JsonOptional::SetIfExists(returnValue.ContentType, jsonHeader, "cty");
    JsonOptional::SetIfExists(returnValue.Critical, jsonHeader, "crit");
    return returnValue;
  }
  std::string AttestationTokenHeaderSerializer::Serialize(AttestationTokenHeader const& tokenHeader)
  {
    Azure::Core::Json::_internal::json serializedHeader;
    JsonOptional::SetFromNullable(tokenHeader.Algorithm, serializedHeader, "alg");
    JsonOptional::SetFromNullable(tokenHeader.KeyURL, serializedHeader, "jku");
    JsonOptional::SetFromNullable<AttestationSigner, std::string>(
        tokenHeader.Key, serializedHeader, "jwk", [](AttestationSigner const& signer) {
          return AttestationSignerInternal::SerializeToJson(signer);
        });
    JsonOptional::SetFromNullable(tokenHeader.ContentType, serializedHeader, "cty");
    JsonOptional::SetFromNullable(tokenHeader.Critical, serializedHeader, "crit");
    JsonOptional::SetFromNullable(tokenHeader.KeyId, serializedHeader, "kid");
    JsonOptional::SetFromNullable(tokenHeader.Type, serializedHeader, "typ");

    JsonOptional::SetFromNullable(tokenHeader.X509CertificateChain, serializedHeader, "x5c");
    JsonOptional::SetFromNullable(tokenHeader.X509Url, serializedHeader, "x5u");
    JsonOptional::SetFromNullable(
        tokenHeader.CertificateSha256Thumbprint, serializedHeader, "x5t#S256");
    JsonOptional::SetFromNullable(tokenHeader.CertificateThumbprint, serializedHeader, "x5t");

    return serializedHeader.dump();
  }

  JsonWebKey JsonWebKeySerializer::Deserialize(const Azure::Core::Json::_internal::json& jwk)
  {
    JsonWebKey returnValue;
    JsonOptional::SetIfExists(returnValue.Kty, jwk, "kty");
    if (!returnValue.Kty)
    {
      throw std::runtime_error("Could not find required field 'kty' in JSON Web Key");
    }
    JsonOptional::SetIfExists(returnValue.Alg, jwk, "alg");
    JsonOptional::SetIfExists(returnValue.Kid, jwk, "kid");
    JsonOptional::SetIfExists(returnValue.Use, jwk, "use");
    JsonOptional::SetIfExists(returnValue.KeyOps, jwk, "key_ops");
    JsonOptional::SetIfExists(returnValue.X5t, jwk, "x5t");
    JsonOptional::SetIfExists(returnValue.X5t256, jwk, "x5t#S256");
    JsonOptional::SetIfExists(returnValue.X5u, jwk, "x5u");
    JsonOptional::SetIfExists(returnValue.X5c, jwk, "x5c");

    // ECDSA key values.
    JsonOptional::SetIfExists(returnValue.Crv, jwk, "crv");
    JsonOptional::SetIfExists(returnValue.X, jwk, "x");
    JsonOptional::SetIfExists(returnValue.Y, jwk, "y");
    JsonOptional::SetIfExists(returnValue.D, jwk, "d");

    // RSA key values.
    JsonOptional::SetIfExists(returnValue.N, jwk, "n");
    JsonOptional::SetIfExists(returnValue.E, jwk, "e");
    JsonOptional::SetIfExists(returnValue.Q, jwk, "p");
    JsonOptional::SetIfExists(returnValue.Dp, jwk, "dp");
    JsonOptional::SetIfExists(returnValue.Dq, jwk, "dq");
    JsonOptional::SetIfExists(returnValue.Qi, jwk, "qi");

    return returnValue;
  }

  std::string JsonWebKeySerializer::Serialize(
      Azure::Security::Attestation::Models::_detail::JsonWebKey const& jwk)
  {
    Azure::Core::Json::_internal::json serialized(SerializeToJson(jwk));
    return serialized.dump();
  }

  json JsonWebKeySerializer::SerializeToJson(
      Azure::Security::Attestation::Models::_detail::JsonWebKey const& jwk)
  {
    Azure::Core::Json::_internal::json serialized;
    JsonOptional::SetFromNullable(jwk.Kty, serialized, "kty");
    JsonOptional::SetFromNullable(jwk.Alg, serialized, "alg");
    JsonOptional::SetFromNullable(jwk.Kid, serialized, "kid");
    JsonOptional::SetFromNullable(jwk.Use, serialized, "use");
    JsonOptional::SetFromNullable(jwk.KeyOps, serialized, "key_ops");
    JsonOptional::SetFromNullable(jwk.X5t, serialized, "x5t");
    JsonOptional::SetFromNullable(jwk.X5t256, serialized, "x5t#S256");
    JsonOptional::SetFromNullable(jwk.X5u, serialized, "x5u");
    JsonOptional::SetFromNullable(jwk.X5c, serialized, "x5c");

    // ECDSA key values.
    JsonOptional::SetFromNullable(jwk.Crv, serialized, "crv");
    JsonOptional::SetFromNullable(jwk.X, serialized, "x");
    JsonOptional::SetFromNullable(jwk.Y, serialized, "y");
    JsonOptional::SetFromNullable(jwk.D, serialized, "d");

    // RSA key values.
    JsonOptional::SetFromNullable(jwk.N, serialized, "n");
    JsonOptional::SetFromNullable(jwk.E, serialized, "e");
    JsonOptional::SetFromNullable(jwk.Q, serialized, "p");
    JsonOptional::SetFromNullable(jwk.Dp, serialized, "dp");
    JsonOptional::SetFromNullable(jwk.Dq, serialized, "dq");
    JsonOptional::SetFromNullable(jwk.Qi, serialized, "qi");

    return serialized;
  }

  // cspell: words jwks
  JsonWebKeySet JsonWebKeySetSerializer::Deserialize(
      std::unique_ptr<Azure::Core::Http::RawResponse> const& response)
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
    for (const auto& key : parsedBody["keys"])
    {
      returnValue.Keys.push_back(JsonWebKeySerializer::Deserialize(key));
    }
    return returnValue;
  }

  Models::_detail::PolicyResult PolicyResultSerializer::Deserialize(
      Azure::Core::Json::_internal::json const& parsedResult)
  {
    Models::_detail::PolicyResult returnValue;
    JsonOptional::SetIfExists(returnValue.PolicyResolution, parsedResult, "x-ms-policy-result");
    JsonOptional::SetIfExists(returnValue.PolicyTokenHash, parsedResult, "x-ms-policy-token-hash");
    if (parsedResult.contains("x-ms-policy-signer"))
    {
      returnValue.PolicySigner
          = JsonWebKeySerializer::Deserialize(parsedResult["x-ms-policy-signer"]);
    }
    JsonOptional::SetIfExists(returnValue.PolicyToken, parsedResult, "x-ms-policy");
    return returnValue;
  }

  Models::_detail::StoredAttestationPolicy StoredAttestationPolicySerializer::Deserialize(
      Azure::Core::Json::_internal::json const& parsedResult)
  {
    Models::_detail::StoredAttestationPolicy returnValue;
    JsonOptional::SetIfExists<std::string, std::vector<uint8_t>>(
        returnValue.AttestationPolicy,
        parsedResult,
        "AttestationPolicy",
        Azure::Core::_internal::Base64Url::Base64UrlDecode);

    return returnValue;
  }

  std::string StoredAttestationPolicySerializer::Serialize(
      Models::_detail::StoredAttestationPolicy const& storedPolicy)
  {
    Azure::Core::Json::_internal::json serializedPolicy;

    JsonOptional::SetFromNullable<std::vector<uint8_t>, std::string>(
        storedPolicy.AttestationPolicy,
        serializedPolicy,
        "AttestationPolicy",
        Azure::Core::_internal::Base64Url::Base64UrlEncode);

    return serializedPolicy.dump();
  }

  Models::_detail::GetPolicyCertificatesResult PolicyCertificateGetResultSerializer::Deserialize(
      Azure::Core::Json::_internal::json const& parsedResult)
  {
    Models::_detail::GetPolicyCertificatesResult returnValue;
    if (parsedResult.contains("x-ms-policy-certificates"))
    {
      returnValue.PolicyCertificates
          = JsonWebKeySetSerializer::Deserialize(parsedResult["x-ms-policy-certificates"]);
    }
    return returnValue;
  }

  std::string PolicyCertificateManagementBodySerializer::Serialize(
      Models::_detail::PolicyCertificateManagementBody const& body)
  {
    Azure::Core::Json::_internal::json serializedPolicy;
    serializedPolicy["policyCertificate"]
        = JsonWebKeySerializer::SerializeToJson(body.policyCertificate);

    return serializedPolicy.dump();
  }

  Models::_detail::PolicyCertificateManagementBody
  PolicyCertificateManagementBodySerializer::Deserialize(
      Azure::Core::Json::_internal::json const& jsonBody)
  {
    Models::_detail::PolicyCertificateManagementBody body;
    JsonOptional::SetIfExists<json, JsonWebKey>(
        body.policyCertificate, jsonBody, "policyCertificate", JsonWebKeySerializer::Deserialize);
    return body;
  }

  Models::_detail::ModifyPolicyCertificatesResult
  ModifyPolicyCertificatesResultSerializer::Deserialize(
      Azure::Core::Json::_internal::json const& jsonResult)
  {
    Models::_detail::ModifyPolicyCertificatesResult returnValue;
    JsonOptional::SetIfExists(
        returnValue.CertificateResolution, jsonResult, "x-ms-policycertificates-result");
    JsonOptional::SetIfExists(
        returnValue.CertificateThumbprint, jsonResult, "x-ms-certificate-thumbprint");
    return returnValue;
  }
  std::string TpmDataSerializer::Serialize(std::string const& tpmData)
  {
    Azure::Core::Json::_internal::json jsonData;
    jsonData["data"] = Azure::Core::_internal::Base64Url::Base64UrlEncode(
        std::vector<uint8_t>(tpmData.begin(), tpmData.end()));
    return jsonData.dump();
  }
  std::string TpmDataSerializer::Deserialize(Azure::Core::Json::_internal::json const& jsonData)
  {
    std::vector<uint8_t> returnValue;
    JsonOptional::SetIfExists<std::string, std::vector<uint8_t>>(
        returnValue, jsonData, "data", Azure::Core::_internal::Base64Url::Base64UrlDecode);
    return std::string(returnValue.begin(), returnValue.end());
  }
  std::string TpmDataSerializer::Deserialize(
      std::unique_ptr<Azure::Core::Http::RawResponse> const& response)
  {
    return TpmDataSerializer::Deserialize(
        Azure::Core::Json::_internal::json::parse(response->GetBody()));
  }

}}}} // namespace Azure::Security::Attestation::_detail

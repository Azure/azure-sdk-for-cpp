// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Azure::Core::Json::_internal::json Deserialization support functions.
 *
 * This file contains a set of support functions to aid in serializing and deserializing JSON
 * objects. It also contains Deserializer classes, one for each model type which each support a
 * static Serialize and Deserialize function which serialize and deserialize the specified model
 * types from and to JSON objects.
 *
 */

#pragma once

#include "azure/core/base64.hpp"
#include "azure/core/context.hpp"
#include "azure/core/http/http.hpp"
#include "azure/core/internal/http/pipeline.hpp"
#include "azure/core/internal/json/json.hpp"
#include "azure/core/internal/json/json_optional.hpp"
#include "azure/core/internal/json/json_serializable.hpp"
#include "azure/core/nullable.hpp"
#include "azure/core/response.hpp"
#include "jsonhelpers.hpp"
#include "jsonwebkeyset.hpp"

//#include "attestation_client_models_private.hpp"
#include "azure/attestation/attestation_client_models.hpp"
#include <memory>
#include <string>
#include <vector>

// cspell: words jwks MrSigner MrEnclave
namespace Azure { namespace Security { namespace Attestation { namespace _detail {

  /***************************************
   * A quick note on the naming convention for the Serialize/Deserialize classes.
   *
   * Serialization classes are named "XxxxSerializer". They contain one or two static methods named
   * "Serialize" and "Deserialize".
   *
   * The Serialize method takes an instance of an "Xxxx" object and returns a std::string which
   * represents the Xxxx object serialized into JSON.
   *
   * The Deserialize method takes an instance of a json object and returns an instance of the Xxxx
   * type.
   *
   */

  struct OpenIdMetadataSerializer final
  {
    static Models::AttestationOpenIdMetadata Deserialize(
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
  };

  struct AttestSgxEnclaveRequestSerializer final
  {
    static std::string Serialize(
        Azure::Security::Attestation::Models::_detail::AttestSgxEnclaveRequest const& request)
    {
      Azure::Core::Json::_internal::json serializedRequest;
      serializedRequest["quote"]
          = Azure::Core::_internal::Base64Url::Base64UrlEncode(request.Quote);
      if (request.RunTimeData.HasValue())
      {
        serializedRequest["runtimeData"] = {
            {"data",
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
  };
  struct AttestOpenEnclaveRequestSerializer final
  {
    static std::string Serialize(
        Azure::Security::Attestation::Models::_detail::AttestOpenEnclaveRequest const& request)
    {
      Azure::Core::Json::_internal::json serializedRequest;
      serializedRequest["report"]
          = Azure::Core::_internal::Base64Url::Base64UrlEncode(request.Report);

      if (request.RunTimeData.HasValue())
      {
        serializedRequest["runtimeData"] = {
            {"data",
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
  };

  struct AttestationServiceTokenResponseSerializer final
  {
    static std::string Deserialize(Azure::Core::Json::_internal::json const& parsedBody)
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

    static std::string Deserialize(std::unique_ptr<Azure::Core::Http::RawResponse>& response)
    {
      auto parsedBody = Azure::Core::Json::_internal::json::parse(response->GetBody());
      return Deserialize(parsedBody);
    }
  };

  struct AttestationResultSerializer final
  {
  public:
    static Models::AttestationResult Deserialize(
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
        result.PolicySigner
            = Azure::Security::Attestation::Models::_detail::AttestationSignerInternal(
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
  };

}}}} // namespace Azure::Security::Attestation::_detail

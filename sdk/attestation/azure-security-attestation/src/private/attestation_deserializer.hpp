// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief JSON Deserialization support functions.
 * 
 * This file contains a set of support functions to aid in serializing and deserializing JSON objects.
 * It also contains Deserializer classes, one for each model type which each support a static Serialize and Deserialize
 * function which serialize and deserialize the specified model types from and to JSON objects.
 *
 */

#pragma once

#include <azure/core/context.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_serializable.hpp>
#include <azure/core/internal/json/json_optional.hpp>
#include <azure/core/nullable.hpp>
#include <azure/core/base64.hpp>
#include <azure/core/response.hpp>
#include <azure/core/base64.hpp>
#include "jsonwebkeyset.hpp"

#include <azure/attestation/attestation_client_models.hpp>
#include "attestation_client_models_private.hpp"
#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Security { namespace Attestation{ namespace _detail {

  using namespace Azure::Core::Json::_internal;
  using namespace Azure::Core::Http;
  using namespace Azure::Security::Attestation::Models::_detail;
  using namespace Azure::Core::_internal;

  std::string ParseStringField(
      const json& field,
      const std::string& fieldName,
      std::unique_ptr<RawResponse>& response)
  {
    if (field.contains(fieldName))
    {
      const auto& fieldVal = field[fieldName];
      if (!fieldVal.is_string())
      {
        throw Azure::Core::RequestFailedException(
            "Field " + fieldName + " is not a string.", response);
      }
      return fieldVal.get<std::string>();
    }
    return "";
  }

  std::vector<std::string> ParseStringArrayField(
      const json& field,
      const std::string& fieldName,
      std::unique_ptr<RawResponse>& response)
  {
    std::vector<std::string> returnValue;
    if (field.contains(fieldName))
    {
      const auto& fieldVal = field[fieldName];
      if (!fieldVal.is_array())
      {
        throw Azure::Core::RequestFailedException(
            "Field " + fieldName + " is not an array.", response);
      }
      for (const auto& item : fieldVal)
      {
        if (!item.is_string())
        {
          throw Azure::Core::RequestFailedException(
              "Field " + fieldName + " element is not a string.");
        }
        returnValue.push_back(item.get<std::string>());
      }
    }
    return returnValue;
  }

  std::string ParseStringJsonField(
      const json& field,
      const std::string& fieldName,
      std::unique_ptr<RawResponse>& response)
  {
    std::string returnValue;
    if (field.contains(fieldName))
    {
      const auto& fieldVal = field[fieldName];
      if (!fieldVal.is_object())
      {
        throw Azure::Core::RequestFailedException(
            "Field " + fieldName + " is not an object.", response);
      }
      returnValue = field[fieldName].dump();
    }
    return returnValue;
  }

  std::vector<uint8_t> ParseBase64UrlField(
      const json& field,
      const std::string& fieldName,
      std::unique_ptr<RawResponse>& response)
  {
    std::vector<uint8_t> returnValue;
    if (field.contains(fieldName))
    {
      const auto& fieldVal = field[fieldName];
      if (!fieldVal.is_string())
      {
        throw Azure::Core::RequestFailedException(
            "Field " + fieldName + " is not a string.", response);
      }
      returnValue = Base64Url::Base64UrlDecode(field[fieldName].get<std::string>());
    }
    return returnValue;
  }

  Azure::Nullable<bool> ParseBooleanField(
      const json& field,
      const std::string& fieldName,
      std::unique_ptr<RawResponse>& response)
  {
    Azure::Nullable<bool> returnValue;
    if (field.contains(fieldName))
    {
      const auto& fieldVal = field[fieldName];
      if (!fieldVal.is_boolean())
      {
        throw Azure::Core::RequestFailedException(
            "Field " + fieldName + " is not a boolean.", response);
      }
      returnValue.Emplace(field[fieldName].get<bool>());
    }
    return returnValue;
  }

    Azure::Nullable<int> ParseIntNumberField(
      const json& field,
      const std::string& fieldName,
      std::unique_ptr<RawResponse>& response)
  {
    Azure::Nullable<int> returnValue;
    if (field.contains(fieldName))
    {
      const auto& fieldVal = field[fieldName];
      if (!fieldVal.is_number_integer())
      {
        throw Azure::Core::RequestFailedException(
            "Field " + fieldName + " is not a number.", response);
      }
      returnValue.Emplace(field[fieldName].get<int>());
    }
    return returnValue;
  }



  struct OpenIdMetadataDeserializer final
  {
    static AttestationOpenIdMetadata Deserialize(std::unique_ptr<RawResponse>& response)
    {
      AttestationOpenIdMetadata returnValue;
      auto parsedBody = json::parse(response->GetBody());
      returnValue.Issuer = ParseStringField(parsedBody, "issuer", response);
      returnValue.JsonWebKeySetUrl = ParseStringField(parsedBody, "jwks_uri", response);
      returnValue.SupportedClaims = ParseStringArrayField(parsedBody, "claims_supported", response);
      returnValue.SupportedTokenSigningAlgorithms
          = ParseStringArrayField(parsedBody, "id_token_signing_alg_values_supported", response);
      returnValue.SupportedResponseTypes
          = ParseStringArrayField(parsedBody, "response_types_supported", response);
      return returnValue;
    }
  };

struct JsonWebKeySerializer final
  {
    static JsonWebKey Deserialize(std::unique_ptr<RawResponse>& response, const json& jwk)
    {
      JsonWebKey returnValue;
      returnValue.kty = ParseStringField(jwk, "kty", response);
      if (returnValue.kty.empty())
      {
        throw Azure::Core::RequestFailedException(
            "JsonWebKey missing required field 'kty'", response);
      }
      returnValue.alg = ParseStringField(jwk, "alg", response);
      returnValue.kid = ParseStringField(jwk, "kid", response);
      returnValue.use = ParseStringField(jwk, "use", response);
      returnValue.keyops = ParseStringArrayField(jwk, "key_ops", response);
      returnValue.x5t = ParseStringField(jwk, "x5t", response);
      returnValue.x5t256 = ParseStringField(jwk, "x5t#S256", response);
      returnValue.x5u = ParseStringField(jwk, "x5u", response);
      returnValue.x5c = ParseStringArrayField(jwk, "x5c", response);

      // ECDS key values.
      returnValue.crv = ParseStringField(jwk, "crv", response);
      returnValue.x = ParseStringField(jwk, "x", response);
      returnValue.y = ParseStringField(jwk, "y", response);
      returnValue.d = ParseStringField(jwk, "d", response);

      // RSA key values.
      returnValue.n = ParseStringField(jwk, "n", response);
      returnValue.e = ParseStringField(jwk, "e", response);
      returnValue.q = ParseStringField(jwk, "p", response);
      returnValue.dp = ParseStringField(jwk, "dp", response);
      returnValue.dq = ParseStringField(jwk, "dq", response);
      returnValue.qi = ParseStringField(jwk, "qi", response);

      return returnValue;
      response;
    }
  };


  struct JsonWebKeySetSerializer final
  {
    static JsonWebKeySet Deserialize(std::unique_ptr<RawResponse>& response)
    {
      JsonWebKeySet returnValue;
      auto parsedBody = json::parse(response->GetBody());
      if (!parsedBody.contains("keys"))
      {
        throw Azure::Core::RequestFailedException(
            "Field 'keys' not found in JWKS.", response);
      }
      if (!parsedBody["keys"].is_array())
      {
        throw Azure::Core::RequestFailedException("Field 'keys' is not an array.", response);
      }
      for (const auto key : parsedBody["keys"])
      {
        returnValue.Keys.push_back(JsonWebKeySerializer::Deserialize(response, key));
      }
      return returnValue;
    };
  };


  struct AttestSgxEnclaveRequestSerializer final
  {
    static std::string Serialize(AttestSgxEnclaveRequest const & request)
    {
      json serializedRequest;
      serializedRequest["quote"]
          = Azure::Core::_internal::Base64Url::Base64UrlEncode(request.Quote);
      if (!request.RunTimeData.Data.empty())
      {
        serializedRequest["runtimeData"] = {
            {"data", Azure::Core::_internal::Base64Url::Base64UrlEncode(request.RunTimeData.Data)},
            {"dataType", request.RunTimeData.DataType.ToString()}};
      }
      if (!request.InitTimeData.Data.empty())
      {
        serializedRequest["inittimeData"] = {
            {"data", Azure::Core::_internal::Base64Url::Base64UrlEncode(request.InitTimeData.Data)},
            {"dataType", request.InitTimeData.DataType.ToString()}};
      }
      if (!request.DraftPolicyForAttestation.empty())
      {
        serializedRequest["draftPolicyForAttestation"] = request.DraftPolicyForAttestation;
      }
      return serializedRequest.dump();
    }
  };
  struct AttestOpenEnclaveRequestSerializer final
  {
    static std::string Serialize(AttestOpenEnclaveRequest const& request)
    {
      json serializedRequest;
      serializedRequest["report"]
          = Azure::Core::_internal::Base64Url::Base64UrlEncode(request.Report);

      if (!request.RunTimeData.Data.empty())
      {
        serializedRequest["runtimeData"] = {
            {"data", Azure::Core::_internal::Base64Url::Base64UrlEncode(request.RunTimeData.Data)},
            {"dataType", request.RunTimeData.DataType.ToString()}};
      }

      if (!request.InitTimeData.Data.empty())
      {
        serializedRequest["inittimeData"] = {
            {"data", Azure::Core::_internal::Base64Url::Base64UrlEncode(request.InitTimeData.Data)},
            {"dataType", request.InitTimeData.DataType.ToString()}};
      }

      if (!request.DraftPolicyForAttestation.empty())
      {
        serializedRequest["draftPolicyForAttestation"] = request.DraftPolicyForAttestation;
      }
      return serializedRequest.dump();
    }
  };

  struct AttestationServiceTokenResponseSerializer final
  {
      static std::string Deserialize(std::unique_ptr<RawResponse>& response) {
        auto parsedBody = json::parse(response->GetBody());
        if (!parsedBody.contains("token"))
        {
          throw new Azure::Core::RequestFailedException(
              "Field 'token' not found in Attestation Service Response", response);
        }
        if (!parsedBody["token"].is_string())
        {
          throw new Azure::Core::RequestFailedException("Field 'token' is not a string", response);
        }
        return parsedBody["token"].get<std::string>();
    }
  };

  struct AttestationResultDeserializer final
  {
  public:
    static AttestationResult Deserialize(
        json const& parsedJson,
        std::unique_ptr<RawResponse>& response)
    {
      AttestationResult result;

      result.Issuer = ParseStringField(parsedJson, "iss", response);
      result.UniqueIdentifier = ParseStringField(parsedJson, "jti", response);
      result.Nonce = ParseStringField(parsedJson, "nonce", response);
      result.Version = ParseStringField(parsedJson, "x-ms-ver", response);
      result.RuntimeClaims = ParseStringJsonField(parsedJson, "x-ms-runtime", response);
      result.InitTimeClaims= ParseStringJsonField(parsedJson, "x-ms-inittime", response);
      result.PolicyClaims= ParseStringJsonField(parsedJson, "x-ms-policy", response);
      result.VerifierType = ParseStringField(parsedJson, "x-ms-attestation-type", response);
      if (parsedJson.contains("x-ms-policy-signer"))
      {
        result.PolicySigner = AttestationSignerInternal(
            JsonWebKeySerializer::Deserialize(response, parsedJson["x-ms-policy-signer"]));
      }
      result.PolicyHash = ParseBase64UrlField(parsedJson, "x-ms-policy-hash", response);
      result.IsDebuggable = ParseBooleanField(parsedJson, "x-ms-sgx-is-debuggable", response);
      result.ProductId = ParseIntNumberField(parsedJson, "x-ms-sgx-product-id", response);
      result.MrEnclave = ParseBase64UrlField(parsedJson, "x-ms-sgx-mrenclave", response);
      result.MrSigner = ParseBase64UrlField(parsedJson, "x-ms-sgx-mrsigner", response);
      result.EnclaveHeldData = ParseBase64UrlField(parsedJson, "x-ms-sgx-ehd", response);
      result.SgxCollateral = ParseStringJsonField(parsedJson, "x-ms-sgx-collateral", response);
      return result;
    }
  };

}}}} // namespace Azure::Security::KeyVault::_detail

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief JSON Deserialization support functions.
 *
 * This file contains a set of support functions to aid in serializing and deserializing JSON
 * objects. It also contains Deserializer classes, one for each model type which each support a
 * static Serialize and Deserialize function which serialize and deserialize the specified model
 * types from and to JSON objects.
 *
 */

#pragma once

#include "jsonwebkeyset.hpp"
#include <azure/core/base64.hpp>
#include <azure/core/context.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_optional.hpp>
#include <azure/core/internal/json/json_serializable.hpp>
#include <azure/core/nullable.hpp>
#include <azure/core/response.hpp>

#include "attestation_client_models_private.hpp"
#include <azure/attestation/attestation_client_models.hpp>
#include <memory>
#include <string>
#include <vector>

// cspell: words jwks MrSigner MrEnclave
namespace Azure { namespace Security { namespace Attestation { namespace _detail {

  using namespace Azure::Core::Json::_internal;
  using namespace Azure::Core::Http;
  using namespace Azure::Core::_internal;

  struct JsonHelpers
  {
    static std::string ParseStringField(const json& field, const std::string& fieldName)
    {
      if (field.contains(fieldName))
      {
        const auto& fieldVal = field[fieldName];
        if (!fieldVal.is_string())
        {
          throw std::runtime_error("Field " + fieldName + " is not a string.");
        }
        return fieldVal.get<std::string>();
      }
      return "";
    }

    static std::vector<std::string> ParseStringArrayField(
        const json& field,
        const std::string& fieldName)
    {
      std::vector<std::string> returnValue;
      if (field.contains(fieldName))
      {
        const auto& fieldVal = field[fieldName];
        if (!fieldVal.is_array())
        {
          throw std::runtime_error("Field " + fieldName + " is not an array.");
        }
        for (const auto& item : fieldVal)
        {
          if (!item.is_string())
          {
            throw std::runtime_error("Field " + fieldName + " element is not a string.");
          }
          returnValue.push_back(item.get<std::string>());
        }
      }
      return returnValue;
    }

    static Azure::Nullable<std::vector<int>> ParseIntArrayField(
        const json& field,
        const std::string& fieldName)
    {
      std::vector<int> returnValue;
      if (field.contains(fieldName))
      {
        const auto& fieldVal = field[fieldName];
        if (!fieldVal.is_array())
        {
          throw std::runtime_error("Field " + fieldName + " is not an array.");
        }
        for (const auto& item : fieldVal)
        {
          if (!item.is_number_integer())
          {
            throw std::runtime_error("Field " + fieldName + " element is not an integer.");
          }
          returnValue.push_back(item.get<int>());
        }
        return returnValue;
      }
      return Azure::Nullable<std::vector<int>>();
    }

    static std::string ParseStringJsonField(const json& field, const std::string& fieldName)
    {
      std::string returnValue;
      if (field.contains(fieldName))
      {
        const auto& fieldVal = field[fieldName];
        if (!fieldVal.is_object())
        {
          throw std::runtime_error("Field " + fieldName + " is not an object.");
        }
        returnValue = field[fieldName].dump();
      }
      return returnValue;
    }

    static Azure::Nullable<Azure::DateTime> ParseDateTimeField(
        json const& object,
        std::string const& fieldName)
    {
      if (object.contains(fieldName))
      {
        const auto& fieldVal = object[fieldName];
        if (!fieldVal.is_number())
        {
          throw std::runtime_error("Field " + fieldName + " is not a number.");
        }

        int64_t epochTime = fieldVal.get<int64_t>();
        return Azure::Core::_internal::PosixTimeConverter::PosixTimeToDateTime(epochTime);
      }
      return Azure::Nullable<Azure::DateTime>();
    }

    static std::vector<uint8_t> ParseBase64UrlField(const json& field, const std::string& fieldName)
    {
      std::vector<uint8_t> returnValue;
      if (field.contains(fieldName))
      {
        const auto& fieldVal = field[fieldName];
        if (!fieldVal.is_string())
        {
          throw std::runtime_error(std::string("Field ") + fieldName + " is not a string.");
        }
        returnValue = Base64Url::Base64UrlDecode(field[fieldName].get<std::string>());
      }
      return returnValue;
    }

    static Azure::Nullable<bool> ParseBooleanField(const json& field, const std::string& fieldName)
    {
      if (field.contains(fieldName))
      {
        const auto& fieldVal = field[fieldName];
        if (!fieldVal.is_boolean())
        {
          throw std::runtime_error("Field " + fieldName + " is not a boolean.");
        }
        return field[fieldName].get<bool>();
      }
      return Azure::Nullable<bool>();
    }

    static Azure::Nullable<int> ParseIntNumberField(const json& field, const std::string& fieldName)
    {
      if (field.contains(fieldName))
      {
        const auto& fieldVal = field[fieldName];
        if (!fieldVal.is_number_integer())
        {
          throw std::runtime_error("Field " + fieldName + " is not a number.");
        }
        return field[fieldName].get<int>();
      }
      return Azure::Nullable<int>();
    }

    // Serialization helpers.

    static void SetField(json& object, std::string const& fieldValue, std::string const& fieldName)
    {
      object[fieldName] = fieldValue;
    }
    static void SetField(json& object, int fieldValue, std::string const& fieldName)
    {
      object[fieldName] = fieldValue;
    }
    static void SetField(
        json& object,
        Azure::Nullable<int> const& fieldValue,
        std::string const& fieldName)
    {
      if (fieldValue.HasValue())
      {
        SetField(object, fieldValue.Value(), fieldName);
      }
    }
    static void SetField(
        json& object,
        std::vector<int> const& fieldValue,
        std::string const& fieldName)
    {
      object[fieldName] = fieldValue;
    }
    static void SetField(
        json& object,
        Azure::Nullable<std::vector<int>> const& fieldValue,
        std::string const& fieldName)
    {
      if (fieldValue.HasValue())
      {
        SetField(object, fieldValue.Value(), fieldName);
      }
    }

    static void SetField(json& object, Azure::Nullable<Azure::DateTime> const&fieldValue, std::string const& fieldName)
    {
      if (fieldValue.HasValue())
      {
        SetField(object, fieldValue.Value(), fieldName);
      }
    }
    static void SetField(
        json& object,
        Azure::DateTime const& fieldValue,
        std::string const& fieldName)
    {
      object[fieldName] = Azure::Core::_internal::PosixTimeConverter::DateTimeToPosixTime(fieldValue);
    }
  };

  struct OpenIdMetadataDeserializer final
  {
    static AttestationOpenIdMetadata Deserialize(std::unique_ptr<RawResponse>& response)
    {
      AttestationOpenIdMetadata returnValue;
      auto parsedBody = json::parse(response->GetBody());
      returnValue.Issuer = JsonHelpers::ParseStringField(parsedBody, "issuer");
      returnValue.JsonWebKeySetUrl = JsonHelpers::ParseStringField(parsedBody, "jwks_uri");
      returnValue.SupportedClaims
          = JsonHelpers::ParseStringArrayField(parsedBody, "claims_supported");
      returnValue.SupportedTokenSigningAlgorithms
          = JsonHelpers::ParseStringArrayField(parsedBody, "id_token_signing_alg_values_supported");
      returnValue.SupportedResponseTypes
          = JsonHelpers::ParseStringArrayField(parsedBody, "response_types_supported");
      return returnValue;
    }
  };

  struct JsonWebKeySerializer final
  {
    static JsonWebKey Deserialize(const json& jwk)
    {
      JsonWebKey returnValue;
      returnValue.kty = JsonHelpers::ParseStringField(jwk, "kty");
      if (returnValue.kty.empty())
      {
        throw std::runtime_error("JsonWebKey missing required field 'kty'");
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
  };

  struct JsonWebKeySetSerializer final
  {
    static JsonWebKeySet Deserialize(std::unique_ptr<RawResponse>& response)
    {
      auto parsedBody = json::parse(response->GetBody());
      return Deserialize(parsedBody);
    }
    static JsonWebKeySet Deserialize(json const& parsedBody)
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

  struct AttestSgxEnclaveRequestSerializer final
  {
    static std::string Serialize(
        Azure::Security::Attestation::Models::_detail::AttestSgxEnclaveRequest const& request)
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
    static std::string Serialize(
        Azure::Security::Attestation::Models::_detail::AttestOpenEnclaveRequest const& request)
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
    static std::string Deserialize(json const& parsedBody)
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

    static std::string Deserialize(std::unique_ptr<RawResponse>& response)
    {
      auto parsedBody = json::parse(response->GetBody());
      return Deserialize(parsedBody);
    }
  };

  struct AttestationResultDeserializer final
  {
  public:
    static AttestationResult Deserialize(json const& parsedJson)
    {
      AttestationResult result;

      result.Issuer = JsonHelpers::ParseStringField(parsedJson, "iss");
      result.UniqueIdentifier = JsonHelpers::ParseStringField(parsedJson, "jti");
      result.Nonce = JsonHelpers::ParseStringField(parsedJson, "nonce");
      result.Version = JsonHelpers::ParseStringField(parsedJson, "x-ms-ver");
      result.RuntimeClaims = JsonHelpers::ParseStringJsonField(parsedJson, "x-ms-runtime");
      result.InitTimeClaims = JsonHelpers::ParseStringJsonField(parsedJson, "x-ms-inittime");
      result.PolicyClaims = JsonHelpers::ParseStringJsonField(parsedJson, "x-ms-policy");
      result.VerifierType = JsonHelpers::ParseStringField(parsedJson, "x-ms-attestation-type");
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

// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX-License-Identifier: MIT

#include "../../src/private/attestation_client_models_private.hpp"
#include "../../src/private/attestation_client_private.hpp"
#include "../../src/private/attestation_deserializers_private.hpp"
#include "../../src/private/crypto/inc/crypto.hpp"
#include "azure/attestation/attestation_client.hpp"
#include <azure/core/datetime.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_optional.hpp>
#include <azure/core/test/test_base.hpp>
#include <gtest/gtest.h>

// cspell:words jwk jwks
namespace Azure { namespace Security { namespace Attestation { namespace Test {
  using namespace Azure::Core::Json::_internal;
  using namespace Azure::Core::Http;
  using namespace Azure::Security::Attestation;
  using namespace Azure::Security::Attestation::_detail;
  using namespace Azure::Security::Attestation::Models;
  using namespace Azure::Security::Attestation::Models::_detail;
  using namespace Azure::Core::_internal;

  TEST(SerializationTests, TestDeserializePrimitivesJsonObject)
  {
    // Present JSON field.
    {
      Azure::Nullable<std::string> val;

      JsonHelpers::SetIfExistsJson(
          val, json::parse(R"({ "jsonObjectValue": {"stringField": "SF2"}})"), "jsonObjectValue");
      EXPECT_TRUE(val);
      EXPECT_EQ(R"({"stringField":"SF2"})", *val);
    }
    // Not present field.
    {
      Azure::Nullable<std::string> val;
      JsonHelpers::SetIfExistsJson(
          val, json::parse("{ \"objectValue\":{\"String Field\": 27}}"), "intValue");
      EXPECT_FALSE(val);
    }
  }

  TEST(SerializationTests, TestDeserializePrimitivesBase64Url)
  {
    std::string testData("Test Data");
    std::string encodedData = Azure::Core::_internal::Base64Url::Base64UrlEncode(
        std::vector<uint8_t>(testData.begin(), testData.end()));

    // Present JSON field.
    {
      Azure::Nullable<std::vector<uint8_t>> val;
      JsonOptional::SetIfExists<std::string, std::vector<uint8_t>>(
          val,
          json::parse("{ \"base64Urlfield\": \"" + encodedData + "\"}"),
          "base64Urlfield",
          Base64Url::Base64UrlDecode);
      EXPECT_EQ(9ul, val->size());
      EXPECT_EQ(testData, std::string(val->begin(), val->end()));
    }
    // Not present field.
    {
      Azure::Nullable<std::vector<uint8_t>> val;
      JsonOptional::SetIfExists<std::string, std::vector<uint8_t>>(
          val,
          json::parse("{ \"base64Urlfield\": \"" + encodedData + "\"}"),
          "intValue",
          Base64Url::Base64UrlDecode);

      EXPECT_FALSE(val);
    }
  }

  TEST(SerializationTests, TestHexString)
  {
    {
      auto bin(JsonHelpers::HexStringToBinary("010203AABBccddee"));
      EXPECT_EQ(bin[0], 0x01);
      EXPECT_EQ(bin[1], 0x02);
      EXPECT_EQ(bin[2], 0x03);
      EXPECT_EQ(bin[3], 0xaa);
      EXPECT_EQ(bin[4], 0xbb);
      EXPECT_EQ(bin[5], 0xcc);
      EXPECT_EQ(bin[6], 0xdd);
      EXPECT_EQ(bin[7], 0xee);
    }
    {
      // cspell: disable
      EXPECT_THROW(JsonHelpers::HexStringToBinary("ABCEQWERTY"), std::invalid_argument);
      // cspell: enable
    }
    {
      // cspell: disable
      EXPECT_THROW(JsonHelpers::HexStringToBinary("ABC"), std::invalid_argument);
      // cspell: enable
    }
  }

  TEST(SerializationTests, TestDeserializeJWK)
  {
    {
      EXPECT_THROW(
          JsonWebKeySerializer::Deserialize(json::parse(R"({"alg": "none"})")), std::runtime_error);
    }
    // cspell:disable
    {
      auto val(JsonWebKeySerializer::Deserialize(json::parse(R"(
      {"kty":"EC",
       "crv":"P-256",
       "x":"MKBCTNIcKUSDii11ySs3526iDZ8AiTo7Tu6KPAqv7D4",
       "y":"4Etl6SRW2YiLUrN5vfvVHuhp7x8PxltmWWlbbM4IFyM",
       "use":"enc",
       "kid":"1"})")));
      EXPECT_EQ("EC", *val.kty);
      EXPECT_EQ("P-256", *val.crv);
      EXPECT_EQ("MKBCTNIcKUSDii11ySs3526iDZ8AiTo7Tu6KPAqv7D4", *val.x);
      EXPECT_EQ("4Etl6SRW2YiLUrN5vfvVHuhp7x8PxltmWWlbbM4IFyM", *val.y);
      EXPECT_EQ("enc", *val.use);
      EXPECT_EQ("1", *val.kid);
    }

    {
      auto val(JsonWebKeySerializer::Deserialize(json::parse(R"({"kty":"RSA",
       "n": "0vx7agoebGcQSuuPiLJXZptN9nndrQmbXEps2aiAFbWhM78LhWx4cbbfAAtVT86zwu1RK7aPFFxuhDR1L6tSoc_BJECPebWKRXjBZCiFV4n3oknjhMstn64tZ_2W-5JsGY4Hc5n9yBXArwl93lqt7_RN5w6Cf0h4QyQ5v-65YGjQR0_FDW2QvzqY368QQMicAtaSqzs8KJZgnYb9c7d0zgdAZHzu6qMQvRL5hajrn1n91CbOpbISD08qNLyrdkt-bFTWhAI4vMQFh6WeZu0fM4lFd2NcRwr3XPksINHaQ-G_xBniIqbw0Ls1jF44-csFCur-kEgU8awapJzKnqDKgw",
       "e":"AQAB",
       "alg":"RS256",
       "kid":"2011-04-29"})")));
      EXPECT_EQ("RS256", *val.alg);
      EXPECT_EQ(
          "0vx7agoebGcQSuuPiLJXZptN9nndrQmbXEps2aiAFbWhM78LhWx4cbbfAAtVT86zwu1RK7aPFFxuhDR1L6tSoc_"
          "BJECPebWKRXjBZCiFV4n3oknjhMstn64tZ_2W-5JsGY4Hc5n9yBXArwl93lqt7_RN5w6Cf0h4QyQ5v-65YGjQR0_"
          "FDW2QvzqY368QQMicAtaSqzs8KJZgnYb9c7d0zgdAZHzu6qMQvRL5hajrn1n91CbOpbISD08qNLyrdkt-"
          "bFTWhAI4vMQFh6WeZu0fM4lFd2NcRwr3XPksINHaQ-G_xBniIqbw0Ls1jF44-csFCur-kEgU8awapJzKnqDKgw",
          *val.n);
      EXPECT_EQ("AQAB", *val.e);
      EXPECT_EQ("2011-04-29", *val.kid);

      // Now serialize the JWK back to JSON.
      auto serializedKey(JsonWebKeySerializer::Serialize(val));
    }
    // cspell:enable
  }
  TEST(SerializationTests, TestPolicyCertificateManagementBody)
  {
    PolicyCertificateManagementBody body;
    // cspell: disable
    body.policyCertificate = JsonWebKeySerializer::Deserialize(json::parse(R"({"kty":"RSA",
       "n": "0vx7agoebGcQSuuPiLJXZptN9nndrQmbXEps2aiAFbWhM78LhWx4cbbfAAtVT86zwu1RK7aPFFxuhDR1L6tSoc_BJECPebWKRXjBZCiFV4n3oknjhMstn64tZ_2W-5JsGY4Hc5n9yBXArwl93lqt7_RN5w6Cf0h4QyQ5v-65YGjQR0_FDW2QvzqY368QQMicAtaSqzs8KJZgnYb9c7d0zgdAZHzu6qMQvRL5hajrn1n91CbOpbISD08qNLyrdkt-bFTWhAI4vMQFh6WeZu0fM4lFd2NcRwr3XPksINHaQ-G_xBniIqbw0Ls1jF44-csFCur-kEgU8awapJzKnqDKgw",
       "e":"AQAB",
       "alg":"RS256",
       "kid":"2011-04-29"})"));
    // cspell: enable
    std::string serializedBody = PolicyCertificateManagementBodySerializer::Serialize(body);
    auto deserializedBody
        = PolicyCertificateManagementBodySerializer::Deserialize(json::parse(serializedBody));
    EXPECT_EQ(body.policyCertificate.n.Value(), deserializedBody.policyCertificate.n.Value());
  }

  TEST(SerializationTests, TestDeserializeJWKS)
  {
    {
      EXPECT_THROW(
          JsonWebKeySetSerializer::Deserialize(json::parse(R"({"keys": [{"alg": "none"}]})")),
          std::runtime_error);
    }
    // cspell:disable
    {
      auto val(JsonWebKeySetSerializer::Deserialize(json::parse(R"(
    {"keys": [
      {"kty":"EC",
       "crv":"P-256",
       "x":"MKBCTNIcKUSDii11ySs3526iDZ8AiTo7Tu6KPAqv7D4",
       "y":"4Etl6SRW2YiLUrN5vfvVHuhp7x8PxltmWWlbbM4IFyM",
       "use":"enc",
       "kid":"1"},

      {"kty":"RSA",
       "n": "0vx7agoebGcQSuuPiLJXZptN9nndrQmbXEps2aiAFbWhM78LhWx4cbbfAAtVT86zwu1RK7aPFFxuhDR1L6tSoc_BJECPebWKRXjBZCiFV4n3oknjhMstn64tZ_2W-5JsGY4Hc5n9yBXArwl93lqt7_RN5w6Cf0h4QyQ5v-65YGjQR0_FDW2QvzqY368QQMicAtaSqzs8KJZgnYb9c7d0zgdAZHzu6qMQvRL5hajrn1n91CbOpbISD08qNLyrdkt-bFTWhAI4vMQFh6WeZu0fM4lFd2NcRwr3XPksINHaQ-G_xBniIqbw0Ls1jF44-csFCur-kEgU8awapJzKnqDKgw",
       "e":"AQAB",
       "alg":"RS256",
       "kid":"2011-04-29"}
    ]})")));
      EXPECT_EQ(2ul, val.Keys.size());
      EXPECT_EQ("EC", *val.Keys[0].kty);
      EXPECT_EQ("P-256", *val.Keys[0].crv);
      EXPECT_EQ("MKBCTNIcKUSDii11ySs3526iDZ8AiTo7Tu6KPAqv7D4", *val.Keys[0].x);
      EXPECT_EQ("4Etl6SRW2YiLUrN5vfvVHuhp7x8PxltmWWlbbM4IFyM", *val.Keys[0].y);
      EXPECT_EQ("enc", *val.Keys[0].use);
      EXPECT_EQ("1", *val.Keys[0].kid);
      EXPECT_EQ("RS256", *val.Keys[1].alg);
      EXPECT_EQ(
          "0vx7agoebGcQSuuPiLJXZptN9nndrQmbXEps2aiAFbWhM78LhWx4cbbfAAtVT86zwu1RK7aPFFxuhDR1L6tSoc_"
          "BJECPebWKRXjBZCiFV4n3oknjhMstn64tZ_2W-5JsGY4Hc5n9yBXArwl93lqt7_RN5w6Cf0h4QyQ5v-65YGjQR0_"
          "FDW2QvzqY368QQMicAtaSqzs8KJZgnYb9c7d0zgdAZHzu6qMQvRL5hajrn1n91CbOpbISD08qNLyrdkt-"
          "bFTWhAI4vMQFh6WeZu0fM4lFd2NcRwr3XPksINHaQ-G_xBniIqbw0Ls1jF44-csFCur-kEgU8awapJzKnqDKgw",
          *val.Keys[1].n);
      EXPECT_EQ("AQAB", *val.Keys[1].e);
      EXPECT_EQ("2011-04-29", *val.Keys[1].kid);
    }

    {
      EXPECT_THROW(
          JsonWebKeySetSerializer::Deserialize(json::parse(R"({"xxx": [{"alg": "none"}]})")),
          std::runtime_error);
    }
    {
      EXPECT_THROW(
          JsonWebKeySetSerializer::Deserialize(json::parse(R"({"keys": {"alg": "none"}})")),
          std::runtime_error);
    }
    // cspell:enable
  }

  TEST(SerializationTests, TestSerializeAttestOpenEnclaveRequest)
  {
    {
      Models::_detail::AttestOpenEnclaveRequest request;
      request.Report = {1, 2, 3, 4};
      request.RunTimeData = AttestationData{{4, 5, 7, 8}, AttestationDataType::Binary};
      request.InitTimeData = AttestationData{{1, 2, 3, 4}, AttestationDataType::Json};
      request.DraftPolicyForAttestation = "Draft";
      request.Nonce = "My Nonce";

      std::string val = AttestOpenEnclaveRequestSerializer::Serialize(request);
      json parsedRequest = json::parse(val);
      EXPECT_TRUE(parsedRequest.is_object());
      EXPECT_TRUE(parsedRequest["report"].is_string());
      EXPECT_TRUE(parsedRequest["inittimeData"].is_object());
      EXPECT_TRUE(parsedRequest["runtimeData"].is_object());
      EXPECT_TRUE(parsedRequest["draftPolicyForAttestation"].is_string());
      EXPECT_TRUE(parsedRequest["nonce"].is_string());
    }
  }

  TEST(SerializationTests, TestDeserializeTokenResponse)
  {
    {
      auto val(AttestationServiceTokenResponseSerializer::Deserialize(
          json::parse(R"({"token": "ABCDEFG.123.456"} )")));
      EXPECT_EQ("ABCDEFG.123.456", val);
    }
    {
      EXPECT_THROW(
          AttestationServiceTokenResponseSerializer::Deserialize(
              json::parse(R"({"fred": "ABCDEFG.123.456"} )")),
          std::runtime_error);
    }
  }

  TEST(SerializationTests, TestDeserializeSignerToJson)
  {
    auto asymmetricKey = Cryptography::CreateRsaKey(2048);
    auto cert
        = Cryptography::CreateX509CertificateForPrivateKey(asymmetricKey, "CN=TestSubject, C=US");

    AttestationSigner signer{
        std::string{"ABCDEFG"}, std::vector<std::string>{cert->ExportAsBase64()}};

    std::string serializedSigner = AttestationSignerInternal::SerializeToJson(signer);
    auto jsonSigner(json::parse(serializedSigner));
    EXPECT_TRUE(jsonSigner["kid"].is_string());

    auto kidJson(jsonSigner["kid"]);
    auto kid=kidJson.get<std::string>();
    EXPECT_EQ(signer.KeyId.Value(), kid);
    EXPECT_TRUE(jsonSigner["x5c"].is_array());

    auto x5c = jsonSigner["x5c"].get<std::vector<json>>();
    EXPECT_TRUE(x5c[0].is_string());
    auto x5c0(x5c[0]);
    std::string x5cval(x5c0.get<std::string>());
    EXPECT_EQ(x5cval, (*signer.CertificateChain)[0]);
  }

  template <typename T> bool CompareNullable(Nullable<T> const& me, Nullable<T> const& them)
  {
    if (me.HasValue() != them.HasValue())
    {
      return false;
    }
    if (me)
    {
      return (*me == *them);
    }
    return true;
  }

  template <>
  bool CompareNullable(Nullable<Azure::DateTime> const& me, Nullable<Azure::DateTime> const& them)
  {
    if (me.HasValue() != them.HasValue())
    {
      return false;
    }
    if (me)
    {
      Azure::DateTime meTime(Azure::Core::_internal::PosixTimeConverter::PosixTimeToDateTime(
          PosixTimeConverter::DateTimeToPosixTime(*me)));
      Azure::DateTime themTime(Azure::Core::_internal::PosixTimeConverter::PosixTimeToDateTime(
          PosixTimeConverter::DateTimeToPosixTime(*them)));
      return (meTime == themTime);
    }
    return true;
  }

  struct TestObject
  {
    Azure::Nullable<std::string> Algorithm;
    Azure::Nullable<int> Integer;
    Azure::Nullable<Azure::DateTime> ExpiresAt;
    Azure::Nullable<Azure::DateTime> IssuedOn;
    Azure::Nullable<Azure::DateTime> NotBefore;
    Azure::Nullable<std::vector<int>> IntegerArray;
    Azure::Nullable<std::string> Issuer;

    bool operator==(TestObject const& that) const
    {
      if (!CompareNullable(Algorithm, that.Algorithm))
      {
        return false;
      }
      if (!CompareNullable(Integer, that.Integer))
      {
        return false;
      }
      if (!CompareNullable(IntegerArray, that.IntegerArray))
      {
        return false;
      }
      if (!CompareNullable(ExpiresAt, that.ExpiresAt))
      {
        return false;
      }
      if (!CompareNullable(IssuedOn, that.IssuedOn))
      {
        return false;
      }
      if (!CompareNullable(NotBefore, that.NotBefore))
      {
        return false;
      }
      if (!CompareNullable(Issuer, that.Issuer))
      {
        return false;
      }
      return true;
    }
    bool operator!=(TestObject const& that) { return !(*this == that); }
  };

  struct TestObjectSerializer
  {
    static TestObject Deserialize(json const& serialized)
    {
      TestObject returnValue;
      JsonOptional::SetIfExists(returnValue.Algorithm, serialized, "alg");
      JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
          returnValue.ExpiresAt,
          serialized,
          "exp",
          Azure::Core::_internal::PosixTimeConverter::PosixTimeToDateTime);
      JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
          returnValue.IssuedOn,
          serialized,
          "iat",
          Azure::Core::_internal::PosixTimeConverter::PosixTimeToDateTime);
      JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
          returnValue.NotBefore,
          serialized,
          "nbf",
          Azure::Core::_internal::PosixTimeConverter::PosixTimeToDateTime);
      JsonOptional::SetIfExists(returnValue.IntegerArray, serialized, "intArray");
      JsonOptional::SetIfExists(returnValue.Issuer, serialized, "iss");
      JsonOptional::SetIfExists(returnValue.Integer, serialized, "int");

      return returnValue;
    }

    static std::string Serialize(TestObject const& testObject)
    {
      json returnValue;

      JsonOptional::SetFromNullable(testObject.Algorithm, returnValue, "alg");
      JsonOptional::SetFromNullable(testObject.Integer, returnValue, "int");
      JsonOptional::SetFromNullable(testObject.IntegerArray, returnValue, "intArray");
      JsonOptional::SetFromNullable<Azure::DateTime, int64_t>(
          testObject.ExpiresAt,
          returnValue,
          "exp",
          Azure::Core::_internal::PosixTimeConverter::DateTimeToPosixTime);

      JsonOptional::SetFromNullable(testObject.Issuer, returnValue, "iss");
      JsonOptional::SetFromNullable<Azure::DateTime, int64_t>(
          testObject.IssuedOn,
          returnValue,
          "iat",
          Azure::Core::_internal::PosixTimeConverter::DateTimeToPosixTime);
      JsonOptional::SetFromNullable<Azure::DateTime, int64_t>(
          testObject.NotBefore,
          returnValue,
          "nbf",
          Azure::Core::_internal::PosixTimeConverter::DateTimeToPosixTime);
      return returnValue.dump();
    }
  };
  TEST(SerializationTests, SerializeDeserializeTestObject)
  {
    {
      TestObject testObject;
      testObject.Algorithm = "RSA";
      testObject.Integer = 314;
      testObject.ExpiresAt = std::chrono::system_clock::now();
      testObject.IssuedOn = std::chrono::system_clock::now();
      testObject.NotBefore = std::chrono::system_clock::now();
      testObject.IntegerArray = {1, 2, 99, 32};
      testObject.Issuer = "George";

      auto serializedObject = TestObjectSerializer::Serialize(testObject);

      auto targetObject = TestObjectSerializer::Deserialize(json::parse(serializedObject));
      EXPECT_EQ(testObject, targetObject);
    }
    {
      TestObject testObject;
      testObject.Algorithm = "RSA";
      testObject.ExpiresAt = std::chrono::system_clock::now();
      testObject.IssuedOn = std::chrono::system_clock::now();
      testObject.IntegerArray = {1, 2, 99, 32};

      auto serializedObject = TestObjectSerializer::Serialize(testObject);

      auto targetObject = TestObjectSerializer::Deserialize(json::parse(serializedObject));
      EXPECT_TRUE(testObject == targetObject);
    }
  }

  TEST(AttestationTokenTests, CreateUnsecuredTokenFromObject)
  {
    {
      TestObject testObject;
      testObject.Algorithm = "RSA";
      testObject.Integer = 314;
      testObject.ExpiresAt = std::chrono::system_clock::now() + std::chrono::seconds(30);
      testObject.IssuedOn = std::chrono::system_clock::now();
      testObject.NotBefore = std::chrono::system_clock::now();
      testObject.IntegerArray = {1, 2, 99, 32};
      testObject.Issuer = "George";

      auto testToken
          = AttestationTokenInternal<TestObject, TestObjectSerializer>::CreateToken(testObject);

      EXPECT_NO_THROW(testToken.ValidateToken({}));

      AttestationToken<TestObject> token = testToken;

      EXPECT_EQ(testObject, token.Body);

      EXPECT_TRUE(token.ExpiresOn);
      EXPECT_TRUE(token.IssuedOn);
      EXPECT_TRUE(token.NotBefore);
      EXPECT_TRUE(token.Issuer);
      EXPECT_EQ(*token.Issuer, "George");
      EXPECT_TRUE(token.Header.Algorithm);
      EXPECT_EQ("none", *token.Header.Algorithm);
    }
  }

  TEST(AttestationTokenTests, TestUnsecuredTokenValidation)
  {

    // Test expired tokens.
    {
      // Capture the current time, needed for future validation.
      auto now = std::chrono::system_clock::now() - std::chrono::seconds(30);

      TestObject testObject;

      testObject.Algorithm = "RSA";
      testObject.Integer = 314;
      testObject.IntegerArray = {1, 2, 99, 32};
      testObject.Issuer = "George";

      // This token was issued 40 seconds ago, and is valid for 15 seconds.
      testObject.ExpiresAt = now + std::chrono::seconds(15);
      testObject.IssuedOn = now;
      testObject.NotBefore = now;

      auto testToken
          = AttestationTokenInternal<TestObject, TestObjectSerializer>::CreateToken(testObject);

      // Simple valdation should throw an exception.
      EXPECT_THROW(testToken.ValidateToken({}), std::runtime_error);

      // Validate the token asking to ignore token validation.
      {
        AttestationTokenValidationOptions tokenOptions{};
        tokenOptions.ValidateToken = false;
        EXPECT_NO_THROW(testToken.ValidateToken(tokenOptions));
      }

      // Validate the token asking to ignore token expiration time.
      {
        AttestationTokenValidationOptions tokenOptions{};
        tokenOptions.ValidateExpirationTime = false;
        EXPECT_NO_THROW(testToken.ValidateToken(tokenOptions));
      }
    }

    // Test tokens which are not yet ready.
    {
      // Capture the current time, 30 seconds in the future.
      auto now = std::chrono::system_clock::now() + std::chrono::seconds(30);

      TestObject testObject;

      testObject.Algorithm = "RSA";
      testObject.Integer = 314;
      testObject.IntegerArray = {1, 2, 99, 32};
      testObject.Issuer = "George";

      // This token will be issued 30 seconds from now, and is valid for 15 seconds.
      testObject.ExpiresAt = now + std::chrono::seconds(15);
      testObject.IssuedOn = now;
      testObject.NotBefore = now;

      auto testToken
          = AttestationTokenInternal<TestObject, TestObjectSerializer>::CreateToken(testObject);

      // Simple valdation should throw an exception.
      EXPECT_THROW(testToken.ValidateToken({}), std::runtime_error);

      // Validate the token asking to ignore token validation.
      {
        AttestationTokenValidationOptions tokenOptions{};
        tokenOptions.ValidateToken = false;
        EXPECT_NO_THROW(testToken.ValidateToken(tokenOptions));
      }

      // Validate the token asking to ignore token expiration time.
      {
        AttestationTokenValidationOptions tokenOptions{};
        tokenOptions.ValidateNotBeforeTime = false;
        EXPECT_NO_THROW(testToken.ValidateToken(tokenOptions));
      }
    }
  }

  void CreateSecuredToken(
      std::unique_ptr<Cryptography::AsymmetricKey> const& key,
      std::unique_ptr<Cryptography::X509Certificate> const& cert)
  {
    TestObject testObject;
    testObject.Algorithm = "UnknownAlgorithm";
    testObject.Integer = 314;
    // Capture the current time, needed for future validation.
    auto now = std::chrono::system_clock::now();

    // This token was issued now, and is valid for 30 seconds.
    testObject.ExpiresAt = now + std::chrono::seconds(30);
    testObject.IssuedOn = now;
    testObject.NotBefore = now;
    testObject.IntegerArray = {1, 2, 99, 32};
    testObject.Issuer = "George";

    AttestationSigningKey signingKey{key->ExportPrivateKey(), cert->ExportAsPEM()};

    // Create a secured attestation token wrapped around the TestObject.
    auto testToken = AttestationTokenInternal<TestObject, TestObjectSerializer>::CreateToken(
        testObject, signingKey);

    // Validate this token - it should not throw.
    EXPECT_NO_THROW(testToken.ValidateToken({}));

    {
      AttestationTokenValidationOptions validationOptions;
      validationOptions.ValidateIssuer = true;
      validationOptions.ExpectedIssuer = "George";
      EXPECT_NO_THROW(testToken.ValidateToken(validationOptions));
    }

    AttestationToken<TestObject> token = testToken;
    EXPECT_EQ(testObject, token.Body);

    EXPECT_TRUE(token.ExpiresOn);
    EXPECT_TRUE(token.IssuedOn);
    EXPECT_TRUE(token.NotBefore);
    EXPECT_TRUE(token.Issuer);
    EXPECT_EQ(*token.Issuer, "George");
  }
  TEST(AttestationTokenTests, CreateSecuredTokenFromObject)
  {
    {
      // Create an RSA public/private key pair.
      auto asymmetricKey = Cryptography::CreateRsaKey(2048);
      auto cert
          = Cryptography::CreateX509CertificateForPrivateKey(asymmetricKey, "CN=TestSubject, C=US");
      CreateSecuredToken(asymmetricKey, cert);
    }

    {
      // Create an RSA public/private key pair.
      auto asymmetricKey = Cryptography::CreateEcdsaKey();
      auto cert
          = Cryptography::CreateX509CertificateForPrivateKey(asymmetricKey, "CN=TestSubject, C=US");
      CreateSecuredToken(asymmetricKey, cert);
    }
  }

  TEST(AttestationTokenTests, TestSecuredTokenValidation)
  {
    // Create an RSA public/private key pair. Use these for the subsequent tests.
    auto asymmetricKey = Cryptography::CreateRsaKey(2048);
    auto cert
        = Cryptography::CreateX509CertificateForPrivateKey(asymmetricKey, "CN=TestSubject, C=US");
    AttestationSigningKey signingKey{asymmetricKey->ExportPrivateKey(), cert->ExportAsPEM()};

    // Test expired tokens.
    {
      // Capture the current time, needed for future validation.
      auto now = std::chrono::system_clock::now() - std::chrono::seconds(30);

      TestObject testObject;

      testObject.Algorithm = "RSA";
      testObject.Integer = 314;
      testObject.IntegerArray = {1, 2, 99, 32};
      testObject.Issuer = "George";

      // This token was issued 40 seconds ago, and is valid for 15 seconds.
      testObject.ExpiresAt = now + std::chrono::seconds(15);
      testObject.IssuedOn = now;
      testObject.NotBefore = now;

      auto testToken = AttestationTokenInternal<TestObject, TestObjectSerializer>::CreateToken(
          testObject, signingKey);

      // Simple valdation should throw an exception.
      EXPECT_THROW(testToken.ValidateToken({}), std::runtime_error);

      // Validate the token asking to ignore token validation.
      {
        AttestationTokenValidationOptions tokenOptions{};
        tokenOptions.ValidateToken = false;
        EXPECT_NO_THROW(testToken.ValidateToken(tokenOptions));
      }

      // Validate the token asking to ignore token expiration time.
      {
        AttestationTokenValidationOptions tokenOptions{};
        tokenOptions.ValidateExpirationTime = false;
        EXPECT_NO_THROW(testToken.ValidateToken(tokenOptions));
      }
    }

    // Test tokens which are not yet ready.
    {
      // Capture the current time, 30 seconds in the future.
      auto now = std::chrono::system_clock::now() + std::chrono::seconds(30);

      TestObject testObject;

      testObject.Algorithm = "RSA";
      testObject.Integer = 314;
      testObject.IntegerArray = {1, 2, 99, 32};
      testObject.Issuer = "George";

      // This token will be issued 30 seconds from now, and is valid for 15 seconds.
      testObject.ExpiresAt = now + std::chrono::seconds(15);
      testObject.IssuedOn = now;
      testObject.NotBefore = now;

      auto testToken = AttestationTokenInternal<TestObject, TestObjectSerializer>::CreateToken(
          testObject, signingKey);

      // Simple valdation should throw an exception.
      EXPECT_THROW(testToken.ValidateToken({}), std::runtime_error);

      // Validate the token asking to ignore token validation.
      {
        AttestationTokenValidationOptions tokenOptions{};
        tokenOptions.ValidateToken = false;
        EXPECT_NO_THROW(testToken.ValidateToken(tokenOptions));
      }

      // Validate the token asking to ignore token expiration time.
      {
        AttestationTokenValidationOptions tokenOptions{};
        tokenOptions.ValidateNotBeforeTime = false;
        EXPECT_NO_THROW(testToken.ValidateToken(tokenOptions));
      }
    }

    // Test signature corruptions...
    {
      // Capture the current time.
      auto now = std::chrono::system_clock::now();

      TestObject testObject;

      testObject.Algorithm = "RSA";
      testObject.Integer = 314;
      testObject.IntegerArray = {1, 2, 99, 32};
      testObject.Issuer = "George";

      // This token will be issued 30 seconds from now, and is valid for 15 seconds.
      testObject.ExpiresAt = now + std::chrono::seconds(15);
      testObject.IssuedOn = now;
      testObject.NotBefore = now;

      auto goodToken = AttestationTokenInternal<TestObject, TestObjectSerializer>::CreateToken(
          testObject, signingKey);

      auto signedToken = static_cast<AttestationToken<TestObject>>(goodToken).RawToken;
      signedToken += "ABCDEFGH"; // Corrupt the signature on the signedToken.

      auto badToken = AttestationTokenInternal<TestObject, TestObjectSerializer>(signedToken);

      // Simple valdation should throw an exception - the signature of the token is invalid.
      EXPECT_THROW(badToken.ValidateToken({}), std::runtime_error);

      // Validate the token asking to ignore token validation.
      {
        AttestationTokenValidationOptions tokenOptions{};
        tokenOptions.ValidateToken = false;
        EXPECT_NO_THROW(badToken.ValidateToken(tokenOptions));
      }

      // Validate the token asking to ignore token signature validation.
      {
        AttestationTokenValidationOptions tokenOptions{};
        tokenOptions.ValidateSigner = false;
        EXPECT_NO_THROW(badToken.ValidateToken(tokenOptions));
      }
    }

    // Test incorrect issuer...
    {
      // Capture the current time.
      auto now = std::chrono::system_clock::now();

      TestObject testObject;

      testObject.Algorithm = "RSA";
      testObject.Integer = 314;
      testObject.IntegerArray = {1, 2, 99, 32};
      testObject.Issuer = "George";

      // This token will be issued 30 seconds from now, and is valid for 15 seconds.
      testObject.ExpiresAt = now + std::chrono::seconds(15);
      testObject.IssuedOn = now;
      testObject.NotBefore = now;

      auto testToken = AttestationTokenInternal<TestObject, TestObjectSerializer>::CreateToken(
          testObject, signingKey);

      AttestationTokenValidationOptions validationOptions;
      validationOptions.ValidateIssuer = true;
      validationOptions.ExpectedIssuer = "Fred";

      // Simple valdation should throw an exception - the signature of the token is invalid.
      EXPECT_THROW(testToken.ValidateToken(validationOptions), std::runtime_error);

      // Validate the token asking to ignore token signature validation.
      {
        AttestationTokenValidationOptions tokenOptions{};
        tokenOptions.ValidateIssuer = false;
        tokenOptions.ExpectedIssuer = "Fred";
        EXPECT_NO_THROW(testToken.ValidateToken(tokenOptions));
      }
    }

    // Test no issuer but issuer validation requested.
    {
      // Capture the current time.
      auto now = std::chrono::system_clock::now();

      TestObject testObject;

      testObject.Algorithm = "RSA";
      testObject.Integer = 314;
      testObject.IntegerArray = {1, 2, 99, 32};

      // This token will be issued 30 seconds from now, and is valid for 15 seconds.
      testObject.ExpiresAt = now + std::chrono::seconds(15);
      testObject.IssuedOn = now;
      testObject.NotBefore = now;

      auto testToken = AttestationTokenInternal<TestObject, TestObjectSerializer>::CreateToken(
          testObject, signingKey);

      AttestationTokenValidationOptions validationOptions;
      validationOptions.ValidateIssuer = true;
      validationOptions.ExpectedIssuer = "Fred";

      // Simple valdation should throw an exception - the signature of the token is invalid.
      EXPECT_THROW(testToken.ValidateToken(validationOptions), std::runtime_error);
    }
  }
}}}} // namespace Azure::Security::Attestation::Test

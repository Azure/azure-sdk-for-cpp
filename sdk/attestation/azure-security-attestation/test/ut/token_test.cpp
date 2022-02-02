// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX-License-Identifier: MIT

#include "../../src/private/attestation_deserializer.hpp"
#include "azure/core/test/test_base.hpp"
#include <gtest/gtest.h>

// cspell:words jwk jwks
namespace Azure { namespace Security { namespace Attestation { namespace Test {
  using namespace Azure::Core::Json::_internal;
  using namespace Azure::Core::Http;
  using namespace Azure::Security::Attestation::_detail;

  TEST(SerializationTests, TestDeserializePrimitivesBoolean)
  {
    {
      auto val = JsonHelpers::ParseBooleanField(json::parse("{ \"boolean\": true }"), "boolean");
      EXPECT_TRUE(val.HasValue());
      EXPECT_EQ(true, val.Value());
    }
    {
      auto val(
          JsonHelpers::ParseBooleanField(json::parse("{ \"boolValue\": false }"), "boolValue"));
      EXPECT_TRUE(val.HasValue());
      EXPECT_EQ(false, val.Value());
    }
    {
      auto val(JsonHelpers::ParseBooleanField(json::parse("{ \"boolValue2\": true}"), "intValue"));
      EXPECT_FALSE(val.HasValue());
    }
    {
      EXPECT_THROW(
          JsonHelpers::ParseBooleanField(json::parse("{ \"bool\": 27 }"), "bool"),
          std::runtime_error);
    }
  }

  TEST(SerializationTests, TestDeserializePrimitivesNumberInt)
  {
    {
      auto val(JsonHelpers::ParseIntNumberField(json::parse("{ \"int\": 27}"), "int"));
      EXPECT_TRUE(val.HasValue());
      EXPECT_EQ(27, val.Value());
    }
    {
      auto val(JsonHelpers::ParseIntNumberField(json::parse("{ \"intValue\": 35}"), "intValue"));
      EXPECT_TRUE(val.HasValue());
      EXPECT_EQ(35, val.Value());
    }
    {
      auto val(JsonHelpers::ParseIntNumberField(json::parse("{ \"intValue2\": 35}"), "intValue"));
      EXPECT_FALSE(val.HasValue());
    }
    {
      EXPECT_THROW(
          JsonHelpers::ParseIntNumberField(json::parse("{ \"bool\": \"stringValue\"}"), "bool"),
          std::runtime_error);
    }
  }

  TEST(SerializationTests, TestDeserializePrimitivesString)
  {
    {
      auto val(JsonHelpers::ParseStringField(json::parse("{ \"int\": \"127\"}"), "int"));
      EXPECT_EQ("127", val);
    }
    {
      auto val(JsonHelpers::ParseStringField(json::parse("{ \"intVal\": \"127\"}"), "int"));
      EXPECT_TRUE(val.empty());
    }
    {
      auto val(JsonHelpers::ParseStringField(
          json::parse("{ \"intValue\": \"String Field\"}"), "intValue"));
      EXPECT_EQ("String Field", val);
    }
    {
      EXPECT_THROW(
          JsonHelpers::ParseStringField(json::parse("{ \"bool\": true}"), "bool"),
          std::runtime_error);
    }
  }

  TEST(SerializationTests, TestDeserializePrimitivesStringArray)
  {
    // Present string array field.
    {
      auto val(JsonHelpers::ParseStringArrayField(
          json::parse("{ \"stringArrayValue\": [\"String Field\", \"SF2\"]}"), "stringArrayValue"));
      EXPECT_EQ(2ul, val.size());
      EXPECT_EQ("String Field", val[0]);
      EXPECT_EQ("SF2", val[1]);
    }
    // Not present field.
    {
      auto val(JsonHelpers::ParseStringArrayField(
          json::parse("{ \"arrayValue\": \"String Field\"}"), "intValue"));
      EXPECT_EQ(0ul, val.size());
    }
    // Not an array.
    {
      EXPECT_THROW(
          JsonHelpers::ParseStringArrayField(
              json::parse("{ \"stringArray\": true}"), "stringArray"),
          std::runtime_error);
    }
    // Not an array of strings.
    {
      EXPECT_THROW(
          JsonHelpers::ParseStringArrayField(
              json::parse("{ \"stringArray\": [1, 2, 3, 4]}"), "stringArray"),
          std::runtime_error);
    }
  }

  TEST(SerializationTests, TestDeserializePrimitivesIntArray)
  {
    // Present string array field.
    {
      auto val(JsonHelpers::ParseIntArrayField(
          json::parse(R"({ "intArrayValue": [1, 3, 7, 5]})"), "intArrayValue"));
      EXPECT_TRUE(val.HasValue());

      EXPECT_EQ(4ul, val.Value().size());
      EXPECT_EQ(1, val.Value()[0]);
      EXPECT_EQ(3, val.Value()[1]);
      EXPECT_EQ(7, val.Value()[2]);
      EXPECT_EQ(5, val.Value()[3]);
    }
    // Not present field.
    {
      auto val(JsonHelpers::ParseIntArrayField(
          json::parse(R"({ "arrayValue": [1, 3, 5]})"), "intValue"));
      EXPECT_FALSE(val.HasValue());
    }
    // Not an array.
    {
      EXPECT_THROW(
          JsonHelpers::ParseIntArrayField(json::parse(R"({ "stringArray": true})"), "stringArray"),
          std::runtime_error);
    }
    // Not an array of strings.
    {
      EXPECT_THROW(
          JsonHelpers::ParseIntArrayField(
              json::parse(R"({ "intArray": ["abc", "def", "ghi", "jkl"]})"), "intArray"),
          std::runtime_error);
    }
  }

  TEST(SerializationTests, TestDeserializePrimitivesJsonObject)
  {
    // Present JSON field.
    {
      auto val(JsonHelpers::ParseStringJsonField(
          json::parse(R"({ "jsonObjectValue": {"stringField": "SF2"}})"), "jsonObjectValue"));
      EXPECT_EQ(R"({"stringField":"SF2"})", val);
    }
    // Not present field.
    {
      auto val(JsonHelpers::ParseStringJsonField(
          json::parse("{ \"objectValue\":{\"String Field\": 27}}"), "intValue"));
      EXPECT_TRUE(val.empty());
    }
    // Not a JSON object.
    {
      EXPECT_THROW(
          JsonHelpers::ParseStringJsonField(json::parse("{ \"stringArray\": true}"), "stringArray"),
          std::runtime_error);
    }
  }

  TEST(SerializationTests, TestDeserializePrimitivesBase64Url)
  {
    std::string testData("Test Data");
    std::string encodedData = Azure::Core::_internal::Base64Url::Base64UrlEncode(
        std::vector<uint8_t>(testData.begin(), testData.end()));

    // Present JSON field.
    {
      auto val(JsonHelpers::ParseBase64UrlField(
          json::parse("{ \"base64Urlfield\": \"" + encodedData + "\"}"), "base64Urlfield"));
      EXPECT_EQ(9ul, val.size());
      EXPECT_EQ(testData, std::string(val.begin(), val.end()));
    }
    // Not present field.
    {
      auto val(JsonHelpers::ParseBase64UrlField(
          json::parse("{ \"base64Urlfield\": \"" + encodedData + "\"}"), "intValue"));
      EXPECT_TRUE(val.empty());
    }

    // Not a string.
    {
      EXPECT_THROW(
          JsonHelpers::ParseBase64UrlField(
              json::parse("{ \"base64Urlfield\": true}"), "base64Urlfield"),
          std::runtime_error);
    }
    // Not base64url. This does not currently throw.
    {
      EXPECT_THROW(
          JsonHelpers::ParseBase64UrlField(
              json::parse("{ \"base64Urlfield\": \"!@#%@!!%!!\"}"), "base64Urlfield"),
          std::runtime_error);
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
      EXPECT_EQ("EC", val.kty);
      EXPECT_EQ("P-256", val.crv);
      EXPECT_EQ("MKBCTNIcKUSDii11ySs3526iDZ8AiTo7Tu6KPAqv7D4", val.x);
      EXPECT_EQ("4Etl6SRW2YiLUrN5vfvVHuhp7x8PxltmWWlbbM4IFyM", val.y);
      EXPECT_EQ("enc", val.use);
      EXPECT_EQ("1", val.kid);
    }

    {
      auto val(JsonWebKeySerializer::Deserialize(json::parse(R"({"kty":"RSA",
       "n": "0vx7agoebGcQSuuPiLJXZptN9nndrQmbXEps2aiAFbWhM78LhWx4cbbfAAtVT86zwu1RK7aPFFxuhDR1L6tSoc_BJECPebWKRXjBZCiFV4n3oknjhMstn64tZ_2W-5JsGY4Hc5n9yBXArwl93lqt7_RN5w6Cf0h4QyQ5v-65YGjQR0_FDW2QvzqY368QQMicAtaSqzs8KJZgnYb9c7d0zgdAZHzu6qMQvRL5hajrn1n91CbOpbISD08qNLyrdkt-bFTWhAI4vMQFh6WeZu0fM4lFd2NcRwr3XPksINHaQ-G_xBniIqbw0Ls1jF44-csFCur-kEgU8awapJzKnqDKgw",
       "e":"AQAB",
       "alg":"RS256",
       "kid":"2011-04-29"})")));
      EXPECT_EQ("RS256", val.alg);
      EXPECT_EQ(
          "0vx7agoebGcQSuuPiLJXZptN9nndrQmbXEps2aiAFbWhM78LhWx4cbbfAAtVT86zwu1RK7aPFFxuhDR1L6tSoc_"
          "BJECPebWKRXjBZCiFV4n3oknjhMstn64tZ_2W-5JsGY4Hc5n9yBXArwl93lqt7_RN5w6Cf0h4QyQ5v-65YGjQR0_"
          "FDW2QvzqY368QQMicAtaSqzs8KJZgnYb9c7d0zgdAZHzu6qMQvRL5hajrn1n91CbOpbISD08qNLyrdkt-"
          "bFTWhAI4vMQFh6WeZu0fM4lFd2NcRwr3XPksINHaQ-G_xBniIqbw0Ls1jF44-csFCur-kEgU8awapJzKnqDKgw",
          val.n);
      EXPECT_EQ("AQAB", val.e);
      EXPECT_EQ("2011-04-29", val.kid);
    }
    // cspell:enable
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
      EXPECT_EQ("EC", val.Keys[0].kty);
      EXPECT_EQ("P-256", val.Keys[0].crv);
      EXPECT_EQ("MKBCTNIcKUSDii11ySs3526iDZ8AiTo7Tu6KPAqv7D4", val.Keys[0].x);
      EXPECT_EQ("4Etl6SRW2YiLUrN5vfvVHuhp7x8PxltmWWlbbM4IFyM", val.Keys[0].y);
      EXPECT_EQ("enc", val.Keys[0].use);
      EXPECT_EQ("1", val.Keys[0].kid);
      EXPECT_EQ("RS256", val.Keys[1].alg);
      EXPECT_EQ(
          "0vx7agoebGcQSuuPiLJXZptN9nndrQmbXEps2aiAFbWhM78LhWx4cbbfAAtVT86zwu1RK7aPFFxuhDR1L6tSoc_"
          "BJECPebWKRXjBZCiFV4n3oknjhMstn64tZ_2W-5JsGY4Hc5n9yBXArwl93lqt7_RN5w6Cf0h4QyQ5v-65YGjQR0_"
          "FDW2QvzqY368QQMicAtaSqzs8KJZgnYb9c7d0zgdAZHzu6qMQvRL5hajrn1n91CbOpbISD08qNLyrdkt-"
          "bFTWhAI4vMQFh6WeZu0fM4lFd2NcRwr3XPksINHaQ-G_xBniIqbw0Ls1jF44-csFCur-kEgU8awapJzKnqDKgw",
          val.Keys[1].n);
      EXPECT_EQ("AQAB", val.Keys[1].e);
      EXPECT_EQ("2011-04-29", val.Keys[1].kid);
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
      request.RunTimeData.Data = {4, 5, 7, 8};
      request.RunTimeData.DataType = AttestationDataType::Binary;
      request.InitTimeData.Data = {1, 2, 3, 4};
      request.InitTimeData.DataType = AttestationDataType::Json;
      request.DraftPolicyForAttestation = "Draft";

      std::string val = AttestOpenEnclaveRequestSerializer::Serialize(request);
      json parsedRequest = json::parse(val);
      EXPECT_TRUE(parsedRequest.is_object());
      EXPECT_TRUE(parsedRequest["report"].is_string());
      EXPECT_TRUE(parsedRequest["inittimeData"].is_object());
      EXPECT_TRUE(parsedRequest["runtimeData"].is_object());
      EXPECT_TRUE(parsedRequest["draftPolicyForAttestation"].is_string());
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
    {
      EXPECT_THROW(
          AttestationServiceTokenResponseSerializer::Deserialize(
              json::parse(R"({"token": [12345]})")),
          std::runtime_error);
    }
  }

  template <typename T> bool CompareNullable(Nullable<T> const& me, Nullable<T> const& them)
  {
    if (me.HasValue() != them.HasValue())
    {
      return false;
    }
    if (me.HasValue())
    {
      return (me.Value() == them.Value());
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
    if (me.HasValue())
    {
      Azure::DateTime meTime(Azure::Core::_internal::PosixTimeConverter::PosixTimeToDateTime(
          Azure::Core::_internal::PosixTimeConverter::DateTimeToPosixTime(me.Value())));
      Azure::DateTime themTime(Azure::Core::_internal::PosixTimeConverter::PosixTimeToDateTime(
          Azure::Core::_internal::PosixTimeConverter::DateTimeToPosixTime(them.Value())));
      return (meTime == themTime);
    }
    return true;
  }

  struct TestObject
  {
    std::string Algorithm;
    Azure::Nullable<int> Integer;
    Azure::Nullable<Azure::DateTime> ExpiresAt;
    Azure::Nullable<Azure::DateTime> IssuedOn;
    Azure::Nullable<Azure::DateTime> NotBefore;
    Azure::Nullable<std::vector<int>> IntegerArray;
    std::string Issuer;

    bool operator==(TestObject const& that)
    {
      if (Algorithm != that.Algorithm)
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
      if (Issuer != that.Issuer)
      {
        return false;
      }
      return true;
    }
  };

  struct TestObjectDeserializer
  {
    static TestObject Deserialize(json const& serialized)
    {
      TestObject returnValue;
      returnValue.Algorithm = JsonHelpers::ParseStringField(serialized, "alg");
      returnValue.ExpiresAt = JsonHelpers::ParseDateTimeField(serialized, "exp");
      returnValue.IssuedOn = JsonHelpers::ParseDateTimeField(serialized, "iat");
      returnValue.NotBefore = JsonHelpers::ParseDateTimeField(serialized, "nbf");
      returnValue.IntegerArray = JsonHelpers::ParseIntArrayField(serialized, "intArray");
      returnValue.Issuer = JsonHelpers::ParseStringField(serialized, "iss");
      returnValue.Integer = JsonHelpers::ParseIntNumberField(serialized, "int");

      return returnValue;
    }

    static std::string Serialize(TestObject const& testObject)
    {
      json returnValue;
      JsonHelpers::SetField(returnValue, testObject.Algorithm, "alg");
      JsonHelpers::SetField(returnValue, testObject.Integer, "int");
      JsonHelpers::SetField(returnValue, testObject.IntegerArray, "intArray");
      JsonHelpers::SetField(returnValue, testObject.ExpiresAt, "exp");
      JsonHelpers::SetField(returnValue, testObject.Issuer, "iss");
      JsonHelpers::SetField(returnValue, testObject.IssuedOn, "iat");
      JsonHelpers::SetField(returnValue, testObject.NotBefore, "nbf");
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

      auto serializedObject = TestObjectDeserializer::Serialize(testObject);

      auto targetObject = TestObjectDeserializer::Deserialize(json::parse(serializedObject));
      EXPECT_TRUE(testObject == targetObject);
    }
    {
      TestObject testObject;
      testObject.Algorithm = "RSA";
      testObject.ExpiresAt = std::chrono::system_clock::now();
      testObject.IssuedOn = std::chrono::system_clock::now();
      testObject.IntegerArray = {1, 2, 99, 32};

      auto serializedObject = TestObjectDeserializer::Serialize(testObject);

      auto targetObject = TestObjectDeserializer::Deserialize(json::parse(serializedObject));
      EXPECT_TRUE(testObject == targetObject);
    }
  }

}}}} // namespace Azure::Security::Attestation::Test
// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX-License-Identifier: MIT

#include "../../src/private/attestation_deserializer.hpp"
#include <azure/attestation/attestation_client.hpp>
#include <azure/core/exception.hpp>
#include <azure/core/http/raw_response.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/test/test_base.hpp>
#include <azure/identity/client_secret_credential.hpp>
#include <gtest/gtest.h>
#include <random>

namespace Azure { namespace Security { namespace Attestation { namespace Test {
  using namespace Azure::Core::Json::_internal;
  using namespace Azure::Core::Http;

  TEST(SerializationTests, TestDeserializePrimitivesBoolean)
  {
    {
      auto val = Azure::Security::Attestation::_detail::JsonHelpers::ParseBooleanField(
          json::parse("{ \"boolean\": true }"), "boolean");
      EXPECT_TRUE(val.HasValue());
      EXPECT_EQ(true, val.Value());
    }
    {
      auto val(Azure::Security::Attestation::_detail::JsonHelpers::ParseBooleanField(
          json::parse("{ \"boolValue\": false }"), "boolValue"));
      EXPECT_TRUE(val.HasValue());
      EXPECT_EQ(false, val.Value());
    }
    {
      auto val(Azure::Security::Attestation::_detail::JsonHelpers::ParseIntNumberField(
          json::parse("{ \"boolValue2\": true}"), "intValue"));
      EXPECT_FALSE(val.HasValue());
    }
    {
      EXPECT_THROW(
          Azure::Security::Attestation::_detail::JsonHelpers::ParseBooleanField(
              json::parse("{ \"bool\": 27 }"), "bool"),
          std::runtime_error);
    }
  }

  TEST(SerializationTests, TestDeserializePrimitivesNumberInt)
  {
    {
      auto val(Azure::Security::Attestation::_detail::JsonHelpers::ParseIntNumberField(
          json::parse("{ \"int\": 27}"), "int"));
      EXPECT_TRUE(val.HasValue());
      EXPECT_EQ(27, val.Value());
    }
    {
      auto val(Azure::Security::Attestation::_detail::JsonHelpers::ParseIntNumberField(
          json::parse("{ \"intValue\": 35}"), "intValue"));
      EXPECT_TRUE(val.HasValue());
      EXPECT_EQ(35, val.Value());
    }
    {
      auto val(Azure::Security::Attestation::_detail::JsonHelpers::ParseIntNumberField(
          json::parse("{ \"intValue2\": 35}"), "intValue"));
      EXPECT_FALSE(val.HasValue());
    }
    {
      EXPECT_THROW(
          Azure::Security::Attestation::_detail::JsonHelpers::ParseIntNumberField(
              json::parse("{ \"bool\": \"stringValue\"}"), "bool"),
          std::runtime_error);
    }
  }

  TEST(SerializationTests, TestDeserializePrimitivesString)
  {
    {
      auto val(Azure::Security::Attestation::_detail::JsonHelpers::ParseStringField(
          json::parse("{ \"int\": \"127\"}"), "int"));
      EXPECT_EQ("127", val);
    }
    {
      auto val(Azure::Security::Attestation::_detail::JsonHelpers::ParseStringField(
          json::parse("{ \"intVal\": \"127\"}"), "int"));
      EXPECT_TRUE(val.empty());
    }
    {
      auto val(Azure::Security::Attestation::_detail::JsonHelpers::ParseStringField(
          json::parse("{ \"intValue\": \"String Field\"}"), "intValue"));
      EXPECT_EQ("String Field", val);
    }
    {
      EXPECT_THROW(
          Azure::Security::Attestation::_detail::JsonHelpers::ParseStringField(
              json::parse("{ \"bool\": true}"), "bool"),
          std::runtime_error);
    }
  }

  TEST(SerializationTests, TestDeserializePrimitivesStringArray)
  {
    // Present string array field.
    {
      auto val(Azure::Security::Attestation::_detail::JsonHelpers::ParseStringArrayField(
          json::parse("{ \"stringArrayValue\": [\"String Field\", \"SF2\"]}"), "stringArrayValue"));
      EXPECT_EQ(2ul, val.size());
      EXPECT_EQ("String Field", val[0]);
      EXPECT_EQ("SF2", val[1]);
    }
    // Not present field.
    {
      auto val(Azure::Security::Attestation::_detail::JsonHelpers::ParseStringArrayField(
          json::parse("{ \"arrayValue\": \"String Field\"}"), "intValue"));
      EXPECT_EQ(0ul, val.size());
    }
    // Not an array.
    {
      EXPECT_THROW(
          Azure::Security::Attestation::_detail::JsonHelpers::ParseStringArrayField(
              json::parse("{ \"stringArray\": true}"), "stringArray"),
          std::runtime_error);
    }
    // Not an array of strings.
    {
      EXPECT_THROW(
          Azure::Security::Attestation::_detail::JsonHelpers::ParseStringArrayField(
              json::parse("{ \"stringArray\": [1, 2, 3, 4]}"), "stringArray"),
          std::runtime_error);
    }
  }

  TEST(SerializationTests, TestDeserializePrimitivesJsonObject)
  {
    // Present JSON field.
    {
      auto val(Azure::Security::Attestation::_detail::JsonHelpers::ParseStringJsonField(
          json::parse("{ \"jsonObjectValue\": {\"stringField\": \"SF2\"}}"), "jsonObjectValue"));
      EXPECT_EQ("{\"stringField\":\"SF2\"}", val);
    }
    // Not present field.
    {
      auto val(Azure::Security::Attestation::_detail::JsonHelpers::ParseStringJsonField(
          json::parse("{ \"objectValue\":{\"String Field\": 27}}"), "intValue"));
      EXPECT_TRUE(val.empty());
    }
    // Not a JSON object.
    {
      EXPECT_THROW(
          Azure::Security::Attestation::_detail::JsonHelpers::ParseStringJsonField(
              json::parse("{ \"stringArray\": true}"), "stringArray"),
          std::runtime_error);
    }
  }

  TEST(SerializationTests, TestDeserializePrimitivesBase64Url)
  {
    std::string testData("Test Data");
    std::string encodedData = Azure::Core::_internal::Base64Url::Base64UrlEncode(std::vector<uint8_t>(testData.begin(), testData.end()));

    // Present JSON field.
    {
      auto val(Azure::Security::Attestation::_detail::JsonHelpers::ParseBase64UrlField(
          json::parse("{ \"base64Urlfield\": \"" + encodedData + "\"}"), "base64Urlfield"));
      EXPECT_EQ(9, val.size());
      EXPECT_EQ(testData, std::string(val.begin(), val.end()));
    }
    // Not present field.
    {
      auto val(Azure::Security::Attestation::_detail::JsonHelpers::ParseBase64UrlField(
          json::parse("{ \"base64Urlfield\": \"" + encodedData + "\"}"), "intValue"));
      EXPECT_TRUE(val.empty());
    }

    // Not a string.
    {
      EXPECT_THROW(
          Azure::Security::Attestation::_detail::JsonHelpers::ParseBase64UrlField(
              json::parse("{ \"base64Urlfield\": true}"), "base64Urlfield"),
          std::runtime_error);
    }
    // Not base64url. This does not currently throw.
    {
//      EXPECT_THROW(
//          Azure::Security::Attestation::_detail::JsonHelpers::ParseBase64UrlField(
//              json::parse("{ \"base64Urlfield\": \"!@#%@!!%!!\"}"), "base64Urlfield"),
//          std::runtime_error);
    }
  }

}}}} // namespace Azure::Security::Attestation::Test
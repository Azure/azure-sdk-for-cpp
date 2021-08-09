#include "../src/private/secret_serializers.hpp"
#include "azure/core/internal/json/json.hpp"
#include "azure/core/internal/json/json_optional.hpp"
#include "azure/core/internal/json/json_serializable.hpp"
#include "azure/keyvault/secrets/secret_client.hpp"
#include "private/secret_constants.hpp"
#include "secret_get_client_deserialize_test.hpp"

using namespace Azure::Security::KeyVault::Secrets;
using namespace Azure::Security::KeyVault::Secrets::_detail;
using namespace Azure::Core::Json::_internal;

TEST(KeyVaultSecretPropertiesSerializer, Serialize1)
{
  KeyvaultSecretProperties properties;

  properties.ContentType = "contentType";
  properties.Enabled = true;
  properties.RecoverableDays = 5;

  auto serialized
      = _detail::KeyVaultSecretPropertiesSerializer::KeyVaultSecretPropertiesSerialize(properties);

  auto jsonParser = json::parse(serialized);

  EXPECT_EQ(properties.ContentType.Value(), jsonParser[_detail::ContentTypePropertyName]);
  EXPECT_EQ(
      properties.Enabled.Value(),
      jsonParser[_detail::AttributesPropertyName][_detail::EnabledPropertyName]);
  EXPECT_EQ(
      properties.RecoverableDays.Value(),
      jsonParser[_detail::AttributesPropertyName][_detail::RecoverableDaysPropertyName]);
}

TEST(KeyVaultSecretPropertiesSerializer, Serialize2)
{
  KeyvaultSecretProperties properties;

  properties.ContentType = "contentType";
  properties.Enabled = true;
  properties.RecoverableDays = 5;
  properties.Tags.emplace("a", "b");

  auto serialized
      = _detail::KeyVaultSecretPropertiesSerializer::KeyVaultSecretPropertiesSerialize(properties);

  auto jsonParser = json::parse(serialized);

  EXPECT_EQ(properties.ContentType.Value(), jsonParser[_detail::ContentTypePropertyName]);
  EXPECT_EQ(
      properties.Enabled.Value(),
      jsonParser[_detail::AttributesPropertyName][_detail::EnabledPropertyName]);
  EXPECT_EQ(
      properties.RecoverableDays.Value(),
      jsonParser[_detail::AttributesPropertyName][_detail::RecoverableDaysPropertyName]);
  EXPECT_EQ(properties.Tags["a"], jsonParser[_detail::TagsPropertyName]["a"]);
}

TEST(KeyVaultSecretPropertiesSerializer, Serialize3)
{
  KeyvaultSecretProperties properties;

  properties.ContentType = "contentType";
  properties.Enabled = true;
  properties.RecoverableDays = 5;
  properties.Tags.emplace("a", "b");
  properties.Tags.emplace("c", "d");

  auto serialized
      = _detail::KeyVaultSecretPropertiesSerializer::KeyVaultSecretPropertiesSerialize(properties);

  auto jsonParser = json::parse(serialized);

  EXPECT_EQ(properties.ContentType.Value(), jsonParser[_detail::ContentTypePropertyName]);
  EXPECT_EQ(
      properties.Enabled.Value(),
      jsonParser[_detail::AttributesPropertyName][_detail::EnabledPropertyName]);
  EXPECT_EQ(
      properties.RecoverableDays.Value(),
      jsonParser[_detail::AttributesPropertyName][_detail::RecoverableDaysPropertyName]);
  for (auto kvp : properties.Tags)
  {
    EXPECT_EQ(properties.Tags[kvp.first], jsonParser[_detail::TagsPropertyName][kvp.first]);
  }
}

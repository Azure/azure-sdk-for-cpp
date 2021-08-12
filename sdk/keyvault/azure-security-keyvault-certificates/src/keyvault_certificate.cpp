// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_optional.hpp>
#include <azure/core/internal/json/json_serializable.hpp>
#include <azure/core/url.hpp>

#include "azure/keyvault/certificates/certificate_client_models.hpp"
//#include "private/key_constants.hpp"
#include "private/certificate_serializers.hpp"

using namespace Azure::Security::KeyVault::Certificates;
using namespace Azure::Core::Json::_internal;
using Azure::Core::_internal::PosixTimeConverter;

KeyVaultCertificateWithPolicy
_detail::KeyVaultCertificateSerializer::KeyVaultCertificateDeserialize(
    std::string const& name,
    Azure::Core::Http::RawResponse const& rawResponse)
{
  (void)name;
  (void)rawResponse;
  KeyVaultCertificateWithPolicy certificate;
  //   KeyVaultCertificateWithPolicy certificate(name);
  //   auto const& body = rawResponse.GetBody();
  //   auto jsonParser = json::parse(body);
  //  using Azure::Core::_internal::PosixTimeConverter;

  //   // Deserialize jwk
  //   _detail::JsonWebKeySerializer::JsonWebDeserialize(key.Key, jsonParser);

  //   // Parse URL for the vaultUri, keyVersion
  //   _detail::KeyVaultKeySerializer::ParseKeyUrl(key.Properties, key.Key.Id);

  //   // "Attributes"
  //   if (jsonParser.contains(_detail::AttributesPropertyName))
  //   {
  //     auto attributes = jsonParser[_detail::AttributesPropertyName];

  //     JsonOptional::SetIfExists(key.Properties.Enabled, attributes,
  //     _detail::EnabledPropertyName);

  //     JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
  //         key.Properties.NotBefore,
  //         attributes,
  //         _detail::NbfPropertyName,
  //         PosixTimeConverter::PosixTimeToDateTime);
  //     JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
  //         key.Properties.ExpiresOn,
  //         attributes,
  //         _detail::ExpPropertyName,
  //         PosixTimeConverter::PosixTimeToDateTime);
  //     JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
  //         key.Properties.CreatedOn,
  //         attributes,
  //         _detail::CreatedPropertyName,
  //         PosixTimeConverter::PosixTimeToDateTime);
  //     JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
  //         key.Properties.UpdatedOn,
  //         attributes,
  //         _detail::UpdatedPropertyName,
  //         PosixTimeConverter::PosixTimeToDateTime);
  //   }

  //   // "Tags"
  //   if (jsonParser.contains(_detail::TagsPropertyName))
  //   {
  //     auto const& tags = jsonParser[_detail::TagsPropertyName];
  //     {
  //       for (auto tag = tags.begin(); tag != tags.end(); ++tag)
  //       {
  //         key.Properties.Tags.emplace(tag.key(), tag.value().get<std::string>());
  //       }
  //     }
  //   }

  //   // managed
  //   if (jsonParser.contains(_detail::ManagedPropertyName))
  //   {
  //     key.Properties.Managed = jsonParser[_detail::ManagedPropertyName].get<bool>();
  //   }

  return certificate;
}

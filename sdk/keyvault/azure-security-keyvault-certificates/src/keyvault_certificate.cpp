// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/base64.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_optional.hpp>
#include <azure/core/internal/json/json_serializable.hpp>
#include <azure/core/url.hpp>

#include "azure/keyvault/certificates/certificate_client_models.hpp"
#include "private/certificate_constants.hpp"
#include "private/certificate_serializers.hpp"

using namespace Azure::Security::KeyVault::Certificates;
using namespace Azure::Core::Json::_internal;
using namespace Azure::Core::_internal;

using Azure::Core::_internal::PosixTimeConverter;

KeyVaultCertificateWithPolicy
_detail::KeyVaultCertificateSerializer::KeyVaultCertificateDeserialize(
    std::string const& name,
    Azure::Core::Http::RawResponse const& rawResponse)
{
  CertificateProperties properties(name);
  auto const& body = rawResponse.GetBody();
  auto jsonResponse = json::parse(body);
  using Azure::Core::_internal::PosixTimeConverter;

  // Parse URL for the name, vaultUrl and version
  _detail::KeyVaultCertificateSerializer::ParseKeyUrl(
      properties, jsonResponse[IdName].get<std::string>());

  // x5t
  properties.X509Thumbprint = Base64Url::Base64UrlDecode(jsonResponse[X5tName].get<std::string>());

  // "Tags"
  if (jsonResponse.contains(TagsName))
  {
    properties.Tags = jsonResponse[TagsName].get<std::unordered_map<std::string, std::string>>();
  }

  // "Attributes"
  if (jsonResponse.contains(AttributesPropertyName))
  {
    auto attributes = jsonResponse[AttributesPropertyName];

    JsonOptional::SetIfExists(properties.Enabled, attributes, EnabledPropertyName);

    JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
        properties.NotBefore, attributes, NbfPropertyName, PosixTimeConverter::PosixTimeToDateTime);
    JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
        properties.ExpiresOn, attributes, ExpPropertyName, PosixTimeConverter::PosixTimeToDateTime);
    JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
        properties.CreatedOn,
        attributes,
        CreatedPropertyName,
        PosixTimeConverter::PosixTimeToDateTime);
    JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
        properties.UpdatedOn,
        attributes,
        UpdatedPropertyName,
        PosixTimeConverter::PosixTimeToDateTime);
    JsonOptional::SetIfExists(properties.RecoveryLevel, attributes, RecoveryLevelPropertyName);
    JsonOptional::SetIfExists(properties.RecoverableDays, attributes, RecoverableDaysPropertyName);
  }

  KeyVaultCertificateWithPolicy certificate(std::move(properties));

  // kid
  certificate.KeyId = jsonResponse[KidPropertyName].get<std::string>();
  // sid
  certificate.SecretId = jsonResponse[SidPropertyName].get<std::string>();
  // cer
  certificate.Cer = Base64Url::Base64UrlDecode(jsonResponse[CerPropertyName].get<std::string>());

  // policy
  if (jsonResponse.contains(PolicyPropertyName))
  {
    auto const policyJson = jsonResponse[PolicyPropertyName];
    // key_props
    {
      auto const keyPropsJson = policyJson[KeyPropsPropertyName];
      JsonOptional::SetIfExists<std::string, CertificateKeyType>(
          certificate.Policy.KeyType, keyPropsJson, KeyTypePropertyName, [](std::string value) {
            return CertificateKeyType(value);
          });
      JsonOptional::SetIfExists(certificate.Policy.ReuseKey, keyPropsJson, ReuseKeyPropertyName);
      JsonOptional::SetIfExists(
          certificate.Policy.Exportable, keyPropsJson, ExportablePropertyName);
      JsonOptional::SetIfExists<std::string, CertificateKeyCurveName>(
          certificate.Policy.KeyCurveName,
          keyPropsJson,
          CurveNamePropertyName,
          [](std::string value) { return CertificateKeyCurveName(value); });
      JsonOptional::SetIfExists(certificate.Policy.KeySize, keyPropsJson, KeySizePropertyName);
    }
    // secret_props
    {
      auto const secretPropsJson = policyJson[SecretPropsPropertyName];
      JsonOptional::SetIfExists<std::string, CertificateContentType>(
          certificate.Policy.ContentType,
          secretPropsJson,
          ContentTypePropertyName,
          [](std::string value) { return CertificateContentType(value); });
    }
    // x509_props
    {
      auto const x509PropsJson = policyJson[X509PropsPropertyName];
      certificate.Policy.Subject = x509PropsJson[SubjectPropertyName].get<std::string>();
      JsonOptional::SetIfExists<std::vector<std::string>, std::vector<std::string>>(
          certificate.Policy.SubjectAlternativeNames.DnsNames,
          x509PropsJson,
          DnsPropertyName,
          [](std::vector<std::string> const& values) { return values; });
      JsonOptional::SetIfExists<std::vector<std::string>, std::vector<std::string>>(
          certificate.Policy.SubjectAlternativeNames.Emails,
          x509PropsJson,
          EmailsPropertyName,
          [](std::vector<std::string> const& values) { return values; });
      JsonOptional::SetIfExists<std::vector<std::string>, std::vector<std::string>>(
          certificate.Policy.SubjectAlternativeNames.UserPrincipalNames,
          x509PropsJson,
          UserPrincipalNamesPropertyName,
          [](std::vector<std::string> const& values) { return values; });
      JsonOptional::SetIfExists<std::vector<std::string>, std::vector<CertificateKeyUsage>>(
          certificate.Policy.KeyUsage,
          x509PropsJson,
          UserPrincipalNamesPropertyName,
          [](std::vector<std::string> const& values) {
            std::vector<CertificateKeyUsage> keyUsage;
            for (auto const& item : values)
            {
              keyUsage.emplace_back(CertificateKeyUsage(item));
            }
            return keyUsage;
          });
      JsonOptional::SetIfExists<std::vector<std::string>, std::vector<std::string>>(
          certificate.Policy.EnhancedKeyUsage,
          x509PropsJson,
          EkusPropertyName,
          [](std::vector<std::string> const& values) { return values; });
      JsonOptional::SetIfExists(
          certificate.Policy.ValidityInMonths, x509PropsJson, ValidityMonthsPropertyName);
    }
    // issuer
    {
      auto const issuerJson = policyJson[IssuerPropertyName];
      JsonOptional::SetIfExists(certificate.Policy.Issuer.Name, issuerJson, IssuerNamePropertyName);
      JsonOptional::SetIfExists(
          certificate.Policy.Issuer.CertTransparency, issuerJson, CertTransparencyPropertyName);
      JsonOptional::SetIfExists(certificate.Policy.Issuer.Cty, issuerJson, CtyPropertyName);
    }
    // attributes
    {
      auto const policyAttributesJson = policyJson[AttributesPropertyName];
      JsonOptional::SetIfExists(
          certificate.Policy.Enabled, policyAttributesJson, EnabledPropertyName);
      JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
          certificate.Policy.CreatedOn,
          policyAttributesJson,
          CreatedPropertyName,
          PosixTimeConverter::PosixTimeToDateTime);
      JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
          certificate.Policy.UpdatedOn,
          policyAttributesJson,
          UpdatedPropertyName,
          PosixTimeConverter::PosixTimeToDateTime);
    }
    // lifetime_actions
    {
      auto const policyAttributesJson = policyJson[LifetimeActionsPropertyName];
      for (auto const& attributeItem : policyAttributesJson)
      {
        LifetimeAction action;
        JsonOptional::SetIfExists<json, CertificatePolicyAction>(
            action.Action, attributeItem, ActionPropertyName, [](json const& value) {
              return CertificatePolicyAction(value[ActionTypePropertyName].get<std::string>());
            });

        if (attributeItem.contains(TriggerPropertyName))
        {
          auto const triggerPropertyJson = attributeItem[TriggerPropertyName];
          JsonOptional::SetIfExists(
              action.DaysBeforeExpiry, triggerPropertyJson, DaysBeforeExpiryPropertyName);
          JsonOptional::SetIfExists(
              action.LifetimePercentage, triggerPropertyJson, LifetimePercentagePropertyName);
        }
        // At this point the action is parsed from json and can be added to the LifeTimeActions from
        // the policy.
        certificate.Policy.LifetimeActions.emplace_back(action);
      }
    }
  }

  return certificate;
}

namespace Azure { namespace Security { namespace KeyVault { namespace Certificates {

  const CertificateKeyUsage CertificateKeyUsage::DigitalSignature(_detail::DigitalSignatureValue);
  const CertificateKeyUsage CertificateKeyUsage::NonRepudiation(_detail::NonRepudiationValue);
  const CertificateKeyUsage CertificateKeyUsage::KeyEncipherment(_detail::KeyEnciphermentValue);
  const CertificateKeyUsage CertificateKeyUsage::DataEncipherment(_detail::DataEnciphermentValue);
  const CertificateKeyUsage CertificateKeyUsage::KeyAgreement(_detail::KeyAgreementValue);
  const CertificateKeyUsage CertificateKeyUsage::KeyCertSign(_detail::KeyCertSignValue);
  const CertificateKeyUsage CertificateKeyUsage::CrlSign(_detail::CrlSignValue);
  const CertificateKeyUsage CertificateKeyUsage::EncipherOnly(_detail::EncipherOnlyValue);
  const CertificateKeyUsage CertificateKeyUsage::DecipherOnly(_detail::DecipherOnlyValue);

  const CertificateKeyType CertificateKeyType::Ec(_detail::EcValue);
  const CertificateKeyType CertificateKeyType::EcHsm(_detail::EcHsmValue);
  const CertificateKeyType CertificateKeyType::Rsa(_detail::RsaValue);
  const CertificateKeyType CertificateKeyType::RsaHsm(_detail::RsaHsmValue);

  const CertificateKeyCurveName CertificateKeyCurveName::P256(_detail::P256Value);
  const CertificateKeyCurveName CertificateKeyCurveName::P256K(_detail::P256KValue);
  const CertificateKeyCurveName CertificateKeyCurveName::P384(_detail::P384Value);
  const CertificateKeyCurveName CertificateKeyCurveName::P521(_detail::P521Value);

  const CertificateContentType CertificateContentType::Pkcs12(_detail::Pkc12Value);
  const CertificateContentType CertificateContentType::Pem(_detail::PemValue);

  const CertificatePolicyAction CertificatePolicyAction::AutoRenew(_detail::AutoRenewValue);
  const CertificatePolicyAction CertificatePolicyAction::EmailContacts(_detail::EmailContactsValue);

}}}} // namespace Azure::Security::KeyVault::Certificates

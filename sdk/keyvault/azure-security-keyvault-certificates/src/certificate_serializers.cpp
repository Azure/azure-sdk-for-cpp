// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/base64.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_optional.hpp>
#include <azure/core/internal/json/json_serializable.hpp>
#include <azure/core/url.hpp>

#include "azure/keyvault/certificates/certificate_client_models.hpp"
#include "private/certificate_constants.hpp"
#include "private/certificate_key_usage.hpp"
#include "private/certificate_serializers.hpp"

using namespace Azure::Security::KeyVault::Certificates::_detail;
using namespace Azure::Security::KeyVault::Certificates;
using namespace Azure::Core::Json::_internal;
using namespace Azure::Core::_internal;

using Azure::Core::_internal::PosixTimeConverter;

void _detail::KeyVaultCertificateSerializer::Deserialize(
    KeyVaultCertificateWithPolicy& certificate,
    std::string const& name,
    Azure::Core::Http::RawResponse const& rawResponse)
{
  certificate = Deserialize(name, rawResponse);
}

KeyVaultCertificateWithPolicy _detail::KeyVaultCertificateSerializer::Deserialize(
    std::string const& name,
    Azure::Core::Http::RawResponse const& rawResponse)
{
  CertificateProperties properties(name);
  auto const& body = rawResponse.GetBody();
  auto jsonResponse = json::parse(body);

  // Parse URL for the name, vaultUrl and version
  _detail::KeyVaultCertificateSerializer::ParseKeyUrl(
      properties, jsonResponse[IdName].get<std::string>());

  // x5t
  properties.X509Thumbprint = Base64Url::Base64UrlDecode(jsonResponse[X5tName].get<std::string>());

  // "Tags"
  if (jsonResponse.contains(TagsPropertyName))
  {
    properties.Tags
        = jsonResponse[TagsPropertyName].get<std::unordered_map<std::string, std::string>>();
  }

  // "Attributes"
  if (jsonResponse.contains(AttributesPropertyName))
  {
    auto attributes = jsonResponse[AttributesPropertyName];
    CertificatePropertiesSerializer::Deserialize(properties, attributes);
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
    CertificatePolicySerializer::Deserialize(certificate.Policy, policyJson);
  }

  return certificate;
}

void CertificatePropertiesSerializer::Deserialize(
    CertificateProperties& properties,
    Azure::Core::Json::_internal::json fragment)
{
  JsonOptional::SetIfExists(properties.Enabled, fragment, EnabledPropertyName);

  JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
      properties.NotBefore, fragment, NbfPropertyName, PosixTimeConverter::PosixTimeToDateTime);
  JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
      properties.ExpiresOn, fragment, ExpPropertyName, PosixTimeConverter::PosixTimeToDateTime);
  JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
      properties.CreatedOn, fragment, CreatedPropertyName, PosixTimeConverter::PosixTimeToDateTime);
  JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
      properties.UpdatedOn, fragment, UpdatedPropertyName, PosixTimeConverter::PosixTimeToDateTime);
  JsonOptional::SetIfExists(properties.RecoveryLevel, fragment, RecoveryLevelPropertyName);
  JsonOptional::SetIfExists(properties.RecoverableDays, fragment, RecoverableDaysPropertyName);
}

std::string CertificatePropertiesSerializer::Serialize(CertificateProperties const& properties)
{
  return JsonSerialize(properties).dump();
}

Azure::Core::Json::_internal::json CertificatePropertiesSerializer::JsonSerialize(
    CertificateProperties const& properties)
{
  json attributes;

  JsonOptional::SetFromNullable(properties.Enabled, attributes, EnabledPropertyName);
  JsonOptional::SetFromNullable<Azure::DateTime, int64_t>(
      properties.NotBefore, attributes, NbfPropertyName, PosixTimeConverter::DateTimeToPosixTime);
  JsonOptional::SetFromNullable<Azure::DateTime, int64_t>(
      properties.ExpiresOn, attributes, ExpPropertyName, PosixTimeConverter::DateTimeToPosixTime);
  JsonOptional::SetFromNullable<Azure::DateTime, int64_t>(
      properties.CreatedOn,
      attributes,
      CreatedPropertyName,
      PosixTimeConverter::DateTimeToPosixTime);
  JsonOptional::SetFromNullable<Azure::DateTime, int64_t>(
      properties.UpdatedOn,
      attributes,
      UpdatedPropertyName,
      PosixTimeConverter::DateTimeToPosixTime);
  JsonOptional::SetFromNullable(properties.RecoveryLevel, attributes, RecoveryLevelPropertyName);
  JsonOptional::SetFromNullable(
      properties.RecoverableDays, attributes, RecoverableDaysPropertyName);

  return attributes;
}

void CertificatePolicySerializer::Deserialize(
    CertificatePolicy& policy,
    Azure::Core::Json::_internal::json fragment)
{
  // key_props
  {
    auto const keyPropsJson = fragment[KeyPropsPropertyName];
    JsonOptional::SetIfExists<std::string, CertificateKeyType>(
        policy.KeyType, keyPropsJson, KeyTypePropertyName, [](std::string value) {
          return CertificateKeyType(value);
        });
    JsonOptional::SetIfExists(policy.ReuseKey, keyPropsJson, ReuseKeyPropertyName);
    JsonOptional::SetIfExists(policy.Exportable, keyPropsJson, ExportablePropertyName);
    JsonOptional::SetIfExists<std::string, CertificateKeyCurveName>(
        policy.KeyCurveName, keyPropsJson, CurveNamePropertyName, [](std::string value) {
          return CertificateKeyCurveName(value);
        });
    JsonOptional::SetIfExists(policy.KeySize, keyPropsJson, KeySizePropertyName);
  }
  // secret_props
  {
    auto const secretPropsJson = fragment[SecretPropsPropertyName];
    JsonOptional::SetIfExists<std::string, CertificateContentType>(
        policy.ContentType, secretPropsJson, ContentTypePropertyName, [](std::string value) {
          return CertificateContentType(value);
        });
  }
  // x509_props
  {
    auto const x509PropsJson = fragment[X509PropsPropertyName];
    policy.Subject = x509PropsJson[SubjectPropertyName].get<std::string>();
    JsonOptional::SetIfExists<std::vector<std::string>, std::vector<std::string>>(
        policy.SubjectAlternativeNames.DnsNames,
        x509PropsJson,
        DnsPropertyName,
        [](std::vector<std::string> const& values) { return values; });
    JsonOptional::SetIfExists<std::vector<std::string>, std::vector<std::string>>(
        policy.SubjectAlternativeNames.Emails,
        x509PropsJson,
        EmailsPropertyName,
        [](std::vector<std::string> const& values) { return values; });
    JsonOptional::SetIfExists<std::vector<std::string>, std::vector<std::string>>(
        policy.SubjectAlternativeNames.UserPrincipalNames,
        x509PropsJson,
        UserPrincipalNamesPropertyName,
        [](std::vector<std::string> const& values) { return values; });
    JsonOptional::SetIfExists<std::vector<std::string>, std::vector<CertificateKeyUsage>>(
        policy.KeyUsage,
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
        policy.EnhancedKeyUsage,
        x509PropsJson,
        EkusPropertyName,
        [](std::vector<std::string> const& values) { return values; });
    JsonOptional::SetIfExists(policy.ValidityInMonths, x509PropsJson, ValidityMonthsPropertyName);
  }
  // issuer
  {
    auto const issuerJson = fragment[IssuerPropertyName];
    JsonOptional::SetIfExists(policy.IssuerName, issuerJson, IssuerNamePropertyName);
    JsonOptional::SetIfExists(
        policy.CertificateTransparency, issuerJson, CertTransparencyPropertyName);
    JsonOptional::SetIfExists(policy.CertificateType, issuerJson, CtyPropertyName);
  }
  // attributes
  {
    auto const policyAttributesJson = fragment[AttributesPropertyName];
    JsonOptional::SetIfExists(policy.Enabled, policyAttributesJson, EnabledPropertyName);
    JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
        policy.CreatedOn,
        policyAttributesJson,
        CreatedPropertyName,
        PosixTimeConverter::PosixTimeToDateTime);
    JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
        policy.UpdatedOn,
        policyAttributesJson,
        UpdatedPropertyName,
        PosixTimeConverter::PosixTimeToDateTime);
  }
  // lifetime_actions
  {
    auto const policyAttributesJson = fragment[LifetimeActionsPropertyName];
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
      policy.LifetimeActions.emplace_back(action);
    }
  }
}

std::string CertificatePolicySerializer::Serialize(CertificatePolicy const& policy)
{
  return JsonSerialize(policy).dump();
}

Azure::Core::Json::_internal::json CertificatePolicySerializer::JsonSerialize(
    CertificatePolicy const& policy)
{
  json result;
  // key_props
  {
    json fragment;
    JsonOptional::SetFromNullable<CertificateKeyType, std::string>(
        policy.KeyType, fragment, KeyTypePropertyName, [](CertificateKeyType const& keyType) {
          return keyType.ToString();
        });
    JsonOptional::SetFromNullable(policy.ReuseKey, fragment, ReuseKeyPropertyName);
    JsonOptional::SetFromNullable(policy.Exportable, fragment, ExportablePropertyName);
    JsonOptional::SetFromNullable<CertificateKeyCurveName, std::string>(
        policy.KeyCurveName, fragment, CurveNamePropertyName, [](CertificateKeyCurveName name) {
          return name.ToString();
        });
    JsonOptional::SetFromNullable(policy.KeySize, fragment, KeySizePropertyName);

    result[KeyPropsPropertyName] = fragment;
  }

  // secret_props
  {
    json fragment;

    JsonOptional::SetFromNullable<CertificateContentType, std::string>(
        policy.ContentType, fragment, ContentTypePropertyName, [](CertificateContentType value) {
          return value.ToString();
        });

    result[SecretPropsPropertyName] = fragment;
  }

  // x509_props
  {
    json fragment;
    fragment[SubjectPropertyName] = policy.Subject;
    JsonOptional::SetFromNullable<std::vector<std::string>, std::vector<std::string>>(
        policy.SubjectAlternativeNames.DnsNames,
        fragment,
        DnsPropertyName,
        [](std::vector<std::string> const& values) { return values; });
    JsonOptional::SetFromNullable<std::vector<std::string>, std::vector<std::string>>(
        policy.SubjectAlternativeNames.Emails,
        fragment,
        EmailsPropertyName,
        [](std::vector<std::string> const& values) { return values; });
    JsonOptional::SetFromNullable<std::vector<std::string>, std::vector<std::string>>(
        policy.SubjectAlternativeNames.UserPrincipalNames,
        fragment,
        UserPrincipalNamesPropertyName,
        [](std::vector<std::string> const& values) { return values; });
    JsonOptional::SetFromNullable<std::vector<CertificateKeyUsage>, std::vector<std::string>>(
        policy.KeyUsage,
        fragment,
        UserPrincipalNamesPropertyName,
        [](std::vector<CertificateKeyUsage> const& values) {
          std::vector<std::string> keyUsage;
          for (auto const& item : values)
          {
            keyUsage.emplace_back(item.ToString());
          }
          return keyUsage;
        });
    JsonOptional::SetFromNullable<std::vector<std::string>, std::vector<std::string>>(
        policy.EnhancedKeyUsage,
        fragment,
        EkusPropertyName,
        [](std::vector<std::string> const& values) { return values; });
    JsonOptional::SetFromNullable(policy.ValidityInMonths, fragment, ValidityMonthsPropertyName);

    result[X509PropsPropertyName] = fragment;
  }

  // issuer
  {
    json fragment;

    JsonOptional::SetFromNullable(policy.IssuerName, fragment, IssuerNamePropertyName);
    JsonOptional::SetFromNullable(
        policy.CertificateTransparency, fragment, CertTransparencyPropertyName);
    JsonOptional::SetFromNullable(policy.CertificateType, fragment, CtyPropertyName);

    result[IssuerPropertyName] = fragment;
  }

  // attributes
  {
    json fragment;

    JsonOptional::SetFromNullable(policy.Enabled, fragment, EnabledPropertyName);
    JsonOptional::SetFromNullable<Azure::DateTime, int64_t>(
        policy.CreatedOn, fragment, CreatedPropertyName, PosixTimeConverter::DateTimeToPosixTime);
    JsonOptional::SetFromNullable<Azure::DateTime, int64_t>(
        policy.UpdatedOn, fragment, UpdatedPropertyName, PosixTimeConverter::DateTimeToPosixTime);

    result[AttributesPropertyName] = fragment;
  }

  // lifetime_actions

  {
    std::vector<json> fragment;
    for (auto const& action : policy.LifetimeActions)
    {
      json trigger;
      JsonOptional::SetFromNullable(
          action.LifetimePercentage, trigger, LifetimePercentagePropertyName);
      JsonOptional::SetFromNullable(action.DaysBeforeExpiry, trigger, DaysBeforeExpiryPropertyName);

      json actionFragment;

      JsonOptional::SetFromNullable<CertificatePolicyAction, std::string>(
          action.Action,
          actionFragment,
          ActionTypePropertyName,
          [](CertificatePolicyAction const& certAction) { return certAction.ToString(); });
      json lifetimeAction;
      lifetimeAction[TriggerPropertyName] = trigger;
      lifetimeAction[ActionPropertyName] = actionFragment;
      fragment.emplace_back(lifetimeAction);
    }

    result[LifetimeActionsPropertyName] = fragment;
  }

  return result;
}

std::string CertificateCreateParametersSerializer::Serialize(
    CertificateCreateParameters const& parameters)
{
  json parameter;

  parameter[PolicyPropertyName] = CertificatePolicySerializer::JsonSerialize(parameters.Policy);

  parameter[AttributesPropertyName]
      = CertificatePropertiesSerializer::JsonSerialize(parameters.Properties);

  parameter[TagsPropertyName] = json(parameters.Properties.Tags);

  return parameter.dump();
}

DeletedCertificate DeletedCertificateSerializer::Deserialize(
    std::string const& name,
    Azure::Core::Http::RawResponse const& rawResponse)
{
  DeletedCertificate result;

  KeyVaultCertificateSerializer::Deserialize(result, name, rawResponse);

  auto const& body = rawResponse.GetBody();
  auto jsonResponse = json::parse(body);

  result.RecoveryId = jsonResponse[RecoveryIdPropertyName];

  JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
      result.DeletedOn,
      jsonResponse,
      DeletedDatePropertyName,
      PosixTimeConverter::PosixTimeToDateTime);

  JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
      result.ScheduledPurgeDate,
      jsonResponse,
      ScheduledPurgeDatePropertyName,
      PosixTimeConverter::PosixTimeToDateTime);

  return result;
}

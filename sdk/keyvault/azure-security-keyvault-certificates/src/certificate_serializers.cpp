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

CertificateIssuer CertificateIssuerSerializer::Deserialize(
    std::string const& name,
    Azure::Core::Http::RawResponse const& rawResponse)
{
  CertificateIssuer issuer;
  issuer.Name = name;
  auto const& body = rawResponse.GetBody();
  auto jsonResponse = json::parse(body);

  issuer.Id = jsonResponse[IdName];
  issuer.Provider = jsonResponse[ProviderPropertyValue];

  if (jsonResponse.contains(CredentialsPropertyValue))
  {
    auto credentialsJson = jsonResponse[CredentialsPropertyValue];
    JsonOptional::SetIfExists(issuer.Credentials.AccountId, credentialsJson, AccountIdValue);
    JsonOptional::SetIfExists(issuer.Credentials.Password, credentialsJson, PwdPropertyValue);
  }

  if (jsonResponse.contains(OrgDetailsPropertyValue))
  {
    auto orgJson = jsonResponse[OrgDetailsPropertyValue];
    JsonOptional::SetIfExists(issuer.Organization.Id, orgJson, IdName);

    for (auto adminJson : orgJson[AdminDetailsPropertyValue])
    {
      AdministratorDetails admin;
      JsonOptional::SetIfExists(admin.EmailAddress, adminJson, EmailPropertyValue);
      JsonOptional::SetIfExists(admin.FirstName, adminJson, FirstNamePropertyValue);
      JsonOptional::SetIfExists(admin.LastName, adminJson, LastNamePropertyValue);
      JsonOptional::SetIfExists(admin.PhoneNumber, adminJson, PhonePropertyValue);

      issuer.Organization.AdminDetails.emplace_back(admin);
    }
  }

  if (jsonResponse.contains(AttributesPropertyName))
  {
    auto attributesJson = jsonResponse[AttributesPropertyName];

    JsonOptional::SetIfExists(issuer.Properties.Enabled, attributesJson, EnabledPropertyName);
    JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
        issuer.Properties.Created,
        attributesJson,
        CreatedPropertyName,
        PosixTimeConverter::PosixTimeToDateTime);
    JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
        issuer.Properties.Updated,
        attributesJson,
        UpdatedPropertyName,
        PosixTimeConverter::PosixTimeToDateTime);
  }

  return issuer;
}

std::string CertificateIssuerSerializer::Serialize(CertificateIssuer const& issuer)
{

  json jsonResponse;
  JsonOptional::SetFromNullable(issuer.Provider, jsonResponse, ProviderPropertyValue);

  {
    json credentialsJson;
    JsonOptional::SetFromNullable(issuer.Credentials.AccountId, credentialsJson, AccountIdValue);
    JsonOptional::SetFromNullable(issuer.Credentials.Password, credentialsJson, PwdPropertyValue);
    jsonResponse[CredentialsPropertyValue] = credentialsJson;
  }

  {
    json orgJson;
    JsonOptional::SetFromNullable(issuer.Organization.Id, orgJson, IdName);

    for (auto admin : issuer.Organization.AdminDetails)
    {
      json adminJson;
      JsonOptional::SetFromNullable(admin.EmailAddress, adminJson, EmailPropertyValue);
      JsonOptional::SetFromNullable(admin.FirstName, adminJson, FirstNamePropertyValue);
      JsonOptional::SetFromNullable(admin.LastName, adminJson, LastNamePropertyValue);
      JsonOptional::SetFromNullable(admin.PhoneNumber, adminJson, PhonePropertyValue);

      orgJson[AdminDetailsPropertyValue].emplace_back(adminJson);
    }

    jsonResponse[OrgDetailsPropertyValue] = orgJson;
  }

  {
    json attributesJson;

    JsonOptional::SetFromNullable(issuer.Properties.Enabled, attributesJson, EnabledPropertyName);
    JsonOptional::SetFromNullable<Azure::DateTime, int64_t>(
        issuer.Properties.Created,
        attributesJson,
        CreatedPropertyName,
        PosixTimeConverter::DateTimeToPosixTime);
    JsonOptional::SetFromNullable<Azure::DateTime, int64_t>(
        issuer.Properties.Updated,
        attributesJson,
        UpdatedPropertyName,
        PosixTimeConverter::DateTimeToPosixTime);

    jsonResponse[AttributesPropertyName] = attributesJson;
  }

  return jsonResponse.dump();
}

std::string CertificateContactsSerializer::Serialize(
    std::vector<CertificateContact> const& contacts)
{
  json payload;

  for (auto contact : contacts)
  {
    json contactJson;

    contactJson[EmailPropertyName] = contact.EmailAddress;
    JsonOptional::SetFromNullable(contact.Name, contactJson, NamePropertyName);
    JsonOptional::SetFromNullable(contact.Phone, contactJson, PhonePropertyName);

    payload[ContactsPropertyName].emplace_back(contactJson);
  }

  return payload.dump();
}

std::vector<CertificateContact> CertificateContactsSerializer::Deserialize(
    Azure::Core::Http::RawResponse const& rawResponse)
{
  std::vector<CertificateContact> response;

  auto const& body = rawResponse.GetBody();
  auto jsonResponse = json::parse(body);

  if (jsonResponse.contains(ContactsPropertyName))
  {
    for (auto contactJson : jsonResponse[ContactsPropertyName])
    {
      CertificateContact contact;

      contact.EmailAddress = contactJson[EmailPropertyName];
      JsonOptional::SetIfExists(contact.Name, contactJson, NamePropertyName);
      JsonOptional::SetIfExists(contact.Phone, contactJson, PhonePropertyName);

      response.emplace_back(contact);
    }
  }

  return response;
}

CertificateOperationProperties CertificateOperationSerializer ::Deserialize(
    Azure::Core::Http::RawResponse const& rawResponse)
{
  CertificateOperationProperties operation;

  auto const& body = rawResponse.GetBody();
  auto jsonResponse = json::parse(body);

  ParseKeyUrl(operation, jsonResponse[IdName]);

  // issuer
  {
    auto const issuerJson = jsonResponse[IssuerPropertyName];
    JsonOptional::SetIfExists(operation.IssuerName, issuerJson, IssuerNamePropertyName);
    JsonOptional::SetIfExists(
        operation.CertificateTransparency, issuerJson, CertTransparencyPropertyName);
    JsonOptional::SetIfExists(operation.CertificateType, issuerJson, CtyPropertyName);
  }

  operation.Csr = Base64Url::Base64UrlDecode(jsonResponse[CsrPropertyName].get<std::string>());
  JsonOptional::SetIfExists(
      operation.CancellationRequested, jsonResponse, CancelationRequestedPropertyName);
  JsonOptional::SetIfExists(operation.Status, jsonResponse, StatusPropertyName);
  JsonOptional::SetIfExists(operation.StatusDetails, jsonResponse, StatusDetailsPropertyName);
  JsonOptional::SetIfExists(operation.Target, jsonResponse, TargetPropertyName);
  JsonOptional::SetIfExists(operation.RequestId, jsonResponse, RequestIdPropertyName);

  return operation;
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
CerticatePropertiesPagedResponse CerticatePropertiesPagedResponseSerializer::Deserialize(
    Azure::Core::Http::RawResponse const& rawResponse)
{
  CerticatePropertiesPagedResponse response;

  auto const& body = rawResponse.GetBody();
  auto jsonResponse = json::parse(body);

  JsonOptional::SetIfExists(response.NextPageToken, jsonResponse, NextLinkPropertyName);

  // Key properties
  auto certificatePropertiesJson = jsonResponse[ValuePropertyName];

  for (auto const& certificate : certificatePropertiesJson)
  {
    CertificateProperties properties;
    // Parse URL for the name, vaultUrl and version
    _detail::KeyVaultCertificateSerializer::ParseKeyUrl(
        properties, certificate[IdName].get<std::string>());

    // x5t
    properties.X509Thumbprint = Base64Url::Base64UrlDecode(certificate[X5tName].get<std::string>());

    // "Tags"
    if (certificate.contains(TagsPropertyName))
    {
      properties.Tags
          = certificate[TagsPropertyName].get<std::unordered_map<std::string, std::string>>();
    }

    // "Attributes"
    if (certificate.contains(AttributesPropertyName))
    {
      auto attributes = certificate[AttributesPropertyName];
      CertificatePropertiesSerializer::Deserialize(properties, attributes);
    }

    response.Items.emplace_back(properties);
  }

  return response;
}

IssuerPropertiesPagedResponse IssuerPropertiesPagedResponseSerializer::Deserialize(
    Azure::Core::Http::RawResponse const& rawResponse)
{
  IssuerPropertiesPagedResponse response;
  auto const& body = rawResponse.GetBody();
  auto jsonResponse = json::parse(body);
  std::string str = jsonResponse.dump();

  JsonOptional::SetIfExists(response.NextPageToken, jsonResponse, NextLinkPropertyName);

  // Key properties
  auto issuersPropertiesJson = jsonResponse[ValuePropertyName];

  for (auto const& oneIssuer : issuersPropertiesJson)
  {
    CertificateIssuerItem issuer;
    issuer.Id = oneIssuer[IdName].get<std::string>();
    issuer.Provider = oneIssuer[ProviderPropertyValue].get<std::string>();
    response.Items.emplace_back(issuer);
  }

  return response;
}

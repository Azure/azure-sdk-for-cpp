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
  if (jsonResponse.contains(KidPropertyName))
  {

    certificate.KeyIdUrl = jsonResponse[KidPropertyName].get<std::string>();
  } // sid
  if (jsonResponse.contains(SidPropertyName))
  {
    certificate.SecretIdUrl = jsonResponse[SidPropertyName].get<std::string>();
  }
  // cer
  if (jsonResponse.contains(CerPropertyName))
  {
    certificate.Cer = Base64Url::Base64UrlDecode(jsonResponse[CerPropertyName].get<std::string>());
  }

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

CertificatePolicy CertificatePolicySerializer::Deserialize(
    Azure::Core::Http::RawResponse const& rawResponse)
{
  CertificatePolicy policy;
  auto const& body = rawResponse.GetBody();
  auto jsonResponse = json::parse(body);

  Deserialize(policy, jsonResponse);

  return policy;
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
        KeyUsagePropertyName,
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

std::string CertificateCreateOptionsSerializer::Serialize(
    CertificateCreateOptions const& parameters)
{
  json parameter;

  parameter[PolicyPropertyName] = CertificatePolicySerializer::JsonSerialize(parameters.Policy);

  parameter[AttributesPropertyName]
      = CertificatePropertiesSerializer::JsonSerialize(parameters.Properties);

  parameter[TagsPropertyName] = json(parameters.Properties.Tags);

  return parameter.dump();
}

std::string CertificateOperationUpdateOptionSerializer::Serialize(
    CertificateOperationUpdateOptions const& parameters)
{
  json parameter;

  parameter[CancelationRequestedPropertyName] = parameters.CancelationRequested;

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

  issuer.IdUrl = jsonResponse[IdName];
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

CertificateContactsResult CertificateContactsSerializer::Deserialize(
    Azure::Core::Http::RawResponse const& rawResponse)
{
  CertificateContactsResult response;

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

      response.Contacts.emplace_back(contact);
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
  JsonOptional::SetIfExists(operation.RequestIdUrl, jsonResponse, RequestIdPropertyName);

  if (jsonResponse.contains(ErrorPropertyName))
  {
    auto errorJson = jsonResponse[ErrorPropertyName];
    ServerError error;
    ServerErrorSerializer::Deserialize(error, errorJson);
    operation.Error = error;
  }

  return operation;
}

void ServerErrorSerializer::Deserialize(
    ServerError& error,
    Azure::Core::Json::_internal::json fragment)
{
  error.Code = fragment[CodePropertyName].get<std::string>();
  error.Message = fragment[CodePropertyName].get<std::string>();
  if (fragment.contains(InnerErrorPropertyName))
  {
    ServerError innerError;
    error.InnerError = std::make_shared<ServerError>(innerError);
    Deserialize(innerError, fragment[InnerErrorPropertyName]);
  }
}

DeletedCertificate DeletedCertificateSerializer::Deserialize(
    std::string const& name,
    Azure::Core::Http::RawResponse const& rawResponse)
{
  DeletedCertificate result;

  KeyVaultCertificateSerializer::Deserialize(result, name, rawResponse);

  auto const& body = rawResponse.GetBody();
  auto jsonResponse = json::parse(body);

  result.RecoveryIdUrl = jsonResponse[RecoveryIdPropertyName];

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

BackupCertificateResult BackupCertificateSerializer::Deserialize(
    Azure::Core::Http::RawResponse const& rawResponse)
{
  auto const& body = rawResponse.GetBody();
  auto jsonParser = json::parse(body);
  auto encodedResult = jsonParser[ValuePropertyName].get<std::string>();
  BackupCertificateResult data;
  data.Certificate = Base64Url::Base64UrlDecode(encodedResult);

  return data;
}

std::string BackupCertificateSerializer::Serialize(std::vector<uint8_t> const& backup)
{
  json payload;
  payload[_detail::ValuePropertyName] = Base64Url::Base64UrlEncode(backup);
  return payload.dump();
}

CertificatePropertiesPagedResponse CertificatePropertiesPagedResponseSerializer::Deserialize(
    Azure::Core::Http::RawResponse const& rawResponse)
{
  CertificatePropertiesPagedResponse response;

  auto const& body = rawResponse.GetBody();
  auto jsonResponse = json::parse(body);

  JsonOptional::SetIfExists(response.NextPageToken, jsonResponse, NextLinkPropertyName);

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
  std::string data = jsonResponse.dump();
  JsonOptional::SetIfExists(response.NextPageToken, jsonResponse, NextLinkPropertyName);

  auto issuersPropertiesJson = jsonResponse[ValuePropertyName];

  for (auto const& oneIssuer : issuersPropertiesJson)
  {
    CertificateIssuerItem issuer;
    issuer.IdUrl = oneIssuer[IdName].get<std::string>();
    issuer.Provider = oneIssuer[ProviderPropertyValue].get<std::string>();
    ParseIdUrl(issuer, issuer.IdUrl);
    response.Items.emplace_back(issuer);
  }

  return response;
}

DeletedCertificatesPagedResponse DeletedCertificatesPagedResponseSerializer::Deserialize(
    Azure::Core::Http::RawResponse const& rawResponse)
{
  DeletedCertificatesPagedResponse response;
  auto const& body = rawResponse.GetBody();
  auto jsonResponse = json::parse(body);

  JsonOptional::SetIfExists(response.NextPageToken, jsonResponse, NextLinkPropertyName);
  auto deletedCertificates = jsonResponse[ValuePropertyName];

  for (auto const& oneDeleted : deletedCertificates)
  {
    std::string deletedString = oneDeleted.dump();
    std::vector<uint8_t> vec(deletedString.begin(), deletedString.end());

    Azure::Core::Http::RawResponse fakeResponse(
        1, 1, Azure::Core::Http::HttpStatusCode::Ok, "Success");
    fakeResponse.SetBody(vec);

    auto deserializedDeletedCert = DeletedCertificateSerializer::Deserialize("", fakeResponse);

    response.Items.emplace_back(deserializedDeletedCert);
  }

  return response;
}

KeyVaultSecret KeyVaultSecretSerializer::Deserialize(
    Azure::Core::Http::RawResponse const& rawResponse)
{
  KeyVaultSecret response;
  auto const& body = rawResponse.GetBody();
  auto jsonResponse = json::parse(body);
  std::string str = jsonResponse.dump();

  response.Value = jsonResponse[ValuePropertyName];
  JsonOptional::SetIfExists<std::string, CertificateContentType>(
      response.ContentType, jsonResponse, ContentTypePropertyName, [](std::string value) {
        return CertificateContentType(value);
      });

  return response;
}

std::string ImportCertificateOptionsSerializer::Serialize(ImportCertificateOptions const& options)
{
  json importOptions;

  importOptions[ValuePropertyName] = options.Certificate;
  JsonOptional::SetFromNullable(options.Password, importOptions, PwdPropertyValue);
  importOptions[PolicyPropertyName] = CertificatePolicySerializer::JsonSerialize(options.Policy);
  importOptions[AttributesPropertyName]
      = CertificatePropertiesSerializer::JsonSerialize(options.Properties);
  importOptions[TagsPropertyName] = json(options.Tags);

  return importOptions.dump();
}

std::string MergeCertificateOptionsSerializer::Serialize(MergeCertificateOptions const& options)
{
  json mergeOptions;

  mergeOptions[X5cPropertyName] = json(options.Certificates);
  mergeOptions[AttributesPropertyName]
      = CertificatePropertiesSerializer::JsonSerialize(options.Properties);
  mergeOptions[TagsPropertyName] = json(options.Tags);

  return mergeOptions.dump();
}

std::string CertificateUpdateOptionsSerializer::Serialize(
    CertificateProperties const& certificateProperties)
{
  json updateOptions;

  updateOptions[AttributesPropertyName]
      = CertificatePropertiesSerializer::JsonSerialize(certificateProperties);
  updateOptions[TagsPropertyName] = json(certificateProperties.Tags);

  return updateOptions.dump();
}

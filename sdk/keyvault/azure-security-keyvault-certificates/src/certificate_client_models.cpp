// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/keyvault/certificates/certificate_client_models.hpp"

#include "generated/certificates_models.hpp"
#include "private/certificate_constants.hpp"
#include "private/certificate_serializers.hpp"

using namespace Azure::Security::KeyVault::Certificates;

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

KeyVaultCertificateWithPolicy::KeyVaultCertificateWithPolicy(
    _detail::Models::CertificateBundle const& bundle)
    : KeyVaultCertificate(bundle)
{
  if (bundle.Policy.HasValue())
  {
    auto policy = bundle.Policy.Value();
    if (policy.IssuerParameters.HasValue())
    {
      Policy.CertificateTransparency = policy.IssuerParameters.Value().CertificateTransparency;
      Policy.CertificateType = policy.IssuerParameters.Value().CertificateType;
      Policy.IssuerName = policy.IssuerParameters.Value().Name;
    }
    if (policy.SecretProperties.HasValue()
        && policy.SecretProperties.Value().ContentType.HasValue())
    {
      Policy.ContentType
          = CertificateContentType(policy.SecretProperties.Value().ContentType.Value());
    }
    if (policy.Attributes.HasValue())
    {
      Policy.Enabled = policy.Attributes.Value().Enabled;
      Policy.CreatedOn = policy.Attributes.Value().Created;
      Policy.UpdatedOn = policy.Attributes.Value().Updated;
    }
    if (policy.X509CertificateProperties.HasValue())
    {
      auto keyUsage = policy.X509CertificateProperties.Value().KeyUsage;
      if (keyUsage.HasValue())
      {
        for (auto const& item : keyUsage.Value())
        {
          Policy.KeyUsage.emplace_back(CertificateKeyUsage(item.ToString()));
        }
      }
      auto enhancedKeyUsage = policy.X509CertificateProperties.Value().Ekus;
      if (enhancedKeyUsage.HasValue())
      {
        for (auto const& item : enhancedKeyUsage.Value())
        {
          Policy.KeyUsage.emplace_back(CertificateKeyUsage(item));
        }
      }
      Policy.ValidityInMonths = policy.X509CertificateProperties.Value().ValidityInMonths;
      if (policy.X509CertificateProperties.Value().Subject.HasValue())
      {
        Policy.Subject = policy.X509CertificateProperties.Value().Subject.Value();
      }
      if (policy.X509CertificateProperties.Value().SubjectAlternativeNames.HasValue())
      {
        auto subjectAlternativeNames
            = policy.X509CertificateProperties.Value().SubjectAlternativeNames.Value();
        if (subjectAlternativeNames.Emails.HasValue())
        {
          Policy.SubjectAlternativeNames.Emails = subjectAlternativeNames.Emails.Value();
        }
        if (subjectAlternativeNames.DnsNames.HasValue())
        {
          Policy.SubjectAlternativeNames.DnsNames = subjectAlternativeNames.DnsNames.Value();
        }
        if (subjectAlternativeNames.Upns.HasValue())
        {
          Policy.SubjectAlternativeNames.UserPrincipalNames = subjectAlternativeNames.Upns.Value();
        }
      }
    }
    if (policy.LifetimeActions.HasValue())
    {
      auto lifetimeActions = policy.LifetimeActions.Value();
      for (auto const& item : lifetimeActions)
      {
        LifetimeAction action;
        if (item.Trigger.HasValue())
        {
          action.DaysBeforeExpiry = item.Trigger.Value().DaysBeforeExpiry;
          action.LifetimePercentage = item.Trigger.Value().LifetimePercentage;
        }
        if (item.Action.HasValue() && item.Action.Value().ActionType.HasValue())

        {
          action.Action
              = CertificatePolicyAction(item.Action.Value().ActionType.Value().ToString());
        }
        Policy.LifetimeActions.emplace_back(action);
      }
    }
    if (policy.KeyProperties.HasValue())
    {
      auto keyProperties = policy.KeyProperties.Value();
      if (keyProperties.Exportable.HasValue())
      {
        Policy.Exportable = keyProperties.Exportable.Value();
      }
      if (keyProperties.ReuseKey.HasValue())
      {
        Policy.ReuseKey = keyProperties.ReuseKey.Value();
      }
      if (keyProperties.KeySize.HasValue())
      {
        Policy.KeySize = keyProperties.KeySize.Value();
      }
      if (keyProperties.Curve.HasValue())
      {
        Policy.KeyCurveName = CertificateKeyCurveName(keyProperties.Curve.Value().ToString());
      }
      if (keyProperties.KeyType.HasValue())
      {
        Policy.KeyType = CertificateKeyType(keyProperties.KeyType.Value().ToString());
      }
    }
  }
}

KeyVaultCertificate::KeyVaultCertificate(_detail::Models::CertificateBundle const& bundle)
{
  if (bundle.Kid.HasValue())
  {
    KeyIdUrl = bundle.Kid.Value();
  }
  if (bundle.Sid.HasValue())
  {
    SecretIdUrl = bundle.Sid.Value();
  }
  if (bundle.Cer.HasValue())
  {
    Cer = bundle.Cer.Value();
  }
  Properties = CertificateProperties(bundle);
}

CertificateProperties::CertificateProperties(_detail::Models::CertificateBundle const& bundle)
{
  if (bundle.Attributes.HasValue())
  {
    CreatedOn = bundle.Attributes.Value().Created;
    Enabled = bundle.Attributes.Value().Enabled;
    ExpiresOn = bundle.Attributes.Value().Expires;
    NotBefore = bundle.Attributes.Value().NotBefore;
    RecoverableDays = bundle.Attributes.Value().RecoverableDays;
    UpdatedOn = bundle.Attributes.Value().Updated;
    if (bundle.Attributes.Value().RecoveryLevel.HasValue())
    {
      RecoveryLevel = bundle.Attributes.Value().RecoveryLevel.Value().ToString();
    }
  }
  _detail::KeyVaultCertificateSerializer::ParseKeyUrl(*this, bundle.Id.Value());
  if (bundle.Tags.HasValue())
  {
    Tags = std::unordered_map<std::string, std::string>(
        bundle.Tags.Value().begin(), bundle.Tags.Value().end());
  }
  if (bundle.X509Thumbprint.HasValue())
  {
    X509Thumbprint = bundle.X509Thumbprint.Value();
  }
}

_detail::Models::CertificateIssuerSetParameters
CertificateIssuer::ToCertificateIssuerSetParameters()
{
  _detail::Models::CertificateIssuerSetParameters issuer;
  if (Provider.HasValue())
  {
    issuer.Provider = Provider.Value();
  }
  {
    _detail::Models::IssuerCredentials creds;
    creds.Password = Credentials.Password;
    creds.AccountId = Credentials.AccountId;
    issuer.Credentials = creds;
  }
  {
    _detail::Models::OrganizationDetails org;
    org.Id = Organization.Id;
    std::vector<_detail::Models::AdministratorDetails> admins;
    for (auto admin : Organization.AdminDetails)
    {
      _detail::Models::AdministratorDetails adminDetails;
      adminDetails.EmailAddress = admin.EmailAddress;
      adminDetails.FirstName = admin.FirstName;
      adminDetails.LastName = admin.LastName;
      adminDetails.Phone = admin.PhoneNumber;
      admins.emplace_back(adminDetails);
    }
    org.AdminDetails = admins;
    issuer.OrganizationDetails = org;
  }
  {
    _detail::Models::IssuerAttributes attributes;
    attributes.Enabled = Properties.Enabled;
    attributes.Created = Properties.Created;
    attributes.Updated = Properties.Updated;
    issuer.Attributes = attributes;
  }
  return issuer;
};
_detail::Models::CertificateIssuerUpdateParameters
CertificateIssuer::ToCertificateIssuerUpdateParameters()
{
  _detail::Models::CertificateIssuerUpdateParameters issuer;
  if (Provider.HasValue())
  {
    issuer.Provider = Provider.Value();
  }
  {
    _detail::Models::IssuerCredentials creds;
    creds.Password = Credentials.Password;
    creds.AccountId = Credentials.AccountId;
    issuer.Credentials = creds;
  }
  {
    _detail::Models::OrganizationDetails org;
    org.Id = Organization.Id;
    std::vector<_detail::Models::AdministratorDetails> admins;
    for (auto admin : Organization.AdminDetails)
    {
      _detail::Models::AdministratorDetails adminDetails;
      adminDetails.EmailAddress = admin.EmailAddress;
      adminDetails.FirstName = admin.FirstName;
      adminDetails.LastName = admin.LastName;
      adminDetails.Phone = admin.PhoneNumber;
      admins.emplace_back(adminDetails);
    }
    org.AdminDetails = admins;
    issuer.OrganizationDetails = org;
  }
  {
    _detail::Models::IssuerAttributes attributes;
    attributes.Enabled = Properties.Enabled;
    attributes.Created = Properties.Created;
    attributes.Updated = Properties.Updated;
    issuer.Attributes = attributes;
  }
  return issuer;
}

CertificateIssuer::CertificateIssuer(
    std::string const& name,
    _detail::Models::IssuerBundle const& issuer)
    : Name(std::move(name))
{

  Provider = issuer.Provider;
  if (issuer.Credentials.HasValue())
  {
    Credentials.AccountId = issuer.Credentials.Value().AccountId;
    Credentials.Password = issuer.Credentials.Value().Password;
  }
  if (issuer.OrganizationDetails.HasValue())
  {
    Organization.Id = issuer.OrganizationDetails.Value().Id;
    if (issuer.OrganizationDetails.Value().AdminDetails.HasValue())
    {
      for (auto admin : issuer.OrganizationDetails.Value().AdminDetails.Value())
      {
        AdministratorDetails adminDetails;
        adminDetails.EmailAddress = admin.EmailAddress;
        adminDetails.FirstName = admin.FirstName;
        adminDetails.LastName = admin.LastName;
        adminDetails.PhoneNumber = admin.Phone;
        Organization.AdminDetails.emplace_back(adminDetails);
      }
    }
  }
  if (issuer.Attributes.HasValue())
  {
    Properties.Enabled = issuer.Attributes.Value().Enabled;
    Properties.Created = issuer.Attributes.Value().Created;
    Properties.Updated = issuer.Attributes.Value().Updated;
  }
  if (issuer.Id.HasValue())
  {
    IdUrl = issuer.Id.Value();
  }

}

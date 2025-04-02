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
    Policy = CertificatePolicy(bundle.Policy.Value());
  }
}
KeyVaultCertificateWithPolicy::KeyVaultCertificateWithPolicy(
    _detail::Models::DeletedCertificateBundle const& bundle)
    : KeyVaultCertificate(bundle)
{
  if (bundle.Policy.HasValue())
  {
    Policy = CertificatePolicy(bundle.Policy.Value());
  }
}
KeyVaultCertificate::KeyVaultCertificate(_detail::Models::DeletedCertificateBundle const& bundle)
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
CertificateProperties::CertificateProperties(
    _detail::Models::DeletedCertificateBundle const& bundle)
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
CertificateProperties::CertificateProperties(_detail::Models::CertificateItem const& item)
{
  if (item.Attributes.HasValue())
  {
    CreatedOn = item.Attributes.Value().Created;
    Enabled = item.Attributes.Value().Enabled;
    ExpiresOn = item.Attributes.Value().Expires;
    NotBefore = item.Attributes.Value().NotBefore;
    RecoverableDays = item.Attributes.Value().RecoverableDays;
    UpdatedOn = item.Attributes.Value().Updated;
    if (item.Attributes.Value().RecoveryLevel.HasValue())
    {
      RecoveryLevel = item.Attributes.Value().RecoveryLevel.Value().ToString();
    }
  }
  _detail::KeyVaultCertificateSerializer::ParseKeyUrl(*this, item.Id.Value());
  if (item.Tags.HasValue())
  {
    Tags = std::unordered_map<std::string, std::string>(
        item.Tags.Value().begin(), item.Tags.Value().end());
  }
  if (item.X509Thumbprint.HasValue())
  {
    X509Thumbprint = item.X509Thumbprint.Value();
  }
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

_detail::Models::CertificateUpdateParameters CertificateProperties::ToCertificateUpdateParameters()
{
  _detail::Models::CertificateUpdateParameters update;
  if (Tags.size() > 0)
  {
    update.Tags = std::map<std::string, std::string>(Tags.begin(), Tags.end());
  }
  if (Enabled.HasValue() || CreatedOn.HasValue() || ExpiresOn.HasValue() || NotBefore.HasValue()
      || RecoverableDays.HasValue() || RecoveryLevel.HasValue() || UpdatedOn.HasValue())
  {
    _detail::Models::CertificateAttributes attributes;
    attributes.Enabled = Enabled;
    attributes.Created = CreatedOn;
    attributes.Expires = ExpiresOn;
    attributes.NotBefore = NotBefore;
    attributes.RecoverableDays = RecoverableDays;
    attributes.RecoveryLevel = _detail::Models::DeletionRecoveryLevel(RecoveryLevel.Value());
    attributes.Updated = UpdatedOn;
    update.CertificateAttributes = attributes;
  }
  return update;
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

CertificateContactsResult::CertificateContactsResult(_detail::Models::Contacts contacts)
{
  Contacts = std::vector<CertificateContact>();
  for (auto contact : contacts.ContactList.Value())
  {
    CertificateContact contactDetails;
    if (contact.EmailAddress.HasValue())
    {
      contactDetails.EmailAddress = contact.EmailAddress.Value();
    }
    contactDetails.Name = contact.Name;
    contactDetails.Phone = contact.Phone;
    Contacts.emplace_back(contactDetails);
  }
}

CertificatePolicy::CertificatePolicy(_detail::Models::CertificatePolicy const& policy)
{
  if (policy.IssuerParameters.HasValue())
  {
    CertificateTransparency = policy.IssuerParameters.Value().CertificateTransparency;
    CertificateType = policy.IssuerParameters.Value().CertificateType;
    IssuerName = policy.IssuerParameters.Value().Name;
  }
  if (policy.SecretProperties.HasValue() && policy.SecretProperties.Value().ContentType.HasValue())
  {
    ContentType = CertificateContentType(policy.SecretProperties.Value().ContentType.Value());
  }
  if (policy.Attributes.HasValue())
  {
    Enabled = policy.Attributes.Value().Enabled;
    CreatedOn = policy.Attributes.Value().Created;
    UpdatedOn = policy.Attributes.Value().Updated;
  }
  if (policy.X509CertificateProperties.HasValue())
  {
    auto keyUsage = policy.X509CertificateProperties.Value().KeyUsage;
    if (keyUsage.HasValue())
    {
      for (auto const& item : keyUsage.Value())
      {
        KeyUsage.emplace_back(CertificateKeyUsage(item.ToString()));
      }
    }
    auto enhancedKeyUsage = policy.X509CertificateProperties.Value().Ekus;
    if (enhancedKeyUsage.HasValue())
    {
      for (auto const& item : enhancedKeyUsage.Value())
      {
        KeyUsage.emplace_back(CertificateKeyUsage(item));
      }
    }
    ValidityInMonths = policy.X509CertificateProperties.Value().ValidityInMonths;
    if (policy.X509CertificateProperties.Value().Subject.HasValue())
    {
      Subject = policy.X509CertificateProperties.Value().Subject.Value();
    }
    if (policy.X509CertificateProperties.Value().SubjectAlternativeNames.HasValue())
    {
      auto subjectAlternativeNames
          = policy.X509CertificateProperties.Value().SubjectAlternativeNames.Value();
      if (subjectAlternativeNames.Emails.HasValue())
      {
        SubjectAlternativeNames.Emails = subjectAlternativeNames.Emails.Value();
      }
      if (subjectAlternativeNames.DnsNames.HasValue())
      {
        SubjectAlternativeNames.DnsNames = subjectAlternativeNames.DnsNames.Value();
      }
      if (subjectAlternativeNames.Upns.HasValue())
      {
        SubjectAlternativeNames.UserPrincipalNames = subjectAlternativeNames.Upns.Value();
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
        action.Action = CertificatePolicyAction(item.Action.Value().ActionType.Value().ToString());
      }
      LifetimeActions.emplace_back(action);
    }
  }
  if (policy.KeyProperties.HasValue())
  {
    auto keyProperties = policy.KeyProperties.Value();
    if (keyProperties.Exportable.HasValue())
    {
      Exportable = keyProperties.Exportable.Value();
    }
    if (keyProperties.ReuseKey.HasValue())
    {
      ReuseKey = keyProperties.ReuseKey.Value();
    }
    if (keyProperties.KeySize.HasValue())
    {
      KeySize = keyProperties.KeySize.Value();
    }
    if (keyProperties.Curve.HasValue())
    {
      KeyCurveName = CertificateKeyCurveName(keyProperties.Curve.Value().ToString());
    }
    if (keyProperties.KeyType.HasValue())
    {
      KeyType = CertificateKeyType(keyProperties.KeyType.Value().ToString());
    }
  }
}
_detail::Models::CertificatePolicy CertificatePolicy::ToCertificatePolicy() const
{
  _detail::Models::CertificatePolicy result;
  if (Enabled.HasValue() || CreatedOn.HasValue() || UpdatedOn.HasValue())
  {
    _detail::Models::CertificateAttributes attributes;
    if (CreatedOn.HasValue())
    {
      attributes.Created = CreatedOn.Value();
    }
    if (Enabled.HasValue())
    {
      attributes.Enabled = Enabled.Value();
    }
    // attributes.Expires = ;
    // attributes.NotBefore = ;
    // attributes.RecoverableDays = ;
    // attributes.RecoveryLevel = ;
    if (UpdatedOn.HasValue())
    {
      attributes.Updated = UpdatedOn.Value();
    }

    result.Attributes = attributes;
  }
  if (IssuerName.HasValue() || CertificateTransparency.HasValue() || CertificateType.HasValue())
  {
    _detail::Models::IssuerParameters issuer;
    if (IssuerName.HasValue())
    {
      issuer.Name = IssuerName.Value();
    }
    if (CertificateTransparency.HasValue())
    {
      issuer.CertificateTransparency = CertificateTransparency.Value();
    }
    if (CertificateType.HasValue())
    {
      issuer.CertificateType = CertificateType.Value();
    }
    result.IssuerParameters = issuer;
  }
  if (Exportable.HasValue() || ReuseKey.HasValue() || KeySize.HasValue() || KeyCurveName.HasValue()
      || KeyType.HasValue())
  {
    _detail::Models::KeyProperties keyProperties;
    if (Exportable.HasValue())
    {
      keyProperties.Exportable = Exportable.Value();
    }
    if (ReuseKey.HasValue())
    {
      keyProperties.ReuseKey = ReuseKey.Value();
    }
    if (KeySize.HasValue())
    {
      keyProperties.KeySize = KeySize.Value();
    }
    if (KeyCurveName.HasValue())
    {
      keyProperties.Curve = _detail::Models::JsonWebKeyCurveName(KeyCurveName.Value().ToString());
    }
    if (KeyType.HasValue())
    {
      keyProperties.KeyType = _detail::Models::JsonWebKeyType(KeyType.Value().ToString());
    }
    result.KeyProperties = keyProperties;
  }
  if (LifetimeActions.size() > 0)
  {
    std::vector<_detail::Models::LifetimeAction> actions;
    for (auto const& item : LifetimeActions)
    {
      _detail::Models::LifetimeAction action;
      if (item.DaysBeforeExpiry.HasValue() || item.LifetimePercentage.HasValue())
      {
        _detail::Models::Trigger trigger;
        if (item.DaysBeforeExpiry.HasValue())
        {
          trigger.DaysBeforeExpiry = item.DaysBeforeExpiry.Value();
        }
        if (item.LifetimePercentage.HasValue())
        {
          trigger.LifetimePercentage = item.LifetimePercentage.Value();
        }
        action.Trigger = trigger;
      }
      _detail::Models::Action actionType;
      actionType.ActionType = _detail::Models::CertificatePolicyAction(item.Action.ToString());
      action.Action = actionType;
      actions.emplace_back(action);
    }
    result.LifetimeActions = actions;
  }
  if (ContentType.HasValue())
  {
    _detail::Models::SecretProperties secretProps;
    secretProps.ContentType = ContentType.Value().ToString();
    result.SecretProperties = secretProps;
  }
  if (Subject.size() > 0 || EnhancedKeyUsage.size() > 0 || KeyUsage.size() > 0
      || SubjectAlternativeNames.Emails.size() > 0 || SubjectAlternativeNames.DnsNames.size() > 0
      || SubjectAlternativeNames.UserPrincipalNames.size() > 0 || ValidityInMonths.HasValue())
  {
    _detail::Models::X509CertificateProperties x509Props;
    if (Subject.size() > 0)
    {
      x509Props.Subject = Subject;
    }
    if (EnhancedKeyUsage.size() > 0)
    {
      std::vector<std::string> keyUsages;
      for (auto const& item : EnhancedKeyUsage)
      {
        keyUsages.emplace_back(item);
      }
      x509Props.Ekus = keyUsages;
    }
    if (KeyUsage.size() > 0)
    {
      std::vector<_detail::Models::KeyUsageType> keyUsages;
      for (auto const& item : KeyUsage)
      {
        if (item == CertificateKeyUsage::DigitalSignature)
        {
          keyUsages.emplace_back(_detail::Models::KeyUsageType(_detail::DigitalSignatureValue));
        }
        else if (item == CertificateKeyUsage::NonRepudiation)
        {
          keyUsages.emplace_back(_detail::Models::KeyUsageType(_detail::NonRepudiationValue));
        }
        else if (item == CertificateKeyUsage::KeyEncipherment)
        {
          keyUsages.emplace_back(_detail::Models::KeyUsageType(_detail::KeyEnciphermentValue));
        }
        else if (item == CertificateKeyUsage::DataEncipherment)
        {
          keyUsages.emplace_back(_detail::Models::KeyUsageType(_detail::DataEnciphermentValue));
        }
        else if (item == CertificateKeyUsage::KeyAgreement)
        {
          keyUsages.emplace_back(_detail::Models::KeyUsageType(_detail::KeyAgreementValue));
        }
        else if (item == CertificateKeyUsage::KeyCertSign)
        {
          keyUsages.emplace_back(_detail::Models::KeyUsageType(_detail::KeyCertSignValue));
        }
        else if (item == CertificateKeyUsage::CrlSign)
        {
          keyUsages.emplace_back(_detail::Models::KeyUsageType(_detail::CrlSignValue));
        }
        else if (item == CertificateKeyUsage::EncipherOnly)
        {
          keyUsages.emplace_back(_detail::Models::KeyUsageType(_detail::EncipherOnlyValue));
        }
        else if (item == CertificateKeyUsage::DecipherOnly)
        {
          keyUsages.emplace_back(_detail::Models::KeyUsageType(_detail::DecipherOnlyValue));
        }
      }
      x509Props.KeyUsage = keyUsages;
    }
    if (SubjectAlternativeNames.Emails.size() > 0 || SubjectAlternativeNames.DnsNames.size() > 0
        || SubjectAlternativeNames.UserPrincipalNames.size() > 0)
    {
      _detail::Models::SubjectAlternativeNames subjectAlternativeNames;

      if (SubjectAlternativeNames.Emails.size() > 0)
      {
        subjectAlternativeNames.Emails = SubjectAlternativeNames.Emails;
      }
      if (SubjectAlternativeNames.DnsNames.size() > 0)
      {
        subjectAlternativeNames.DnsNames = SubjectAlternativeNames.DnsNames;
      }
      if (SubjectAlternativeNames.UserPrincipalNames.size() > 0)
      {
        subjectAlternativeNames.Upns = SubjectAlternativeNames.UserPrincipalNames;
      }
      x509Props.SubjectAlternativeNames = subjectAlternativeNames;
    }
    if (ValidityInMonths.HasValue())
    {
      x509Props.ValidityInMonths = ValidityInMonths.Value();
    }
    result.X509CertificateProperties = x509Props;
  }
  return result;
}

DeletedCertificate::DeletedCertificate(_detail::Models::DeletedCertificateBundle const& bundle)
    : KeyVaultCertificateWithPolicy(bundle)
{
  if (bundle.RecoveryId.HasValue())
  {
    RecoveryIdUrl = bundle.RecoveryId.Value();
  }
  if (bundle.DeletedDate.HasValue())
  {
    DeletedOn = bundle.DeletedDate.Value();
  }
  if (bundle.ScheduledPurgeDate.HasValue())
  {
    ScheduledPurgeDate = bundle.ScheduledPurgeDate.Value();
  }
}
DeletedCertificate::DeletedCertificate(_detail::Models::DeletedCertificateItem const& item) {
  if (item.RecoveryId.HasValue())
  {
    RecoveryIdUrl = item.RecoveryId.Value();
  }
  if (item.DeletedDate.HasValue())
  {
    DeletedOn = item.DeletedDate.Value();
  }
  if (item.ScheduledPurgeDate.HasValue())
  {
    ScheduledPurgeDate = item.ScheduledPurgeDate.Value();
  }
}
_detail::Models::CertificateMergeParameters MergeCertificateOptions::ToCertificateMergeParameters()
{
  _detail::Models::CertificateMergeParameters parameters;
  if (Tags.size() > 0)
  {
    parameters.Tags = std::map<std::string, std::string>(Tags.begin(), Tags.end());
  }
  if (Properties.Enabled.HasValue() || Properties.CreatedOn.HasValue()
      || Properties.ExpiresOn.HasValue() || Properties.NotBefore.HasValue()
      || Properties.RecoverableDays.HasValue() || Properties.RecoveryLevel.HasValue()
      || Properties.UpdatedOn.HasValue())
  {
    _detail::Models::CertificateAttributes attributes;
    attributes.Enabled = Properties.Enabled;
    attributes.Created = Properties.CreatedOn;
    attributes.Expires = Properties.ExpiresOn;
    attributes.NotBefore = Properties.NotBefore;
    attributes.RecoverableDays = Properties.RecoverableDays;
    attributes.RecoveryLevel
        = _detail::Models::DeletionRecoveryLevel(Properties.RecoveryLevel.Value());
    attributes.Updated = Properties.UpdatedOn;
    parameters.CertificateAttributes = attributes;
  }
  if (this->Certificates.size() > 0)
  {
    for (auto const& cert : this->Certificates)
    {
      parameters.X509Certificates.emplace_back(std::vector<uint8_t>(cert.begin(), cert.end()));
    }
  }
  return parameters;
}

_detail::Models::CertificateImportParameters
ImportCertificateOptions::ToCertificateImportParameters()
{
  _detail::Models::CertificateImportParameters parameters;
  if (Tags.size() > 0)
  {
    parameters.Tags = std::map<std::string, std::string>(Tags.begin(), Tags.end());
  }
  parameters.Base64EncodedCertificate = Certificate;
  parameters.Password = Password;
  if (Properties.Enabled.HasValue() || Properties.CreatedOn.HasValue()
      || Properties.ExpiresOn.HasValue() || Properties.NotBefore.HasValue()
      || Properties.RecoverableDays.HasValue() || Properties.RecoveryLevel.HasValue()
      || Properties.UpdatedOn.HasValue())
  {
    _detail::Models::CertificateAttributes attributes;
    attributes.Enabled = Properties.Enabled;
    attributes.Created = Properties.CreatedOn;
    attributes.Expires = Properties.ExpiresOn;
    attributes.NotBefore = Properties.NotBefore;
    attributes.RecoverableDays = Properties.RecoverableDays;
    attributes.RecoveryLevel
        = _detail::Models::DeletionRecoveryLevel(Properties.RecoveryLevel.Value());
    attributes.Updated = Properties.UpdatedOn;
    parameters.CertificateAttributes = attributes;
  }
  parameters.CertificatePolicy = Policy.ToCertificatePolicy();
  // parameters.PreserveCertOrder;

  return parameters;
}

_detail::Models::CertificateCreateParameters
CertificateCreateOptions::ToCertificateCreateParameters()
{
  _detail::Models::CertificateCreateParameters parameters;
  {
    parameters.Tags = std::map<std::string, std::string>(Tags.begin(), Tags.end());
  }
  {
    _detail::Models::CertificatePolicy policy = Policy.ToCertificatePolicy();
    parameters.CertificatePolicy = policy;
  }
  {
    _detail::Models::CertificateAttributes attributes;
    attributes.Enabled = Properties.Enabled;
    attributes.Created = Properties.CreatedOn;
    attributes.Expires = Properties.ExpiresOn;
    attributes.NotBefore = Properties.NotBefore;
    attributes.RecoverableDays = Properties.RecoverableDays;
    if (Properties.RecoveryLevel.HasValue())
    {
      attributes.RecoveryLevel
          = _detail::Models::DeletionRecoveryLevel(Properties.RecoveryLevel.Value());
    }
    attributes.Updated = Properties.UpdatedOn;
    parameters.CertificateAttributes = attributes;
  }
  return parameters;
}

CertificateOperationProperties::CertificateOperationProperties(
    _detail::Models::CertificateOperation const& operation)
{
  if (operation.Id.HasValue())
  {
    IdUrl = operation.Id.Value();
  }
  if (operation.Csr.HasValue())
  {
    Csr = operation.Csr.Value();
  }
  if (operation.CancellationRequested.HasValue())
  {
    CancellationRequested = operation.CancellationRequested.Value();
  }
  if (operation.Status.HasValue())
  {
    Status = operation.Status.Value();
  }
  if (operation.StatusDetails.HasValue())
  {
    StatusDetails = operation.StatusDetails.Value();
  }
  if (operation.Target.HasValue())
  {
    Target = operation.Target.Value();
  }
  if (operation.RequestId.HasValue())
  {
    RequestIdUrl = operation.RequestId.Value();
  }
  if (operation.IssuerParameters.HasValue())
  {
    IssuerName = operation.IssuerParameters.Value().Name;
    CertificateTransparency = operation.IssuerParameters.Value().CertificateTransparency;
    CertificateType = operation.IssuerParameters.Value().CertificateType;
  }
  if (operation.Error.HasValue())
  {
    Error = ServerError();
    if (operation.Error.Value().Message.HasValue())
    {
      Error.Value().Code = operation.Error.Value().Code.Value();
    }
    if (operation.Error.Value().Message.HasValue())
    {
      Error.Value().Message = operation.Error.Value().Message.Value();
    }
  }

  /*if (operation.PreserveCertOrder.HasValue())
  {
    PreserveCertOrder = operation.PreserveCertOrder.Value();
  }*/
}


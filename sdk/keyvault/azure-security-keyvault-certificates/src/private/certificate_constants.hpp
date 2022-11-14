//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Centralize the string constants used by Key Vault Certificates Client.
 *
 */

#pragma once

namespace Azure { namespace Security { namespace KeyVault { namespace Certificates {
  namespace _detail {

    /***************** Certificates Requests *****************/
    constexpr static const char KeyVaultServicePackageName[] = "keyvault-certificates";
    constexpr static const char CertificatesPath[] = "certificates";
    constexpr static const char CertificatesCreatePath[] = "create";
    constexpr static const char IssuersPath[] = "issuers";
    constexpr static const char ContactsPath[] = "contacts";
    constexpr static const char PendingPath[] = "pending";
    constexpr static const char DeletedCertificatesPath[] = "deletedcertificates";
    constexpr static const char RecoverPath[] = "recover";
    constexpr static const char PolicyPath[] = "policy";
    constexpr static const char BackupPath[] = "backup";
    constexpr static const char RestorePath[] = "restore";
    constexpr static const char IncludePendingQuery[] = "includePending";
    constexpr static const char VersionsPath[] = "versions";
    constexpr static const char ImportPath[] = "import";
    constexpr static const char MergePath[] = "merge";

    /***************** Certificates Properties *****************/
    constexpr static const char IdName[] = "id";
    constexpr static const char X5tName[] = "x5t";
    constexpr static const char TagsPropertyName[] = "tags";
    constexpr static const char AttributesPropertyName[] = "attributes";
    constexpr static const char EnabledPropertyName[] = "enabled";
    constexpr static const char NbfPropertyName[] = "nbf";
    constexpr static const char ExpPropertyName[] = "exp";
    constexpr static const char CreatedPropertyName[] = "created";
    constexpr static const char UpdatedPropertyName[] = "updated";
    constexpr static const char RecoverableDaysPropertyName[] = "recoverableDays";
    constexpr static const char RecoveryLevelPropertyName[] = "recoveryLevel";
    constexpr static const char KidPropertyName[] = "kid";
    constexpr static const char SidPropertyName[] = "sid";
    constexpr static const char CerPropertyName[] = "cer";

    /***************** Certificates Policy *****************/
    constexpr static const char PolicyPropertyName[] = "policy";
    constexpr static const char KeyPropsPropertyName[] = "key_props";
    constexpr static const char KeyTypePropertyName[] = "kty";
    constexpr static const char ReuseKeyPropertyName[] = "reuse_key";
    constexpr static const char ExportablePropertyName[] = "exportable";
    constexpr static const char CurveNamePropertyName[] = "crv";
    constexpr static const char KeySizePropertyName[] = "key_size";
    constexpr static const char SecretPropsPropertyName[] = "secret_props";
    constexpr static const char ContentTypePropertyName[] = "contentType";
    constexpr static const char X509PropsPropertyName[] = "x509_props";
    constexpr static const char SubjectPropertyName[] = "subject";
    constexpr static const char SansPropertyName[] = "sans";
    constexpr static const char DnsPropertyName[] = "dns_names";
    constexpr static const char EmailsPropertyName[] = "emails";
    constexpr static const char UserPrincipalNamesPropertyName[] = "upns";
    constexpr static const char KeyUsagePropertyName[] = "key_usage";
    constexpr static const char EkusPropertyName[] = "ekus";
    constexpr static const char ValidityMonthsPropertyName[] = "validity_months";
    constexpr static const char IssuerPropertyName[] = "issuer";
    constexpr static const char CertTransparencyPropertyName[] = "cert_transparency";
    constexpr static const char CtyPropertyName[] = "cty";
    constexpr static const char IssuerNamePropertyName[] = "name";
    constexpr static const char LifetimeActionsPropertyName[] = "lifetime_actions";
    constexpr static const char TriggerPropertyName[] = "trigger";
    constexpr static const char ActionPropertyName[] = "action";
    constexpr static const char LifetimePercentagePropertyName[] = "lifetime_percentage";
    constexpr static const char DaysBeforeExpiryPropertyName[] = "days_before_expiry";
    constexpr static const char ActionTypePropertyName[] = "action_type";

    /***************** Certificates Key Usage *****************/
    constexpr static const char DigitalSignatureValue[] = "digitalSignature";
    constexpr static const char NonRepudiationValue[] = "nonRepudiation";
    constexpr static const char KeyEnciphermentValue[] = "keyEncipherment";
    constexpr static const char DataEnciphermentValue[] = "dataEncipherment";
    constexpr static const char KeyAgreementValue[] = "keyAgreement";
    constexpr static const char KeyCertSignValue[] = "keyCertSign";
    constexpr static const char CrlSignValue[] = "crlSign";
    constexpr static const char EncipherOnlyValue[] = "encipherOnly";
    constexpr static const char DecipherOnlyValue[] = "decipherOnly";

    /***************** Certificates Key Type *****************/
    constexpr static const char EcValue[] = "EC";
    constexpr static const char EcHsmValue[] = "EC-HSM";
    constexpr static const char RsaValue[] = "RSA";
    constexpr static const char RsaHsmValue[] = "RSA-HSM";

    /***************** Certificates Curve Name *****************/
    constexpr static const char P256Value[] = "P-256";
    constexpr static const char P256KValue[] = "P-256K";
    constexpr static const char P384Value[] = "P-384";
    constexpr static const char P521Value[] = "P-521";

    /***************** Certificates Content Type *****************/
    constexpr static const char Pkc12Value[] = "application/x-pkcs12";
    constexpr static const char PemValue[] = "application/x-pem-file";

    /***************** Certificates Policy Action *****************/
    constexpr static const char AutoRenewValue[] = "AutoRenew";
    constexpr static const char EmailContactsValue[] = "EmailContacts";

    /***************** Certificates Issuer Action *****************/
    constexpr static const char CredentialsPropertyValue[] = "credentials";
    constexpr static const char AccountIdValue[] = "account_id";
    constexpr static const char PwdPropertyValue[] = "pwd";
    constexpr static const char ProviderPropertyValue[] = "provider";
    constexpr static const char OrgDetailsPropertyValue[] = "org_details";
    constexpr static const char AdminDetailsPropertyValue[] = "admin_details";
    constexpr static const char FirstNamePropertyValue[] = "first_name";
    constexpr static const char LastNamePropertyValue[] = "last_name";
    constexpr static const char EmailPropertyValue[] = "email";
    constexpr static const char PhonePropertyValue[] = "phone";

    /***************** Certificates Contact *****************/
    constexpr static const char EmailPropertyName[] = "email";
    constexpr static const char NamePropertyName[] = "name";
    constexpr static const char PhonePropertyName[] = "phone";
    constexpr static const char ContactsPropertyName[] = "contacts";

    /****************** Certificate Operation Properties **********/
    constexpr static const char CsrPropertyName[] = "csr";
    constexpr static const char StatusPropertyName[] = "status";
    constexpr static const char ErrorPropertyName[] = "error";
    constexpr static const char CodePropertyName[] = "code";
    constexpr static const char RequestIdPropertyName[] = "request_id";
    constexpr static const char TargetPropertyName[] = "target";
    constexpr static const char MessagePropertyName[] = "message";
    constexpr static const char InnerErrorPropertyName[] = "innererror";
    constexpr static const char StatusDetailsPropertyName[] = "status_details";
    constexpr static const char CancelationRequestedPropertyName[] = "cancellation_requested";
    constexpr static const char CompletedValue[] = "completed";
    constexpr static const char DeletedValue[] = "deleted";

    /**************** Deleted Certificate property names ********/
    constexpr static const char RecoveryIdPropertyName[] = "recoveryId";
    constexpr static const char ScheduledPurgeDatePropertyName[] = "scheduledPurgeDate";
    constexpr static const char DeletedDatePropertyName[] = "deletedDate";

    /****************** Certificate Backup/Restore Properties **********/
    constexpr static const char ValuePropertyName[] = "value";

    /****************** Paged Properties **********/
    constexpr static const char TrueQueryValue[] = "true";
    constexpr static const char FalseQueryValue[] = "false";
    constexpr static const char NextLinkPropertyName[] = "nextLink";

    /****************** Certificate Backup/Restore Properties **********/
    constexpr static const char X5cPropertyName[] = "x5c";
}}}}} // namespace Azure::Security::KeyVault::Certificates::_detail

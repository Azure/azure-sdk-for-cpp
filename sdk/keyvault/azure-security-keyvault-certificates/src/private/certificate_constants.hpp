// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Centralize the string constants used by Key Vault Certificates Client.
 *
 */

#pragma once

namespace Azure { namespace Security { namespace KeyVault { namespace Certificates {
  namespace _detail {

    /***************** Certificates Properties *****************/
    constexpr static const char IdName[] = "id";
    constexpr static const char X5tName[] = "x5t";
    constexpr static const char TagsName[] = "tags";
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
}}}}} // namespace Azure::Security::KeyVault::Certificates::_detail

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

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the Key Vault Certificates types.
 *
 */

#pragma once

#include "azure/keyvault/certificates/dll_import_export.hpp"

#include <azure/core/context.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/nullable.hpp>
#include <azure/core/response.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Azure { namespace Security { namespace KeyVault { namespace Certificates {

  /**
   * @brief Contains identity and other basic properties of a Certificate.
   *
   */
  struct CertificateProperties
  {
    // Attributes

    /**
     * @brief Indicate when the certificate will be valid and can be used for cryptographic
     * operations.
     *
     */
    Azure::Nullable<Azure::DateTime> NotBefore;

    /**
     * @brief Indicate when the certificate will expire and cannot be used for cryptographic
     * operations.
     *
     */
    Azure::Nullable<Azure::DateTime> ExpiresOn;

    /**
     * @brief Indicate when the certificate was created.
     *
     */
    Azure::Nullable<Azure::DateTime> CreatedOn;

    /**
     * @brief Indicate when the certificate was updated.
     *
     */
    Azure::Nullable<Azure::DateTime> UpdatedOn;

    /**
     * @brief The number of days a certificate is retained before being deleted for a soft
     * delete-enabled Key Vault.
     *
     */
    Azure::Nullable<int> RecoverableDays;

    /**
     * @brief The recovery level currently in effect for keys in the Key Vault.
     *
     * @remark If Purgeable, the certificate can be permanently deleted by an authorized user;
     * otherwise, only the service can purge the keys at the end of the retention interval.
     *
     */
    Azure::Nullable<std::string> RecoveryLevel;

    // Properties

    /**
     * @brief Dictionary of tags with specific metadata about the certificate.
     *
     */
    std::unordered_map<std::string, std::string> Tags;

    /**
     * @brief The name of the certificate.
     *
     */
    std::string Name;

    /**
     * @brief The certificate identifier.
     *
     */
    std::string Id;

    /**
     * @brief The Key Vault base Url.
     *
     */
    std::string VaultUrl;

    /**
     * @brief The version of the certificate.
     *
     */
    std::string Version;

    /**
     * @brief Gets the digital thumbprint of the certificate which can be used to uniquely identify
     * it.
     *
     */
    std::vector<uint8_t> X509Thumbprint;

    /**
     * @brief Indicate whether the certificate is enabled and useable for cryptographic operations.
     *
     */
    Azure::Nullable<bool> Enabled;

    /**
     * @brief Construct a new Certificate Properties object
     *
     */
    CertificateProperties() = default;

    /**
     * @brief Construct a new Certificate Properties object
     *
     * @param name The name of the certificate.
     */
    CertificateProperties(std::string const& name) : Name(name) {}
  };

  struct KeyVaultCertificate
  {

    std::string KeyId;
    std::string SecretId;
    /**
     * @brief Additional fields for the certificate.
     *
     */
    CertificateProperties Properties;

    std::vector<uint8_t> Cer;

    /**
     * @brief Get the name of the certificate.
     *
     * @return The name of the certificate.
     */
    std::string const& Name() const { return Properties.Name; }

    KeyVaultCertificate(CertificateProperties const& properties) : Properties(properties) {}

    KeyVaultCertificate() = default;
  };

  class CertificateKeyType {
  private:
    std::string m_value;

  public:
    explicit CertificateKeyType(std::string keyType) : m_value(std::move(keyType)) {}

    CertificateKeyType() = default;

    /**
     * @brief Enables using the equal operator for JWT.
     *
     * @param other A JWT to be compared.
     */
    bool operator==(const CertificateKeyType& other) const noexcept
    {
      return m_value == other.m_value;
    }

    /**
     * @brief Return the JSON Web Token (JWT) as a string.
     *
     * @return The JWT represented as string.
     */
    std::string const& ToString() const { return m_value; }

    /**
     * @brief An Elliptic Curve Cryptographic (ECC) algorithm.
     *
     */
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificateKeyType Ec;

    /**
     * @brief An Elliptic Curve Cryptographic (ECC) algorithm backed by a Hardware Security Module
     * (HSM).
     *
     */
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificateKeyType EcHsm;

    /**
     * @brief An RSA cryptographic algorithm.
     *
     */
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificateKeyType Rsa;

    /**
     * @brief An RSA cryptographic algorithm backed by a Hardware Security Module (HSM).
     *
     */
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificateKeyType RsaHsm;

    /**
     * @brief An AES cryptographic algorithm.
     *
     */
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificateKeyType Oct;

    /**
     * @brief An AES cryptographic algorithm backed by a Hardware Security Module (HSM).
     *
     */
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificateKeyType OctHsm;
  };

  class CertificateKeyCurveName final {
  private:
    std::string m_value;

  public:
    /**
     * @brief Construct a new Key Curve Name object.
     *
     * @param value The string value of the instance.
     */
    explicit CertificateKeyCurveName(std::string value)
    {
      if (value.empty())
      {
        throw std::invalid_argument("The value for the curve name can not be empty");
      }
      m_value = std::move(value);
    }

    /**
     * @brief Construct a default key curve.
     *
     */
    CertificateKeyCurveName() = default;

    /**
     * @brief Enables using the equal operator for key curve.
     *
     * @param other A key curve to be compared.
     */
    bool operator==(const CertificateKeyCurveName& other) const noexcept
    {
      return m_value == other.m_value;
    }

    /**
     * @brief Get the string value of the key curve.
     *
     */
    std::string const& ToString() const { return m_value; }

    /**
     * @brief Gets the NIST P-256 elliptic curve, AKA SECG curve SECP256R1.
     *
     * @remark For more information, see
     *  <a href="https://docs.microsoft.com/azure/key-vault/keys/about-keys#curve-types">Curve
     * types</a>.
     *
     */
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificateKeyCurveName P256;

    /**
     * @brief Gets the SECG SECP256K1 elliptic curve.
     *
     * @remark For more information, see
     * <a href="https://docs.microsoft.com/azure/key-vault/keys/about-keys#curve-types">Curve
     * types</a>.
     *
     */
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificateKeyCurveName P256K;

    /**
     * @brief Gets the NIST P-384 elliptic curve, AKA SECG curve SECP384R1.
     *
     * @remark For more information, see
     * <a href="https://docs.microsoft.com/azure/key-vault/keys/about-keys#curve-types">Curve
     * types</a>.
     *
     */
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificateKeyCurveName P384;

    /**
     * @brief Gets the NIST P-521 elliptic curve, AKA SECG curve SECP521R1.
     *
     * @remark For more information, see
     * <a href="https://docs.microsoft.com/azure/key-vault/keys/about-keys#curve-types">Curve
     * types</a>.
     *
     */
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificateKeyCurveName P521;
  };

  struct SubjectAlternativeNameList
  {
    std::vector<std::string> DnsNames;
    std::vector<std::string> Emails;
    std::vector<std::string> UserPrincipalNames;
  };

  class CertificateContentType final {
  private:
    std::string m_value;

  public:
    /**
     * @brief Construct a new Key Curve Name object.
     *
     * @param value The string value of the instance.
     */
    explicit CertificateContentType(std::string value)
    {
      if (value.empty())
      {
        throw std::invalid_argument("The value for the curve name can not be empty");
      }
      m_value = std::move(value);
    }

    /**
     * @brief Construct a default key curve.
     *
     */
    CertificateContentType() = default;

    /**
     * @brief Enables using the equal operator for key curve.
     *
     * @param other A key curve to be compared.
     */
    bool operator==(const CertificateContentType& other) const noexcept
    {
      return m_value == other.m_value;
    }

    /**
     * @brief Get the string value of the key curve.
     *
     */
    std::string const& ToString() const { return m_value; }

    /**
     * @brief Gets the NIST P-256 elliptic curve, AKA SECG curve SECP256R1.
     *
     * @remark For more information, see
     *  <a href="https://docs.microsoft.com/azure/key-vault/keys/about-keys#curve-types">Curve
     * types</a>.
     *
     */
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificateContentType Pkcs12;

    /**
     * @brief Gets the SECG SECP256K1 elliptic curve.
     *
     * @remark For more information, see
     * <a href="https://docs.microsoft.com/azure/key-vault/keys/about-keys#curve-types">Curve
     * types</a>.
     *
     */
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificateContentType Pem;
  };

  class CertificateKeyUsage final {
  private:
    std::string m_value;

  public:
    /**
     * @brief Construct a new Key Curve Name object.
     *
     * @param value The string value of the instance.
     */
    explicit CertificateKeyUsage(std::string value)
    {
      if (value.empty())
      {
        throw std::invalid_argument("The value for the curve name can not be empty");
      }
      m_value = std::move(value);
    }

    /**
     * @brief Construct a default key curve.
     *
     */
    CertificateKeyUsage() = default;

    /**
     * @brief Enables using the equal operator for key curve.
     *
     * @param other A key curve to be compared.
     */
    bool operator==(const CertificateKeyUsage& other) const noexcept
    {
      return m_value == other.m_value;
    }

    /**
     * @brief Get the string value of the key curve.
     *
     */
    std::string const& ToString() const { return m_value; }

    /**
     * @brief Gets the NIST P-256 elliptic curve, AKA SECG curve SECP256R1.
     *
     * @remark For more information, see
     *  <a href="https://docs.microsoft.com/azure/key-vault/keys/about-keys#curve-types">Curve
     * types</a>.
     *
     */
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificateKeyUsage DigitalSignature;

    /**
     * @brief Gets the SECG SECP256K1 elliptic curve.
     *
     * @remark For more information, see
     * <a href="https://docs.microsoft.com/azure/key-vault/keys/about-keys#curve-types">Curve
     * types</a>.
     *
     */
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificateKeyUsage AutoRenew;
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificateKeyUsage EmailContacts;
  };

  class CertificatePolicyAction final {
  private:
    std::string m_value;

  public:
    /**
     * @brief Construct a new Key Curve Name object.
     *
     * @param value The string value of the instance.
     */
    explicit CertificatePolicyAction(std::string value)
    {
      if (value.empty())
      {
        throw std::invalid_argument("The value for the curve name can not be empty");
      }
      m_value = std::move(value);
    }

    /**
     * @brief Construct a default key curve.
     *
     */
    CertificatePolicyAction() = default;

    /**
     * @brief Enables using the equal operator for key curve.
     *
     * @param other A key curve to be compared.
     */
    bool operator==(const CertificatePolicyAction& other) const noexcept
    {
      return m_value == other.m_value;
    }

    /**
     * @brief Get the string value of the key curve.
     *
     */
    std::string const& ToString() const { return m_value; }

    /**
     * @brief Gets the NIST P-256 elliptic curve, AKA SECG curve SECP256R1.
     *
     * @remark For more information, see
     *  <a href="https://docs.microsoft.com/azure/key-vault/keys/about-keys#curve-types">Curve
     * types</a>.
     *
     */
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificatePolicyAction
        DigitalSignature;

    /**
     * @brief Gets the SECG SECP256K1 elliptic curve.
     *
     * @remark For more information, see
     * <a href="https://docs.microsoft.com/azure/key-vault/keys/about-keys#curve-types">Curve
     * types</a>.
     *
     */
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificatePolicyAction NonRepudiation;
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificatePolicyAction
        KeyEncipherment;
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificatePolicyAction
        DataEncipherment;
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificatePolicyAction KeyAgreement;
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificatePolicyAction KeyCertSign;
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificatePolicyAction CrlSign;
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificatePolicyAction EncipherOnly;
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificatePolicyAction DecipherOnly;
  };

  struct LifetimeAction
  {
    CertificatePolicyAction Action;
    Azure::Nullable<int32_t> DaysBeforeExpiry;
    Azure::Nullable<int32_t> LifetimePercentage;
  };

  struct CertificatePolicy {
    Azure::Nullable<CertificateKeyType> KeyType;
    Azure::Nullable<bool> ReuseKey;
    Azure::Nullable<bool> Exportable;
    Azure::Nullable<CertificateKeyCurveName> KeyCurveName;
    Azure::Nullable<int32_t> KeySize;
    std::string Subject;
    SubjectAlternativeNameList SubjectAlternativeNames;
    std::string IssuerName;
    Azure::Nullable<CertificateContentType> ContentType;
    std::string CertificateType;
    Azure::Nullable<bool> CertificateTransparency;
    Azure::Nullable<int32_t> ValidityInMonths;
    Azure::Nullable<bool> Enabled;
    Azure::Nullable<Azure::DateTime> UpdatedOn;
    Azure::Nullable<Azure::DateTime> CreatedOn;
    std::vector<CertificateKeyUsage> KeyUsage;
    std::vector<std::string> EnhancedKeyUsage;
    std::vector<LifetimeAction> LifetimeActions;
  };

  struct KeyVaultCertificateWithPolicy : public KeyVaultCertificate
  {
    CertificatePolicy Policy;

    KeyVaultCertificateWithPolicy(CertificateProperties const& properties)
        : KeyVaultCertificate(properties)
    {
    }
  };
}}}} // namespace Azure::Security::KeyVault::Certificates

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
  struct CertificateProperties final
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
    Azure::Nullable<int32_t> RecoverableDays;

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
     * @brief Get the digital thumbprint of the certificate which can be used to uniquely identify
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

  /**
   * @brief An Azure Key Vault certificate.
   *
   */
  class KeyVaultCertificate {
  public:
    /**
     * @brief Get the identifier of the certificate.
     *
     */
    std::string KeyId;

    /**
     * @brief Get the identifier of the Key Vault Secret which contains the PEM of PFX formatted
     * content of the certificate and its private key.
     *
     */
    std::string SecretId;

    /**
     * @brief Additional fields for the certificate.
     *
     */
    CertificateProperties Properties;

    /**
     * @brief Get the CER formatted public X509 certificate.
     *
     * @remarks This property contains only the public key.
     *
     */
    std::vector<uint8_t> Cer;

    /**
     * @brief Get the name of the certificate.
     *
     * @return The name of the certificate.
     */
    std::string const& Name() const { return Properties.Name; }

    /**
     * @brief Get the certificate Id,
     *
     * @return The id of the certificate.
     */
    std::string const& Id() const { return Properties.Id; }

    /**
     * @brief Construct a new Key Vault Certificate object
     *
     * @param properties The properties to create a new certificate.
     */
    KeyVaultCertificate(CertificateProperties const& properties) : Properties(properties) {}

    /**
     * @brief Construct a new Key Vault Certificate object
     *
     */
    KeyVaultCertificate() = default;

    /**
     * @brief Destroy the Key Vault Certificate object
     *
     */
    virtual ~KeyVaultCertificate() = default;
  };

  /**
   * @brief Supported JsonWebKey key types (kty).
   *
   */
  class CertificateKeyType final {
  private:
    std::string m_value;

  public:
    /**
     * @brief Construct a new Certificate Key Type object
     *
     * @param keyType The type of the certificate.
     */
    explicit CertificateKeyType(std::string keyType) : m_value(std::move(keyType)) {}

    /**
     * @brief Construct a new Certificate Key Type object
     *
     */
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
  };

  /**
   * @brief Elliptic Curve Cryptography (ECC) curve names.
   *
   */
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
     * @brief Get the NIST P-256 elliptic curve, AKA SECG curve SECP256R1.
     *
     * @remark For more information, see
     *  <a href="https://docs.microsoft.com/azure/key-vault/keys/about-keys#curve-types">Curve
     * types</a>.
     *
     */
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificateKeyCurveName P256;

    /**
     * @brief Get the SECG SECP256K1 elliptic curve.
     *
     * @remark For more information, see
     * <a href="https://docs.microsoft.com/azure/key-vault/keys/about-keys#curve-types">Curve
     * types</a>.
     *
     */
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificateKeyCurveName P256K;

    /**
     * @brief Get the NIST P-384 elliptic curve, AKA SECG curve SECP384R1.
     *
     * @remark For more information, see
     * <a href="https://docs.microsoft.com/azure/key-vault/keys/about-keys#curve-types">Curve
     * types</a>.
     *
     */
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificateKeyCurveName P384;

    /**
     * @brief Get the NIST P-521 elliptic curve, AKA SECG curve SECP521R1.
     *
     * @remark For more information, see
     * <a href="https://docs.microsoft.com/azure/key-vault/keys/about-keys#curve-types">Curve
     * types</a>.
     *
     */
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificateKeyCurveName P521;
  };

  /**
   * @brief A collection of subject alternative names (SANs) for a X.509 certificate. SANs can be
   * DNS entries, emails, or unique principal names.
   *
   */
  struct SubjectAlternativeNames final
  {
    /**
     * @brief Get a collection of DNS names.
     *
     */
    std::vector<std::string> DnsNames;

    /**
     * @brief Get a collection of email addresses.
     *
     */
    std::vector<std::string> Emails;

    /**
     * @brief Get a collection of user principal names (UPNs).
     *
     */
    std::vector<std::string> UserPrincipalNames;
  };

  /**
   * @brief Content type of the certificate when downloaded from getecret.
   *
   */
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
     * @brief Get the NIST P-256 elliptic curve, AKA SECG curve SECP256R1.
     *
     * @remark For more information, see
     *  <a href="https://docs.microsoft.com/azure/key-vault/keys/about-keys#curve-types">Curve
     * types</a>.
     *
     */
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificateContentType Pkcs12;

    /**
     * @brief Get the SECG SECP256K1 elliptic curve.
     *
     * @remark For more information, see
     * <a href="https://docs.microsoft.com/azure/key-vault/keys/about-keys#curve-types">Curve
     * types</a>.
     *
     */
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificateContentType Pem;
  };

  /**
   * @brief Supported usages of a certificate key.
   *
   */
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
     * @brief Get a CertificateKeyUsage indicating that the certificate key can be
     * used as a digital signatures.
     *
     */
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificateKeyUsage DigitalSignature;

    /**
     * @brief Get a CertificateKeyUsage indicating that the certificate key can be used for
     * authentication.
     *
     */
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificateKeyUsage NonRepudiation;

    /**
     * @brief Get a CertificateKeyUsage indicating that the certificate key can be used for key
     * encryption.
     *
     */
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificateKeyUsage KeyEncipherment;

    /**
     * @brief Get a CertificateKeyUsage indicating that the certificate key can be used for data
     * encryption.
     *
     */
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificateKeyUsage DataEncipherment;

    /**
     * @brief Get a CertificateKeyUsage indicating that the certificate key can be used to determine
     * key agreement, such as a key created using the Diffie-Hellman key agreement algorithm.
     *
     */
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificateKeyUsage KeyAgreement;

    /**
     * @brief Get a CertificateKeyUsage indicating that the certificate key can be used to sign
     * certificates.
     *
     */
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificateKeyUsage KeyCertSign;

    /**
     * @brief Get a CertificateKeyUsage indicating that the certificate key can be used to sign a
     * certificate revocation list.
     *
     */
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificateKeyUsage CrlSign;

    /**
     * @brief Get a CertificateKeyUsage indicating that the certificate key can be used for
     * encryption only.
     *
     */
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificateKeyUsage EncipherOnly;

    /**
     * @brief Get a CertificateKeyUsage indicating that the certificate key can be used for
     * decryption only.
     *
     */
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificateKeyUsage DecipherOnly;
  };

  /**
   * @brief An action that will be executed.
   *
   */
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
     * @brief Get the NIST P-256 elliptic curve, AKA SECG curve SECP256R1.
     *
     * @remark For more information, see
     *  <a href="https://docs.microsoft.com/azure/key-vault/keys/about-keys#curve-types">Curve
     * types</a>.
     *
     */
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificatePolicyAction
        DigitalSignature;

    /**
     * @brief Gets a CertificatePolicyAction that will auto-renew a certificate.
     *
     */
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificatePolicyAction AutoRenew;

    /**
     * @brief Get a CertificatePolicyAction action that will email certificate contacts.
     *
     */
    AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT static const CertificatePolicyAction EmailContacts;
  };

  /**
   * @brief An action to be executed at a prescribed time in a certificates lifecycle.
   *
   */
  struct LifetimeAction final
  {
    /**
     * @brief Get the CertificatePolicyAction to be performed.
     *
     */
    CertificatePolicyAction Action;

    /**
     * @brief Get the action should be performed the specified number of days before the certificate
     * will expire.
     *
     */
    Azure::Nullable<int32_t> DaysBeforeExpiry;

    /**
     * @brief Get the action should be performed when the certificate reaches the specified
     * percentage of its lifetime. Valid values include 1-99.
     *
     */
    Azure::Nullable<int32_t> LifetimePercentage;
  };

  /**
   * @brief A policy which governs the lifecycle a properties of a certificate managed by Azure Key
   * Vault.
   *
   */
  struct CertificatePolicy final
  {
    /**
     * @brief Get the type of backing key to be generated when issuing new certificates.
     *
     */
    Azure::Nullable<CertificateKeyType> KeyType;

    /**
     * @brief Get a value indicating whether the certificate key should be reused when rotating the
     * certificate.
     *
     */
    Azure::Nullable<bool> ReuseKey;

    /**
     * @brief Get a value indicating whether the certificate key is exportable from the vault or
     * secure certificate store.
     *
     */
    Azure::Nullable<bool> Exportable;

    /**
     * @brief Get the curve which back an Elliptic Curve (EC) key.
     *
     */
    Azure::Nullable<CertificateKeyCurveName> KeyCurveName;

    /**
     * @brief Get the size of the RSA key. The value must be a valid RSA key length such as 2048 or
     * 4092.
     *
     */
    Azure::Nullable<int32_t> KeySize;

    /**
     * @brief Get the subject name of a certificate.
     *
     */
    std::string Subject;

    /**
     * @brief Get the subject alternative names (SANs) of a certificate.
     *
     */
    struct SubjectAlternativeNames SubjectAlternativeNames;

    /**
     * @brief Indicates if the certificates generated under this policy should be published to
     * certificate transparency logs.
     *
     */
    Azure::Nullable<bool> CertificateTransparency;

    /**
     * @brief Certificate type as supported by the provider (optional); for example 'OV-SSL',
     * 'EV-SSL'.
     *
     */
    Azure::Nullable<std::string> CertificateType;

    /**
     * @brief Name of the referenced issuer object or reserved names; for example, 'Self' or
     'Unknown'.
     *
     */
    Azure::Nullable<std::string> IssuerName;

    /**
     * @brief Get the CertificateContentType of the certificate.
     *
     */
    Azure::Nullable<CertificateContentType> ContentType;

    /**
     * @brief Get the validity period for a certificate in months.
     *
     */
    Azure::Nullable<int32_t> ValidityInMonths;

    /**
     * @brief Get a value indicating whether the certificate is currently enabled. If null, the
     * server default will be used.
     *
     */
    Azure::Nullable<bool> Enabled;

    /**
     * @brief Get a DateTime indicating when the certificate was updated.
     *
     */
    Azure::Nullable<Azure::DateTime> UpdatedOn;

    /**
     * @brief Get a DateTime indicating when the certificate was created.
     *
     */
    Azure::Nullable<Azure::DateTime> CreatedOn;

    /**
     * @brief Gets the allowed usages for the key of the certificate.
     *
     */
    std::vector<CertificateKeyUsage> KeyUsage;

    /**
     * @brief Get the allowed enhanced key usages (EKUs) of the certificate.
     *
     */
    std::vector<std::string> EnhancedKeyUsage;

    /**
     * @brief Get the actions to be executed at specified times in the certificates lifetime.
     *
     */
    std::vector<LifetimeAction> LifetimeActions;
  };

  /**
   * @brief A KeyVaultCertificate along with its CertificatePolicy.
   *
   */
  struct KeyVaultCertificateWithPolicy final : public KeyVaultCertificate
  {
    /**
     * @brief Gets the current policy for the certificate.
     *
     */
    CertificatePolicy Policy;

    /**
     * @brief Construct a new Key Vault Certificate With Policy object
     *
     * @param properties The properties to create a new certificate.
     */
    KeyVaultCertificateWithPolicy(CertificateProperties const& properties)
        : KeyVaultCertificate(properties)
    {
    }
  };

  /**
   * @brief The options for calling GetCertificate.
   *
   */
  struct GetCertificateOptions final
  {
    std::string Version;
  };

  /**
   * @brief Parameters for StartCreateCertificate.
   *
   */
  struct CertificateCreateParameters final
  {
    /**
     * @brief Certificate policy.
     *
     */
    CertificatePolicy Policy;

    /**
     * @brief Certificate attributes.
     *
     */
    CertificateProperties Properties;

    /**
     * @brief Respresents if certificate is enabled.
     *
     */
    Azure::Nullable<bool> Enabled() { return Properties.Enabled; }

    /**
     * @brief Certificate tags.
     *
     */
    std::unordered_map<std::string, std::string> Tags;
  };

  /**
   * @brief Issuer Credentials
   *
   */
  struct IssuerCredentials final
  {
    /**
     * @brief Account ID.
     *
     */
    Azure::Nullable<std::string> AccountId;

    /**
     * @brief Password.
     *
     */
    Azure::Nullable<std::string> Password;
  };

  /**
   * @brief Administrator details
   *
   */
  struct AdministratorDetails final
  {
    /**
     * @brief Administrator first name.
     *
     */
    Azure::Nullable<std::string> FirstName;

    /**
     * @brief Administrator last name.
     *
     */
    Azure::Nullable<std::string> LastName;

    /**
     * @brief Administrator email address.
     *
     */
    Azure::Nullable<std::string> EmailAddress;

    /**
     * @brief Administrator phone number.
     *
     */
    Azure::Nullable<std::string> PhoneNumber;
  };

  /**
   * @brief Certificate Issuer Properties.
   *
   */
  struct IssuerProperties final
  {
    /**
     * @brief Issuer enabled.
     *
     */
    Azure::Nullable<bool> Enabled;

    /**
     * @brief Issuer creation date.
     *
     */
    Azure::Nullable<DateTime> Created;

    /**
     * @brief Issuer last update date.
     *
     */
    Azure::Nullable<DateTime> Updated;
  };

  /**
   * @brief Organization details.
   *
   */
  struct OrganizationDetails final
  {
    /**
     * @brief Organization id
     *
     */
    Azure::Nullable<std::string> Id;

    /**
     * @brief Organization Administators collection.
     *
     */
    std::vector<AdministratorDetails> AdminDetails;
  };

  /**
   * @brief Certificate issuer.
   *
   */
  struct CertificateIssuer final
  {
    /**
     * @brief Certificate issuer name.
     *
     */
    std::string Name;

    /**
     * @brief  Certificate issuer id.
     *
     */
    Azure::Nullable<std::string> Id;

    /**
     * @brief Certificate issuer provider.
     *
     */
    Azure::Nullable<std::string> Provider;

    /**
     * @brief Certificate issuer credentials.
     *
     */
    IssuerCredentials Credentials;

    /**
     * @brief Certificate issuer organization.
     *
     */
    OrganizationDetails Organization;

    /**
     * @brief Certificate issuer properties.
     *
     */
    IssuerProperties Properties;
  };

  /**
   * @brief The contact information for the vault certificates.
   *
   */
  struct CertificateContact
  {
    /**
     * @brief Contact e-mail address.
     *
     */
    std::string EmailAddress;

    /**
     * @brief Contact name.
     *
     */
    Azure::Nullable<std::string> Name;

    /**
     * @brief Contact phone number.
     *
     */
    Azure::Nullable<std::string> Phone;
  };
}}}} // namespace Azure::Security::KeyVault::Certificates

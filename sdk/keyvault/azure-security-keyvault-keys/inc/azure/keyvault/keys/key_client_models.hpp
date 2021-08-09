// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the JsonWebKey types.
 *
 */

#pragma once

#include "azure/keyvault/keys/dll_import_export.hpp"

#include <azure/core/context.hpp>
#include <azure/core/datetime.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/nullable.hpp>
#include <azure/core/operation.hpp>
#include <azure/core/operation_status.hpp>
#include <azure/core/paged_response.hpp>
#include <azure/core/response.hpp>

#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  /**
   * @brief Define a model for a purged key.
   *
   */
  struct PurgedKey final
  {
  };

  /**
   * @brief The BackupKeyResult type containing the backup key of bytes.
   *
   */
  struct BackupKeyResult final
  {
    /**
     * @brief The backup key array data.
     *
     */
    std::vector<uint8_t> BackupKey;
  };

  /**
   * @brief An operation that can be performed with the key.
   *
   */
  class KeyOperation final {
  private:
    std::string m_operation;

  public:
    /**
     * @brief Construct a new Key Operation object.
     *
     * @param operation The operation for the key as string.
     */
    explicit KeyOperation(std::string operation) : m_operation(std::move(operation)) {}

    /**
     * @brief Construct a default Key operation.
     *
     */
    KeyOperation() = default;

    /**
     * @brief Enables using the equal operator for key operations.
     *
     * @param other A key operation to be compared.
     */
    bool operator==(const KeyOperation& other) const noexcept
    {
      return m_operation == other.m_operation;
    }

    /**
     * @brief Returns the fully qualified type name of this instance.
     *
     * @return The operation represented as string.
     */
    std::string const& ToString() const { return m_operation; }

    /**
     * @brief The key can be used to encrypt with the #Encrypt(EncryptionAlgorithm, Byte[],
     * CancellationToken) method.
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyOperation Encrypt;

    /**
     * @brief The key can be used to decrypt with the #Decrypt(EncryptionAlgorithm, Byte[],
     * CancellationToken) method.
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyOperation Decrypt;

    /**
     * @brief The key can be used to sign with the Sign(SignatureAlgorithm, Byte[],
     * CancellationToken) method.
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyOperation Sign;

    /**
     * @brief The key can be used to verify with the Verify(SignatureAlgorithm, Byte[], Byte[],
     * CancellationToken) method.
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyOperation Verify;

    /**
     * @brief The key can be used to wrap another key with the WrapKey(KeyWrapAlgorithm, Byte[],
     * CancellationToken) method.
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyOperation WrapKey;

    /**
     * @brief The key can be used to unwrap another key with the UnwrapKey(KeyWrapAlgorithm, Byte[],
     * CancellationToken) method.
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyOperation UnwrapKey;

    /**
     * @brief The key can be imported during creation using the ImportKey(ImportKeyOptions,
     * CancellationToken) method.
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyOperation Import;
  };

  /**
   * @brief The JsonWebKey types.
   *
   */
  class KeyVaultKeyType final {
  private:
    std::string m_value;

  public:
    /**
     * @brief Construct a new JSON Web Token (JWT) type object.
     *
     * @param jwt The JWT as a string.
     */
    explicit KeyVaultKeyType(std::string jwt) : m_value(std::move(jwt)) {}

    /**
     * @brief Construct a default KeyVaultKeyType with an empty string.
     *
     */
    KeyVaultKeyType() = default;

    /**
     * @brief Enables using the equal operator for JWT.
     *
     * @param other A JWT to be compared.
     */
    bool operator==(const KeyVaultKeyType& other) const noexcept
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
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyVaultKeyType Ec;

    /**
     * @brief An Elliptic Curve Cryptographic (ECC) algorithm backed by a Hardware Security Module
     * (HSM).
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyVaultKeyType EcHsm;

    /**
     * @brief An RSA cryptographic algorithm.
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyVaultKeyType Rsa;

    /**
     * @brief An RSA cryptographic algorithm backed by a Hardware Security Module (HSM).
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyVaultKeyType RsaHsm;

    /**
     * @brief An AES cryptographic algorithm.
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyVaultKeyType Oct;

    /**
     * @brief An AES cryptographic algorithm backed by a Hardware Security Module (HSM).
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyVaultKeyType OctHsm;
  };

  /**
   * @brief Elliptic Curve Cryptography (ECC) curve names.
   *
   */
  class KeyCurveName final {
  private:
    std::string m_value;

  public:
    /**
     * @brief Construct a new Key Curve Name object.
     *
     * @param value The string value of the instance.
     */
    explicit KeyCurveName(std::string value)
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
    KeyCurveName() = default;

    /**
     * @brief Enables using the equal operator for key curve.
     *
     * @param other A key curve to be compared.
     */
    bool operator==(const KeyCurveName& other) const noexcept { return m_value == other.m_value; }

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
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyCurveName P256;

    /**
     * @brief Gets the SECG SECP256K1 elliptic curve.
     *
     * @remark For more information, see
     * <a href="https://docs.microsoft.com/azure/key-vault/keys/about-keys#curve-types">Curve
     * types</a>.
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyCurveName P256K;

    /**
     * @brief Gets the NIST P-384 elliptic curve, AKA SECG curve SECP384R1.
     *
     * @remark For more information, see
     * <a href="https://docs.microsoft.com/azure/key-vault/keys/about-keys#curve-types">Curve
     * types</a>.
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyCurveName P384;

    /**
     * @brief Gets the NIST P-521 elliptic curve, AKA SECG curve SECP521R1.
     *
     * @remark For more information, see
     * <a href="https://docs.microsoft.com/azure/key-vault/keys/about-keys#curve-types">Curve
     * types</a>.
     *
     */
    AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT static const KeyCurveName P521;
  };

  /**
   * @brief Represents a JSON Web Key as defined in http://tools.ietf.org/html/rfc7517.
   *
   */
  class JsonWebKey final {
  public:
    /**
     * @brief The Identifier of the key. This is not limited to a Url.
     *
     */
    std::string Id;

    /**
     * @brief They type of the key.
     *
     */
    KeyVaultKeyType KeyType;

    /**
     * @brief Construct a new Json Web Key object.
     *
     */
    JsonWebKey() = default;

    /**
     * @brief Set the Key Operations object based on a list of operations.
     *
     * @param keyOperations The list of key operations.
     */
    void SetKeyOperations(std::vector<KeyOperation> const& keyOperations)
    {
      m_keyOps = keyOperations;
    }

    /**
     * @brief Get the list of operations from the JsonWebKey.
     *
     * @return std::vector<KeyOperation> const&
     */
    std::vector<KeyOperation> const& KeyOperations() const { return m_keyOps; }

    /**
     * @brief Gets or sets the elliptic curve name.
     *
     * @remark See #KeyCurveName for possible values.
     *
     * @remark If null, the service default is used.
     *
     */
    Azure::Nullable<KeyCurveName> CurveName;

    /**** RSA fields ****/

    /// The RSA modulus.
    std::vector<uint8_t> N;
    /// The RSA public exponent.
    std::vector<uint8_t> E;
    /// The RSA private key parameter.
    std::vector<uint8_t> DP;
    /// The RSA private key parameter.
    std::vector<uint8_t> DQ;
    /// The RSA private key parameter.
    std::vector<uint8_t> QI;
    /// The RSA secret prime.
    std::vector<uint8_t> P;
    /// The RSA secret prime.
    std::vector<uint8_t> Q;

    /// The RSA private exponent or EC private key.
    std::vector<uint8_t> D;

    /// Gets the symmetric key.
    std::vector<uint8_t> K;
    /// Gets the protected key used with "Bring Your Own Key".
    std::vector<uint8_t> T;
    /// Gets the X coordinate of the elliptic curve point.
    std::vector<uint8_t> X;
    /// Gets the Y coordinate for the elliptic curve point.
    std::vector<uint8_t> Y;

    bool SupportsOperation(KeyOperation operation) const
    {
      for (auto supportedOperation : m_keyOps)
      {
        if (operation == supportedOperation)
        {
          return true;
        }
      }
      return false;
    }

  private:
    std::vector<KeyOperation> m_keyOps;
  };

  /**
   * @brief The resource containing all the properties of the KeyVaultKey except JsonWebKey
   * properties.
   *
   */
  struct KeyProperties final
  {
    /**
     * @brief The name of the key.
     *
     */
    std::string Name;

    /**
     * @brief The key identifier.
     *
     */
    std::string Id;

    /**
     * @brief The Key Vault base Url.
     *
     */
    std::string VaultUrl;

    /**
     * @brief The version of the key.
     *
     */
    std::string Version;

    /**
     * @brief Indicate whether the key's lifetime is managed by Key Vault. If this key is backing a
     * Key Vault certificate, the value will be true.
     *
     */
    bool Managed = false;

    /**
     * @brief Dictionary of tags with specific metadata about the key.
     *
     */
    std::unordered_map<std::string, std::string> Tags;

    /**
     * @brief Indicate whether the key is enabled and useable for cryptographic operations.
     *
     */
    Azure::Nullable<bool> Enabled;

    /**
     * @brief Indicate when the key will be valid and can be used for cryptographic operations.
     *
     */
    Azure::Nullable<Azure::DateTime> NotBefore;

    /**
     * @brief Indicate when the key will expire and cannot be used for cryptographic operations.
     *
     */
    Azure::Nullable<Azure::DateTime> ExpiresOn;

    /**
     * @brief Indicate when the key was created.
     *
     */
    Azure::Nullable<Azure::DateTime> CreatedOn;

    /**
     * @brief Indicate when the key was updated.
     *
     */
    Azure::Nullable<Azure::DateTime> UpdatedOn;

    /**
     * @brief The number of days a key is retained before being deleted for a soft delete-enabled
     * Key Vault.
     *
     */
    Azure::Nullable<int> RecoverableDays;

    /**
     * @brief The recovery level currently in effect for keys in the Key Vault.
     *
     * @remark If Purgeable, the key can be permanently deleted by an authorized user; otherwise,
     * only the service can purge the keys at the end of the retention interval.
     *
     */
    std::string RecoveryLevel;

    /**
     * @brief Construct a new Key Properties object.
     *
     */
    KeyProperties() = default;

    /**
     * @brief Construct a new Key Properties object.
     *
     * @param name The name of the key.
     */
    KeyProperties(std::string name) : Name(std::move(name)) {}
  };

  /**
   * @brief A key resource and its properties.
   *
   */
  struct KeyVaultKey
  {
    /**
     * @brief Destructor.
     *
     */
    virtual ~KeyVaultKey() = default;

    /**
     * @brief The cryptographic key, the key type, and the operations you can perform using the key.
     *
     */
    JsonWebKey Key;

    /**
     * @brief The additional properties.
     *
     */
    KeyProperties Properties;

    /**
     * @brief Construct an empty Key.
     *
     */
    KeyVaultKey() = default;

    /**
     * @brief Construct a new Key Vault Key object.
     *
     * @param name The name of the key.
     */
    KeyVaultKey(std::string name) : Properties(std::move(name)) {}

    /**
     * @brief Get the Key identifier.
     *
     * @return The key ID.
     */
    std::string const& Id() const { return Key.Id; }

    /**
     * @brief Gets the name of the Key.
     *
     * @return The name of the key.
     */
    std::string const& Name() const { return Properties.Name; }

    /**
     * @brief Get the Key Type.
     *
     * @return The type of the key.
     */
    KeyVaultKeyType const& GetKeyType() const { return Key.KeyType; }

    /**
     * @brief Gets the operations you can perform using the key.
     *
     * @return A vector with the supported operations for the key.
     */
    std::vector<KeyOperation> const& KeyOperations() const { return Key.KeyOperations(); }
  };

  /**
   * @brief Represents a Key Vault key that has been deleted, allowing it to be recovered, if
   * needed.
   *
   */
  struct DeletedKey final : public KeyVaultKey
  {
    /**
     * @brief A recovery URL that can be used to recover it.
     *
     */
    std::string RecoveryId;

    /**
     * @brief Construct an empty DeletedKey
     *
     */
    DeletedKey() = default;

    /**
     * @brief Construct a new Deleted Key object.
     *
     * @param name The name of the deleted key.
     */
    DeletedKey(std::string name) : KeyVaultKey(name) {}

    /**
     * @brief Indicate when the key was deleted.
     *
     */
    Azure::DateTime DeletedDate;

    /**
     * @brief Indicate when the deleted key will be purged.
     *
     */
    Azure::DateTime ScheduledPurgeDate;
  };

  class KeyClient;

  /**
   * @brief Define a single page to list the keys from the Key Vault.
   *
   */
  class KeyPropertiesPagedResponse final
      : public Azure::Core::PagedResponse<KeyPropertiesPagedResponse> {
  private:
    friend class KeyClient;
    friend class Azure::Core::PagedResponse<KeyPropertiesPagedResponse>;

    std::string m_keyName;
    std::shared_ptr<KeyClient> m_keyClient;
    void OnNextPage(const Azure::Core::Context&);

    /**
     * @brief Construct a new Key Properties Single Page object.
     *
     * @remark The constructor is private and only a key client or PagedResponse can init this.
     *
     * @param keyProperties A previously created #KeyPropertiesPageResponse that is used to init
     * this instance.
     * @param rawResponse The HTTP raw response from where the #KeyPropertiesPagedResponse was
     * parsed.
     * @param keyClient A key client required for getting the next pages.
     * @param keyName When \p keyName is set, the response is listing key versions. Otherwise, the
     * response is for listing keys from the Key Vault.
     */
    KeyPropertiesPagedResponse(
        KeyPropertiesPagedResponse&& keyProperties,
        std::unique_ptr<Azure::Core::Http::RawResponse> rawResponse,
        std::shared_ptr<KeyClient> keyClient,
        std::string const& keyName = std::string())
        : PagedResponse(std::move(keyProperties)), m_keyName(keyName), m_keyClient(keyClient),
          Items(std::move(keyProperties.Items))
    {
      RawResponse = std::move(rawResponse);
    }

  public:
    /**
     * @brief Construct a new key properties object.
     *
     */
    KeyPropertiesPagedResponse() = default;

    /**
     * @brief Each #KeyProperties represent a Key in the Key Vault.
     *
     */
    std::vector<KeyProperties> Items;
  };

  /**
   * @brief Define a single page containing the deleted keys from the Key Vault.
   *
   */
  class DeletedKeyPagedResponse final : public Azure::Core::PagedResponse<DeletedKeyPagedResponse> {
  private:
    friend class KeyClient;
    friend class Azure::Core::PagedResponse<DeletedKeyPagedResponse>;

    std::shared_ptr<KeyClient> m_keyClient;
    void OnNextPage(const Azure::Core::Context& context);

    /**
     * @brief Construct a new Key Properties Single Page object.
     *
     * @remark The constructor is private and only a key client or PagedResponse can init this.
     *
     * @param deletedKeyProperties A previously created #DeletedKeyPagedResponse that is used to
     * init this new instance.
     * @param rawResponse The HTTP raw response from where the #DeletedKeyPagedResponse was parsed.
     * @param keyClient A key client required for getting the next pages.
     */
    DeletedKeyPagedResponse(
        DeletedKeyPagedResponse&& deletedKeyProperties,
        std::unique_ptr<Azure::Core::Http::RawResponse> rawResponse,
        std::shared_ptr<KeyClient> keyClient)
        : PagedResponse(std::move(deletedKeyProperties)), m_keyClient(keyClient),
          Items(std::move(deletedKeyProperties.Items))
    {
      RawResponse = std::move(rawResponse);
    }

  public:
    /**
     * @brief Construct a new Deleted Key Single Page object
     *
     */
    DeletedKeyPagedResponse() = default;

    /**
     * @brief Each #DeletedKey represent a deleted key in the Key Vault.
     *
     */
    std::vector<DeletedKey> Items;
  };

  /**
   * @brief A long running operation to delete a key.
   *
   */
  class DeleteKeyOperation final
      : public Azure::Core::Operation<Azure::Security::KeyVault::Keys::DeletedKey> {
  private:
    /* DeleteKeyOperation can be constructed only by friends classes (internal creation). The
     * constructor is private and requires internal components.*/
    friend class KeyClient;

    std::shared_ptr<Azure::Security::KeyVault::Keys::KeyClient> m_keyClient;
    Azure::Security::KeyVault::Keys::DeletedKey m_value;
    std::string m_continuationToken;

    /* This is the implementation for checking the status of a deleted key. The key is considered
     * deleted if querying /deletedkeys/keyName returns 200 from server. Or whenever soft-delete is
     * disabled.*/
    std::unique_ptr<Azure::Core::Http::RawResponse> PollInternal(
        Azure::Core::Context const& context) override;

    Azure::Response<Azure::Security::KeyVault::Keys::DeletedKey> PollUntilDoneInternal(
        std::chrono::milliseconds period,
        Azure::Core::Context& context) override
    {
      while (true)
      {
        // Poll will update the raw response.
        Poll(context);
        if (IsDone())
        {
          break;
        }
        std::this_thread::sleep_for(period);
      }

      return Azure::Response<Azure::Security::KeyVault::Keys::DeletedKey>(
          m_value, std::make_unique<Azure::Core::Http::RawResponse>(*m_rawResponse));
    }

    /*
     * Only friend classes are permitted to construct a DeleteOperation. This is because a
     * KeyVaultPipelne is required and it is not exposed to customers.
     *
     * Since C++ doesn't offer `internal` access, we use friends-only instead.
     */
    DeleteKeyOperation(
        std::shared_ptr<Azure::Security::KeyVault::Keys::KeyClient> keyClient,
        Azure::Response<Azure::Security::KeyVault::Keys::DeletedKey> response);

    DeleteKeyOperation(
        std::string resumeToken,
        std::shared_ptr<Azure::Security::KeyVault::Keys::KeyClient> keyClient)
        : m_keyClient(keyClient), m_value(DeletedKey(resumeToken)),
          m_continuationToken(std::move(resumeToken))
    {
    }

    /**
     * @brief Get the #Azure::Core::Http::RawResponse of the operation request.
     * @return A reference to an #Azure::Core::Http::RawResponse.
     * @note Does not give up ownership of the RawResponse.
     */
    Azure::Core::Http::RawResponse const& GetRawResponseInternal() const override
    {
      return *m_rawResponse;
    }

  public:
    /**
     * @brief Get the #Azure::Security::KeyVault::Keys::DeletedKey object.
     *
     * @remark The deleted key contains the recovery id if the key can be recovered.
     *
     * @return A deleted key object.
     */
    Azure::Security::KeyVault::Keys::DeletedKey Value() const override { return m_value; }

    /**
     * @brief Get an Url as string which can be used to get the status of the delete key operation.
     *
     * @return std::string
     */
    std::string GetResumeToken() const override { return m_continuationToken; }

    /**
     * @brief Create a #DeleteKeyOperation from the \p resumeToken fetched from another
     * `Operation<T>`, updated to the the latest operation status.
     *
     * @remark After the operation is initialized, it is used to poll the last update from the
     * server using the \p context.
     *
     * @param resumeToken A previously generated token used to resume the polling of the operation.
     * @param client A #KeyClient that is used for getting status updates.
     * @param context A #Azure::Core::Context controlling the request lifetime.
     * @return DeleteKeyOperation
     */
    static DeleteKeyOperation CreateFromResumeToken(
        std::string const& resumeToken,
        Azure::Security::KeyVault::Keys::KeyClient const& client,
        Azure::Core::Context const& context = Azure::Core::Context());
  };

  /**
   * @brief A long running operation to recover a key.
   *
   */
  class RecoverDeletedKeyOperation final : public Azure::Core::Operation<KeyVaultKey> {
  private:
    /* RecoverDeletedKeyOperation can be constructed only by friends classes (internal creation).
     * The constructor is private and requires internal components.*/
    friend class KeyClient;

    std::shared_ptr<Azure::Security::KeyVault::Keys::KeyClient> m_keyClient;
    Azure::Security::KeyVault::Keys::KeyVaultKey m_value;
    std::string m_continuationToken;

    std::unique_ptr<Azure::Core::Http::RawResponse> PollInternal(
        Azure::Core::Context const& context) override;

    Azure::Response<Azure::Security::KeyVault::Keys::KeyVaultKey> PollUntilDoneInternal(
        std::chrono::milliseconds period,
        Azure::Core::Context& context) override
    {
      while (true)
      {
        // Poll will update the raw response.
        Poll(context);
        if (IsDone())
        {
          break;
        }
        std::this_thread::sleep_for(period);
      }

      return Azure::Response<Azure::Security::KeyVault::Keys::KeyVaultKey>(
          m_value, std::make_unique<Azure::Core::Http::RawResponse>(*m_rawResponse));
    }

    /*
     * Only friend classes are permitted to construct a RecoverDeletedKeyOperation. This is because
     * a KeyVaultPipelne is required and it is not exposed to customers.
     *
     * Since C++ doesn't offer `internal` access, we use friends-only instead.
     */
    RecoverDeletedKeyOperation(
        std::shared_ptr<Azure::Security::KeyVault::Keys::KeyClient> keyClient,
        Azure::Response<Azure::Security::KeyVault::Keys::KeyVaultKey> response);

    RecoverDeletedKeyOperation(
        std::string resumeToken,
        std::shared_ptr<Azure::Security::KeyVault::Keys::KeyClient> keyClient)
        : m_keyClient(keyClient), m_value(DeletedKey(resumeToken)),
          m_continuationToken(std::move(resumeToken))
    {
    }

    /**
     * @brief Get the #Azure::Core::Http::RawResponse of the operation request.
     * @return A reference to an #Azure::Core::Http::RawResponse.
     * @note Does not give up ownership of the RawResponse.
     */
    Azure::Core::Http::RawResponse const& GetRawResponseInternal() const override
    {
      return *m_rawResponse;
    }

  public:
    /**
     * @brief Get the #Azure::Security::KeyVault::Keys::KeyVaultKey object.
     *
     * @remark The deleted key contains the recovery ID if the key can be recovered.
     *
     * @return A deleted key object.
     */
    Azure::Security::KeyVault::Keys::KeyVaultKey Value() const override { return m_value; }

    /**
     * @brief Get an Url as string which can be used to get the status of the delete key operation.
     *
     * @return std::string
     */
    std::string GetResumeToken() const override { return m_continuationToken; }

    /**
     * @brief Create a #RecoverDeletedKeyOperation from the \p resumeToken fetched from another
     * `Operation<T>`, updated to the the latest operation status.
     *
     * @remark After the operation is initialized, it is used to poll the last update from the
     * server using the \p context.
     *
     * @param resumeToken A previously generated token used to resume the polling of the operation.
     * @param client A #KeyClient that is used for getting status updates.
     * @param context A #Azure::Core::Context controlling the request lifetime.
     * @return DeleteKeyOperation
     */
    static RecoverDeletedKeyOperation CreateFromResumeToken(
        std::string const& resumeToken,
        Azure::Security::KeyVault::Keys::KeyClient const& client,
        Azure::Core::Context const& context = Azure::Core::Context());
  };

}}}} // namespace Azure::Security::KeyVault::Keys

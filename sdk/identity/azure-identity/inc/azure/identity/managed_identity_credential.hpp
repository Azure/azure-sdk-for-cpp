// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Managed Identity Credential and options.
 */

#pragma once

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/credentials/token_credential_options.hpp>

#include <memory>
#include <string>

namespace Azure { namespace Identity {
  namespace _detail {
    class ManagedIdentitySource;
  }

  // This will move to Azure::Core.
  /**
   * @brief An Azure Resource Manager resource identifier.
   */
  class ResourceIdentifier final {
    std::string m_resourceId;

  public:
    /**
     * @brief Constructs a resource identifier.
     *
     * @param resourceId The id string to create the ResourceIdentifier from.
     */
    explicit ResourceIdentifier(std::string const& resourceId) : m_resourceId(resourceId){};

    /**
     * @brief The string representation of this resource identifier.
     *
     * @return The resource identifier string.
     */
    std::string ToString() const { return m_resourceId; }
  };

  /**
   * @brief The type of managed identity identifier depending on how the managed identity is
   * configured.
   *
   * @remark This can either be system-assigned, or user-assigned which corresponds to an identifier
   * that represents either client ID, resource ID, or object ID, depending on how the managed
   * identity is configured.
   */
  enum class ManagedIdentityIdType
  {
    SystemAssigned,
    ClientId,
    ObjectId,
    ResourceId,
  };

  /**
   * @brief The type of managed identity and its corresponding identifier.
   *
   * @remark This class holds the type and unique identifier for either a system or user-assigned
   * managed identity.
   */
  class ManagedIdentityType final {
  private:
    ManagedIdentityIdType m_idType;
    std::string m_id;

  public:
    /**
     * @brief Constructs the type of managed identity.
     *
     * @remark This defaults to ManagedIdentityIdType::SystemAssigned.
     */
    explicit ManagedIdentityType() : m_idType(ManagedIdentityIdType::SystemAssigned) {}

    /**
     * @brief Constructs the type of managed identity.
     *
     * @param idType The type of the managed identity identifier.
     * @param id The value of the managed identity identifier. This can be either a client ID,
     * resource ID, or object ID.
     *
     * @remark For ManagedIdentityIdType::SystemAssigned, the id must be an empty string.
     *
     * @remark Make sure the type of ID matches the value of the ID. For example, the client
     * ID and object ID are NOT interchangeable, even though they are both Uuid values.
     */
    explicit ManagedIdentityType(ManagedIdentityIdType idType, std::string id)
        : m_idType(idType), m_id(id)
    {
      if (idType == ManagedIdentityIdType::SystemAssigned && !id.empty())
      {
        throw std::invalid_argument(
            "There is no need to provide an ID (such as client, object, or resource ID) if you are "
            "using system-assigned managed identity.");
      }

      if (id.empty()
          && (idType == ManagedIdentityIdType::ClientId || idType == ManagedIdentityIdType::ObjectId
              || idType == ManagedIdentityIdType::ResourceId))
      {
        throw std::invalid_argument(
            "Provide the value of the client, object, or resource ID corresponding to the "
            "ManagedIdentityIdType specified. The provided ID should not be empty in the case of "
            "user-assigned managed identity.");
      }
    }

    /**
     * @brief Gets the identifier for a user-assigned managed identity.
     *
     * @remark In the case of system-assigned managed identity, this will return an empty string.
     */
    std::string const& GetId() const { return m_id; }

    /**
     * @brief Gets the type of identifier used for the managed identity, depending on how it is
     * configured.
     */
    ManagedIdentityIdType GetManagedIdentityIdType() const { return m_idType; }
  };

  /**
   * @brief Options for managed identity credential.
   *
   */
  struct ManagedIdentityCredentialOptions final : public Core::Credentials::TokenCredentialOptions
  {
    /**
     * @brief Specifies the type of managed identity and its corresponding identifier, based on how
     * it was configured.
     */
    ManagedIdentityType IdentityType;
  };

  /**
   * @brief Attempts authentication using a managed identity that has been assigned to the
   * deployment environment. This authentication type works in Azure VMs, App Service and Azure
   * Functions applications, as well as the Azure Cloud Shell. More information about configuring
   * managed identities can be found here:
   * https://learn.microsoft.com/entra/identity/managed-identities-azure-resources/overview
   */
  class ManagedIdentityCredential final : public Core::Credentials::TokenCredential {
  private:
    std::unique_ptr<_detail::ManagedIdentitySource> m_managedIdentitySource;

  public:
    /**
     * @brief Destructs `%TokenCredential`.
     *
     */
    ~ManagedIdentityCredential() override;

    /**
     * @brief Constructs a Managed Identity Credential.
     *
     * @param clientId Client ID.
     * @param options Options for token retrieval.
     */
    explicit ManagedIdentityCredential(
        std::string const& clientId = std::string(),
        Azure::Core::Credentials::TokenCredentialOptions const& options
        = Azure::Core::Credentials::TokenCredentialOptions());

    /**
     * @brief Constructs a Managed Identity Credential.
     *
     * @param options Options for token retrieval.
     */
    explicit ManagedIdentityCredential(
        Azure::Identity::ManagedIdentityCredentialOptions const& options);

    /**
     * @brief Constructs a Managed Identity Credential.
     *
     * @param options Options for token retrieval.
     */
    explicit ManagedIdentityCredential(
        Azure::Core::Credentials::TokenCredentialOptions const& options);

    /**
     * @brief Gets an authentication token.
     *
     * @param tokenRequestContext A context to get the token in.
     * @param context A context to control the request lifetime.
     *
     * @return Authentication token.
     *
     * @throw Azure::Core::Credentials::AuthenticationException Authentication error occurred.
     */
    Core::Credentials::AccessToken GetToken(
        Core::Credentials::TokenRequestContext const& tokenRequestContext,
        Core::Context const& context) const override;
  };

}} // namespace Azure::Identity

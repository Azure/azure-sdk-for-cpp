// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the supported options to create a Attestation clients.
 *
 */

#pragma once

#include "azure/core/internal/client_options.hpp"

#include "attestation_client_models.hpp"
#include "dll_import_export.hpp"

namespace Azure { namespace Security { namespace Attestation {

  class ServiceVersion final {
  private:
    std::string m_version;

  public:
    /**
     * @brief Construct a new Service Version object
     *
     * @param version The string version for the Attestation service.
     */
    ServiceVersion(std::string version) : m_version(std::move(version)) {}

    /**
     * @brief Enable comparing the ext enum.
     *
     * @param other Another #ServiceVersion to be compared.
     */
    bool operator==(ServiceVersion const& other) const { return m_version == other.m_version; }

    /**
     * @brief Return the #ServiceVersion string representation.
     *
     */
    std::string const& ToString() const { return m_version; }

    /**
     * @brief Use to send request to the 2020-10-01 version of Attestation service.
     *
     */
    AZ_ATTESTATION_DLLEXPORT static const ServiceVersion V2020_10_01;
  };

  /// The TokenValidationCallbackFn represents a callback which is called to allow the caller
  /// to perform additional token validation options beyond the validations performed by the
  /// attestation SDK.
  using TokenValidationCallbackFn = std::function<
      void(std::string const& rawToken, Models::AttestationSigner const& tokenSigner)>;

  /** @brief The AttestationTokenValidationOptions represents a set of options which control how
   * attestation tokens are validated. */
  struct AttestationTokenValidationOptions final
  {
    /// Controls whether attestation tokens are validated at all.
    ///
    /// Default value: true.
    bool ValidateToken{true};

    /// Controls whether the signature for the attestation token should be validated.
    ///
    /// Default value: true.
    bool ValidateSigner{true};

    /// Controls whether the attestation token expiration time is checked.
    ///
    /// Default value: true.
    bool ValidateExpirationTime{true};

    /// Controls whether or not the attestation token start time is checked.
    ///
    /// Default value: true.
    bool ValidateNotBeforeTime{true};

    /// Controls whether the issuer of the attestation token is checked.
    ///
    /// Default value: false;
    bool ValidateIssuer{false};

    /// The expected issuer for this attestation token.
    ///
    /// Ignored unless {@link AttestationTokenValidationOptions::ValidateIssuer} is true.
    std::string ExpectedIssuer;

    /// The slack used when comparing two time elements.
    std::chrono::seconds ValidationTimeSlack{0};

    /// The TokenValidationCallback specifies a callback function which can perform additional token
    /// validation actions.
    TokenValidationCallbackFn ValidationCallback;
  };

  /**
   * @brief Define the options to create an SDK Keys client.
   *
   */
  struct AttestationClientOptions final : public Azure::Core::_internal::ClientOptions
  {
    ServiceVersion Version;
    AttestationTokenValidationOptions TokenValidationOptions;
    /**
     * @brief Construct a new Key Client Options object.
     *
     * @param version Optional version for the client.
     */
    AttestationClientOptions(
        ServiceVersion version = ServiceVersion::V2020_10_01,
        AttestationTokenValidationOptions const& tokenValidationOptions = {})
        : Azure::Core::_internal::ClientOptions(), Version(version),
          TokenValidationOptions(tokenValidationOptions)
    {
    }
  };

  /** @brief The AttestationDataType represents how the attestation service should interpret the
   * {@link AttestOptions::RuntimeData} and {@link AttestOptions::InittimeData} fields.
   */
  class AttestationDataType final {
  private:
    std::string m_dataType;

  public:
    /**
     * @brief Construct a new attestation DataType object
     *
     * @param dataType The string version for the Key Vault keys service.
     */
    AttestationDataType(std::string dataType) : m_dataType(std::move(dataType)) {}
    AttestationDataType() {}
    /**
     * @brief Enable comparing the ext enum.
     *
     * @param other Another AttestationDataType to be compared.
     */
    bool operator==(AttestationDataType const& other) const
    {
      return m_dataType == other.m_dataType;
    }

    /**
     * @brief Return the #AttestationDataType string representation.
     *
     */
    std::string const& ToString() const { return m_dataType; }

    /**
     * @brief When specified, instructs the attestation service to express the runtime data in the
     * generated token as a JSON object.
     *
     */
    AZ_ATTESTATION_DLLEXPORT static const AttestationDataType Json;

    /**
     * @brief When specified, instructs the attestation service to express the runtime data in the
     * generated token as a Binary object.
     *
     */
    AZ_ATTESTATION_DLLEXPORT static const AttestationDataType Binary;
  };

  /** @brief AttestationData represents a block of data to be sent to the attestation service.
   * See the description of the {@link AttestationClient} class for more information about how the
   * AttestationData type works.
   */
  struct AttestationData final
  {
    /// Data contained within attestation evidence. The attestation service will verify that the
    /// evidence does contain this data and will include the Data in the attestation token. A
    /// relying party can then use this data.
    std::vector<uint8_t> Data;

    /// Reflects how the Data field should be represented in the resulting attestation token.
    AttestationDataType DataType;
  };

  /** @brief Parameters sent to the attestation service to be consumed in the attestation operation.
   */
  struct AttestOptions final
  {
    /// Data created dynamically by the enclave
    Azure::Nullable<AttestationData> RuntimeData;

    /// Data created when the enclave is created.
    Azure::Nullable<AttestationData> InittimeData{};

    /// Nonce which is sent to the attestation service to allow a caller to prevent replay attacks.
    Azure::Nullable<std::string> Nonce{};

    /// A test hook which allows developers to test attestation policies before they commit them to
    /// the service.
    Azure::Nullable<std::string> DraftPolicyForAttestation{};

    /// Specifies the options which 
    Azure::Nullable<AttestationTokenValidationOptions> TokenValidationOptions{};
  };

  /** @brief The AttestationSigningKey represents a tuple of asymmetric private cryptographic key
   * and X.509 certificate wrapping the public key contained in the certificate.
   *
   * It is used when signing a value to be sent to the attestation service for the Set Policy,
   * Reset Policy, Add Policy Management Certificate, and Remove Policy Management Certificate.
   */
  struct AttestationSigningKey final
  {
    /// A PEM encoded RSA or ECDSA private key which will be used to
    /// sign an attestation token.
    std::string PemEncodedPrivateKey;

    /// A PEM encoded X.509 certificate which will be sent to the
    /// attestation service to validate an attestation token. The
    /// public key embedded in the certificate MUST be the public
    /// key of the SigningPrivateKey.
    std::string PemEncodedX509Certificate;
  };

}}} // namespace Azure::Security::Attestation

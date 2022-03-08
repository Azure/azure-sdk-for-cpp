// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the supported options to create a Attestation clients.
 *
 */

#pragma once

#include "azure/attestation/attestation_client_models.hpp"
#include "dll_import_export.hpp"
#include <azure/core/internal/client_options.hpp>

namespace Azure { namespace Security { namespace Attestation {

  /** @brief Version to be used when communicating with the Attestation service.
   */
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
     * @brief Enable comparing the extensible enum.
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

  /**
   * @brief The TokenValidationCallbackFn represents a callback which is called to allow the caller
   *  to perform additional token validation options beyond the validations performed by the
   * attestation SDK.
   */
  using TokenValidationCallbackFn = std::function<
      void(std::string const& rawToken, Models::AttestationSigner const& tokenSigner)>;

  /** @brief The AttestationTokenValidationOptions represents a set of options which control how
   * attestation tokens are validated. */
  struct AttestationTokenValidationOptions final
  {
    /** @brief Controls whether attestation tokens are validated at all.
     *
     * Default value: true.
     */
    bool ValidateToken{true};

    /**
     * @brief Controls whether the signature for the attestation token should be validated.
     *
     * Default Value: true:
     */
    bool ValidateSigner{true};

    /** @brief Controls whether the attestation token expiration time is checked.
     *
     * Default value: true.
     */
    bool ValidateExpirationTime{true};

    /** @brief Controls whether or not the attestation token start time is checked.
     *
     * Default value: true.
     */
    bool ValidateNotBeforeTime{true};

    /** @brief Controls whether the issuer of the attestation token is checked.
     *
     * Default value: false.
     */
    bool ValidateIssuer{false};

    /** @brief The expected issuer for this attestation token.
     *
     * Ignored unless {@link AttestationTokenValidationOptions::ValidateIssuer} is true.
     */
    std::string ExpectedIssuer;

    /** @brief The slack used when comparing two time elements.
     */
    std::chrono::seconds ValidationTimeSlack{0};

    /** @brief The TokenValidationCallback specifies a callback function which can perform
     * additional token validation actions.
     */
    TokenValidationCallbackFn ValidationCallback;
  };

  /**
   * @brief Define the options to create an Attestation client.
   */
  struct AttestationClientOptions final : public Azure::Core::_internal::ClientOptions
  {
    /** @brief Version to use when communicating with the attestation service.
     */
    ServiceVersion Version;
    /** @brief Options sent when validating tokens received by the attestation service.
     */
    AttestationTokenValidationOptions TokenValidationOptions;
    /**
     * @brief Construct a new Attestation Client Options object.
     *
     * @param version Optional version for the client.
     * @param tokenValidationOptions Options applied when validating attestation tokens returned by
     * the service.
     */
    AttestationClientOptions(
        ServiceVersion version = ServiceVersion::V2020_10_01,
        AttestationTokenValidationOptions const& tokenValidationOptions = {})
        : Azure::Core::_internal::ClientOptions(), Version(version),
          TokenValidationOptions(tokenValidationOptions)
    {
    }
  };

  /**
   * @brief Define the options to create an Attestation Administration client.
   */
  struct AttestationAdministrationClientOptions final : public Azure::Core::_internal::ClientOptions
  {
    /** @brief Version to use when communicating with the attestation service.
     */
    ServiceVersion Version;
    /** @brief Options sent when validating tokens received by the attestation service.
     */
    AttestationTokenValidationOptions TokenValidationOptions;
    /**
     * @brief Construct a new Attestation Client Options object.
     *
     * @param version Optional version for the client.
     * @param tokenValidationOptions Options applied when validating attestation tokens returned by
     * the service.
     */
    AttestationAdministrationClientOptions(
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
     * @brief Enable comparing the extensible enum.
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
    /**
     * @brief Data contained within attestation evidence.
     * @details The attestation service will verify that the
     * evidence does contain this data and will include the Data in the attestation token. A
     * relying party can then use this data.
     */
    std::vector<uint8_t> Data;

    /** @brief Reflects how the Data field should be represented in the resulting attestation token.
     */
    AttestationDataType DataType;
  };

  /** @brief Parameters sent to the attestation service to be consumed in an attestation operation.
   */
  struct AttestOptions final
  {
    /**
     * @brief Data created dynamically within the enclave
     */
    Azure::Nullable<AttestationData> RuntimeData{};

    /**
     * @brief Data created when the enclave was created. Not supported on Coffeelake processors.
     */
    Azure::Nullable<AttestationData> InittimeData{};

    /**
     * @brief Nonce which is sent to the attestation service to allow a caller to prevent replay
     * attacks.
     */
    Azure::Nullable<std::string> Nonce{};

    /**
     * @brief A test hook which allows developers to test attestation policies before they commit
     * them to the service.
     */
    Azure::Nullable<std::string> DraftPolicyForAttestation{};

    /** @brief Specifies the options which should be used to validate the attestation token returned
     * by the attestation service.
     * @details If not provided by the caller, the token validation options
     * specified when the @{link AttestationClient} was created will be used.
     */
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
    /** @brief A PEM encoded RSA or ECDSA private key which will be used to
     * sign an attestation token.
     */
    std::string PemEncodedPrivateKey;

    /** @brief A PEM encoded X.509 certificate which will be sent to the
     * attestation service to validate an attestation token. The
     * public key embedded in the certificate MUST be the public
     * key of the SigningPrivateKey.
     */
    std::string PemEncodedX509Certificate;
  };

  /** @brief Parameters sent to the attestation service when retrieving an attestation policy.
   */
  struct GetPolicyOptions final
  {
    /** @brief Specifies the options which should be used to validate the attestation token returned
     * by the attestation service.
     * @details If not provided by the caller, the token validation options
     * specified when the @{link AttestationAdministrationClient} was created will be used.
     */
    Azure::Nullable<AttestationTokenValidationOptions> TokenValidationOptions{};
  };

  /** @brief Parameters sent to the attestation service when setting an attestation policy.
   */
  struct SetPolicyOptions final
  {
    /** @brief Optional Signing Key which is used to sign the SetPolicy request.
     */
    Azure::Nullable<AttestationSigningKey> SigningKey;

    /** @brief Specifies the options which should be used to validate the attestation token returned
     * by the attestation service.
     * @details If not provided by the caller, the token validation options
     * specified when the @{link AttestationAdministrationClient} was created will be used.
     */
    Azure::Nullable<AttestationTokenValidationOptions> TokenValidationOptions{};
  };

  /** @brief Parameters sent to the attestation service when retrieving the list of policy management certificates.
   */
  struct GetPolicyManagementCertificatesOptions final
  {
    /** @brief Specifies the options which should be used to validate the attestation token returned
     * by the attestation service.
     * @details If not provided by the caller, the token validation options
     * specified when the @{link AttestationAdministrationClient} was created will be used.
     */
    Azure::Nullable<AttestationTokenValidationOptions> TokenValidationOptions{};
  };

}}} // namespace Azure::Security::Attestation

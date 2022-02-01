// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the supported options to create a Attestation clients.
 *
 */

#pragma once

#include <azure/core/internal/client_options.hpp>

#include "azure/attestation/attestation_client_models.hpp"
#include "azure/attestation/dll_import_export.hpp"

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

  /**
   * @brief Define the options to create an SDK Keys client.
   *
   */
  struct AttestationClientOptions final : public Azure::Core::_internal::ClientOptions
  {
    ServiceVersion Version;

    /**
     * @brief Construct a new Key Client Options object.
     *
     * @param version Optional version for the client.
     */
    AttestationClientOptions(ServiceVersion version = ServiceVersion::V2020_10_01)
        : Azure::Core::_internal::ClientOptions(), Version(version)
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
    AttestationData RuntimeData;

    /// Data created when the enclave is created.
    AttestationData InittimeData = {};

    /// A test hook which allows developers to test attestation policies before they commit them to
    /// the service.
    std::string DraftPolicyForAttestation = {};
  };
}}} // namespace Azure::Security::Attestation

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the supported options to create a Attestation clients.
 *
 */

#pragma once

#include <azure/core/internal/client_options.hpp>

#include "azure/attestation/dll_import_export.hpp"
#include "azure/attestation/attestation_client_models.hpp"

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

  class DataType final {
  private:
    std::string m_dataType;

  public:
    /**
     * @brief Construct a new attestation DataType object
     *
     * @param dataType The string version for the Key Vault keys service.
     */
    DataType(std::string dataType) : m_dataType(std::move(dataType)) {}
    DataType() {}
    /**
     * @brief Enable comparing the ext enum.
     *
     * @param other Another #ServiceVersion to be compared.
     */
    bool operator==(DataType const& other) const { return m_dataType == other.m_dataType; }

    /**
     * @brief Return the #ServiceVersion string representation.
     *
     */
    std::string const& ToString() const { return m_dataType; }

    /**
     * @brief Specified to express the runtime data in the generated token as a JSON object.
     *
     */
    AZ_ATTESTATION_DLLEXPORT static const DataType Json;

    /**
     * @brief Specified to express the runtime data in the generated token as a Binary object.
     *
     */
    AZ_ATTESTATION_DLLEXPORT static const DataType Binary;

  };

  struct AttestationData final
  {
    std::vector<uint8_t> Data;
    DataType DataType;
  };

  struct AttestOptions final
  {
    AttestationData RuntimeData;
    AttestationData InittimeData;
    std::string DraftPolicyForAttestation;
  };
}}} // namespace Azure::Security::Attestation
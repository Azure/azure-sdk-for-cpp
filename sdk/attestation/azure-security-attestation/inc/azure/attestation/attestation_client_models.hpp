// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the Azure Attestation API types.
 *
 */

#pragma once

#include "azure/attestation/dll_import_export.hpp"
#include <azure/core/context.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/nullable.hpp>
#include <azure/core/paged_response.hpp>
#include <azure/core/response.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// cspell: words MRSIGNER MRENCLAVE
namespace Azure { namespace Security { namespace Attestation { namespace Models {
  /**
   * @brief Contains information about this instance of the attestation service, which can be
   * used to validate attestation service responses.
   *
   */
  struct AttestationOpenIdMetadata final
  {
    /** @brief The issuer which will be used for tokens generated by this instance.
     */
    Azure::Nullable<std::string> Issuer;

    /** @brief  A URI which can be used to retrieve the AttestationSigner
     * objects returned by the attestation service.
     */
    Azure::Nullable<std::string> JsonWebKeySetUrl;

    /** @brief  The response types that are supported by the service.
     */
    Azure::Nullable<std::vector<std::string>> SupportedResponseTypes;

    /** @brief  The algorithms which can be used
     * to sign attestation tokens.
     */
    Azure::Nullable<std::vector<std::string>> SupportedTokenSigningAlgorithms;

    /** @brief  A list of claims which may be returned by the attestation service.
     */
    Azure::Nullable<std::vector<std::string>> SupportedClaims;
  };

  /** @brief An AttestationSigner represents an X .509 certificate and KeyID pair.
   *
   * @note
   * There are two use scenarios for an AttestationSigner:
   * -# The certificate in an AttestationSigner can be used to sign a token generated
   * by the attestation service.
   * -# The certificate which is used to sign an attestation policy.
   */
  struct AttestationSigner final
  {
    /** @brief The KeyID associated with the Certificate Chain.
     */
    Azure::Nullable<std::string> KeyId;

    /** @brief  An array of PEM encoded X .509 certificates.The
     * first certificate in the array
     * will be used to sign an attestation token or policy.
     */
    Azure::Nullable<std::vector<std::string>> CertificateChain;
  };

  /** @brief An AttestationResult reflects the result of an Attestation operation.
   *
   * The fields in the AttestationResult represent the claims in the AttestationToken returned
   * by the attestation service.
   */
  struct AttestationResult final
  {

    /** @brief The nonce provided by the client in the attestation operation.
     */
    Azure::Nullable<std::string> Nonce;

    /** @brief  The version of this attestation response. */
    Azure::Nullable<std::string> Version;

    /** @brief JSON encoded runtime claims - this will be the input RuntimeData
     * parameter decoded and interpreted as JSON.
     */
    Azure::Nullable<std::string> RuntimeClaims;

    /** @brief InitTime claims
     *    - this will be the InitTimeData parameter
     *  decoded and interpreted as JSON.
     */
    Azure::Nullable<std::string> InitTimeClaims;

    /** @brief PolicyClaims is the JSON encoded values of all the claims created
     * by attestation policies on this instance.
     */
    Azure::Nullable<std::string> PolicyClaims;

    /** @brief If the RuntimeData parameter is specified as being of
     * {@link Models::DataType}::Binary, this will be the
     * value of the RuntimeData input.
     */
    Azure::Nullable<std::vector<uint8_t>> SgxEnclaveHeldData;

    /** @brief  The verifier which generated this AttestationResult. */
    Azure::Nullable<std::string> VerifierType;

    /** @brief  If the attestation policy is signed,
     * this will be the certificate chain used
     * to sign the policy.
     */
    Azure::Nullable<AttestationSigner> PolicySigner;

    /** @brief  The SHA256 hash of the policy which was used generating the
     *attestation result.
     */
    Azure::Nullable<std::vector<uint8_t>> PolicyHash;

    /** @brief  If present, reflects that the enclave being attestated can be debugged.
     * @note: If VerifierType is "sgx", then this field *must* be present */

    Azure::Nullable<bool> SgxIsDebuggable;

    /** @brief If present, the ProductId for the enclave being attested.
     * @note : If VerifierType is "sgx", then this field *must* be present */
    Azure::Nullable<int> SgxProductId;

    /** @brief If present, the contents of the MRENCLAVE register for the SGX enclave being
     * attested - this reflects the hash of the binary being run in the enclave.
     *
     * @note : If VerifierType is "sgx", then this field *must* be present
     */
    Azure::Nullable<std::vector<uint8_t>> SgxMrEnclave;

    /** @brief If present, the contents of the MRSIGNER register for the SGX
     * enclave being attested - this reflects the key which was used
     * to sign the enclave image being run in the enclave.
     * @note : If VerifierType is "sgx", then this field *must* be present
     */
    Azure::Nullable<std::vector<uint8_t>> SgxMrSigner;

    /** @brief  The security version number of the SGX enclave.
     * @note : If VerifierType is "sgx", then this field *must* be present
     */

    Azure::Nullable<int> SgxSvn;

    /** @brief A JSON encoded string representing the collateral which was used
     * to perform the attestation operation.
     * @note : If VerifierType is "sgx", then this field *must* be present
     */
    Azure::Nullable<std::string> SgxCollateral;
  };

}}}} // namespace Azure::Security::Attestation::Models

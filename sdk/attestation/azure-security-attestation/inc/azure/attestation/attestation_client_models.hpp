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

  class AttestationType final {
  private:
    std::string m_attestationType;

  public:
    /**
     * @brief Construct a new AttestationType object
     *
     * @param attestationType The string attestationType used for the attestation policy operation.
     */
    AttestationType(std::string attestationType) : m_attestationType(std::move(attestationType)) {}

    /**
     * @brief Enable comparing the ext enum.
     *
     * @param other Another #ServiceVersion to be compared.
     */
    bool operator==(AttestationType const& other) const
    {
      return m_attestationType == other.m_attestationType;
    }

    /**
     * @brief Return the #ServiceVersion string representation.
     *
     */
    std::string const& ToString() const { return m_attestationType; }

    /**
     * @brief Specifies that this should apply to SGX enclaves.
     *
     */
    AZ_ATTESTATION_DLLEXPORT static const AttestationType SgxEnclave;

    /**
     * @brief Specifies that this should apply to SGX enclaves using the OpenEnclave APIs.
     *
     */
    AZ_ATTESTATION_DLLEXPORT static const AttestationType OpenEnclave;

    /**
     * @brief Specifies that this should apply to TPM enclaves.
     *
     */
    AZ_ATTESTATION_DLLEXPORT static const AttestationType Tpm;
  };

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

  /** @brief An AttestationTokenHeader represents common properties in an the RFC 7515 JSON Web
   * Token.
   */
  struct AttestationTokenHeader
  {
    /** The "" alg " token header property. See
     *  <a href='https://datatracker.ietf.org/doc/html/rfc7515#section-4.1.1'>RFC 7515
     * section 4.1.1</a>
     */
    Azure::Nullable<std::string> Algorithm;

    /**
     * @brief The "kid" token header property See
     * <a href='https://datatracker.ietf.org/doc/html/rfc7515#section-4.1.4'>RFC 7515
     * section 4.1.4</a>
     */
    Azure::Nullable<std::string> KeyId;

    /**
     * Returns the signer for this token if the caller provided a JSON Web Key.
     *
     * See <a href='https://datatracker.ietf.org/doc/html/rfc7515#section-4.1.3'>RFC 7515
     * section 4.1.3</a> for more information.
     *
     */
    Azure::Nullable<AttestationSigner> Key;

    /**
     * The "cty" header property of the JWS.
     *
     * See <a href='https://datatracker.ietf.org/doc/html/rfc7515#section-4.1.10'>RFC 7515
     * section 4.1.10</a> for more information.
     *
     */
    Azure::Nullable<std::string> ContentType;

    /**
     * A URI which can be used to retrieve a JSON Web Key which can verify the signature on
     * this token.
     *
     * See <a href='https://datatracker.ietf.org/doc/html/rfc7515#section-4.1.5'>RFC 7515
     * section 4.1.5</a> for more information.
     *
     */
    Azure::Nullable<std::string> KeyURL;

    /**
     * Returns the "crit" header property from the JSON Web Signature object.
     *
     * See <a href='https://datatracker.ietf.org/doc/html/rfc7515#section-4.1.11'>RFC 7515
     * section 4.1.11</a> for more information.
     *
     */
    Azure::Nullable<std::vector<std::string>> Critical;

    /**
     * Returns a URI which can be used to retrieve an X.509 certificate which can verify the
     * signature on this token.
     *
     * See <a href='https://datatracker.ietf.org/doc/html/rfc7515#section-4.1.5'>RFC 7515
     * section 4.1.5</a> for more information.
     *
     */
    Azure::Nullable<std::string> X509Url;

    /**
     * Returns the "typ" header property from the JWS.
     *
     * See <a href='https://datatracker.ietf.org/doc/html/rfc7515#section-4.1.9'>RFC 7515
     * section 4.1.9</a> for more information.
     *
     */
    Azure::Nullable<std::string> Type;

    /**
     * Returns the SHA-1 thumbprint of the leaf certificate in the getCertificateChain.
     *
     * See <a href='https://datatracker.ietf.org/doc/html/rfc7515#section-4.1.7'>RFC 7515
     * section 4.1.7</a> for more information.
     *
     */
    Azure::Nullable<std::string> CertificateThumbprint;

    /**
     * Returns the SHA-256 thumbprint of the leaf certificate in the getCertificateChain.
     *
     * See <a href='https://datatracker.ietf.org/doc/html/rfc7515#section-4.1.8'>RFC 7515
     * section 4.1.8</a> for more information.
     *
     */
    Azure::Nullable<std::string> CertificateSha256Thumbprint;

    /**
     * Returns the signing certificate chain as an AttestationSigner.
     *
     * See <a href='https://datatracker.ietf.org/doc/html/rfc7515#section-4.1.6'>RFC 7515
     * section 4.1.6</a> for more information.
     *
     */
    Azure::Nullable<std::vector<std::string>> X509CertificateChain;
  };

  /** @brief A collection of {@link AttestationSigner} objects.
   *
   */
  struct AttestationSigningCertificateResult final
  {
    /** @brief The collection of signers.
     */
    std::vector<AttestationSigner> Signers;
  };

  /** An AttestationToken represents an RFC 7519 JSON Web Token returned from the attestation
   * service with the specialized body type.
   * <typeparam name="T"></typeparam> The type which represents the body of the attestation token.
   */
  template <typename T> class AttestationToken final {
  public:
    /**
     * @brief The full RFC 7515 JWS/JWT token returned by the attestation service.
     */
    std::string RawToken;

    /**
     * @brief The elements of the raw token which will be signed by the Signature.
     */
    std::string SignedElements;

    /**
     * @brief Signature (if present) for the attestation token.
     */
    std::vector<uint8_t> Signature;

    /**
     * @brief RFC 7515 header properties.
     */
    Models::AttestationTokenHeader Header;

    // RFC 7519 properties.

    /**
     *  The Expiration time for this attestation token.
     *
     * After this time, the token cannot be considered valid.
     *
     * See <a href='https://datatracker.ietf.org/doc/html/rfc7519#section-4.1.4'>RFC 7519
     * Section 4.1.4</a> for more information.
     */
    Azure::Nullable<Azure::DateTime> ExpiresOn;

    /**
     *  The time at which this token was issued.
     *
     *
     * See <a href='https://datatracker.ietf.org/doc/html/rfc7519#section-4.1.6'>RFC 7519
     * Section 4.1.6</a> for more information.
     */
    Azure::Nullable<Azure::DateTime> IssuedOn;

    /**
     *  The time before which this token cannot be considered valid.
     *
     *
     * See <a href='https://datatracker.ietf.org/doc/html/rfc7519#section-4.1.5'>RFC 7519
     * Section 4.1.5</a> for more information.
     */
    Azure::Nullable<Azure::DateTime> NotBefore;

    /**
     *  The issuer of this attestation token
     *
     *
     * See <a href='https://datatracker.ietf.org/doc/html/rfc7519#section-4.1.1'>RFC 7519
     * Section 4.1.1</a> for more information.
     */
    Azure::Nullable<std::string> Issuer;

    /**
     *  An identifier which uniquely identifies this token.
     *
     * See <a href='https://datatracker.ietf.org/doc/html/rfc7519#section-4.1.7'>RFC 7519
     * Section 4.1.7</a> for more information.
     */
    Azure::Nullable<std::string> UniqueIdentifier;

    /**
     * The subject for this attestation token.
     *
     * See <a href='https://datatracker.ietf.org/doc/html/rfc7519#section-4.1.2'>RFC 7519
     * Section 4.1.2</a> for more information.
     */
    Azure::Nullable<std::string> Subject;

    /**
     * The audience for this attestation token.
     *
     * See <a href='https://datatracker.ietf.org/doc/html/rfc7519#section-4.1.3'>RFC 7519
     * Section 4.1.3</a> for more information.
     */
    Azure::Nullable<std::string> Audience;

    /**
     * @brief The deserialized body of the attestation token.
     *
     */
    T Body;
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

  class PolicyModification final {
  private:
    std::string m_policyModification;

  public:
    /**
     * @brief Construct a new PolicyResolution object
     *
     * @param resolution The string resolution used for the result of an attestation policy
     * operation.
     */
    PolicyModification(std::string modification) : m_policyModification(std::move(modification)) {}

    /**
     * @brief Enable comparing the ext enum.
     *
     * @param other Another #ServiceVersion to be compared.
     */
    bool operator==(PolicyModification const& other) const
    {
      return m_policyModification == other.m_policyModification;
    }

    /**
     * @brief Return the #ServiceVersion string representation.
     *
     */
    std::string const& ToString() const { return m_policyModification; }

    /**
     * @brief Specifies that the policy object was updated.
     *
     */
    AZ_ATTESTATION_DLLEXPORT static const PolicyModification Updated;

    /**
     * @brief Specifies that the policy object was removed.
     *
     */
    AZ_ATTESTATION_DLLEXPORT static const PolicyModification Removed;
  };

  /**
   * @brief Result of a SetPolicy or ResetPolicy operation.
   */
  struct PolicyResult
  {
    /**
     * @brief Result of a modification.
     */
    Azure::Nullable<PolicyModification> PolicyResolution;

    /**
     * @brief The SHA256 hash of the policy object which was received by the service.
     */
    Azure::Nullable<std::vector<uint8_t>> PolicyTokenHash;

    /**
     * @brief A JSON Web Key containing the signer of the policy token. If not present, the token was unsecured.
     */
    Azure::Nullable<AttestationSigner> PolicySigner;
  };

}}}} // namespace Azure::Security::Attestation::Models

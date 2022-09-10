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
#include <azure/core/internal/extendable_enumeration.hpp>
#include <azure/core/nullable.hpp>
#include <azure/core/paged_response.hpp>
#include <azure/core/response.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Azure { namespace Security { namespace Attestation { namespace Models {

  /**
   * @brief The AttestationType type represent a Trusted Execution Environment supported by
   * the attestation service.
   *
   */
  class AttestationType final
      : public Azure::Core::_internal::ExtendableEnumeration<AttestationType> {
  public:
    /**
     * @brief Construct a new AttestationType object
     *
     * @param attestationType The string attestationType used for the attestation policy operation.
     */
    explicit AttestationType(std::string attestationType)
        : ExtendableEnumeration(std::move(attestationType))
    {
    }

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
   * The OpenIdMetadata value is retrieved using the [OpenID Connect Discovery
   * Protocol](https://openid.net/specs/openid-connect-discovery-1_0.html#ProviderMetadata).
   *
   * This structure represents the values from that specification which are returned by the
   * attestation service.
   *
   */
  struct OpenIdMetadata final
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
  struct TokenValidationCertificateResult final
  {
    /** @brief The collection of signers.
     */
    std::vector<AttestationSigner> Signers;
  };

  /**
   * @brief Optional elements when an AttestationToken is specialized on a type.
   */
  template <typename T> struct AttestationTokenOptional
  {
    /**
     * @brief The deserialized body of the attestation token.
     *
     */
    T Body;
  };

  template <> struct AttestationTokenOptional<void>
  {
  };

  /** @brief An AttestationResult reflects the result of an Attestation operation.
   *
   * The fields in the AttestationResult represent the claims in the AttestationToken returned
   * by the attestation service.
   *
   * @details When the attestation service returns a model type to the client, it embeds the
   * response in an AttestationToken, which is an [RFC7519 JSON Web Token]
   * (https://www.rfc-editor.org/rfc/rfc7519.html). The AttestationToken type represents both the
   * token and the embedded model type. In this scenario, the AttestationToken template will be
   * specialized on the model type (In other words, `AttestationToken<ModelType>`).
   *
   * There is another use for an AttestationToken object. That's when the model type for the
   * attestation token is unknown, or when it is not meaningful in context.
   *
   * For example, when the AttestationAdministrationClient::SetAttestationPolicy API returns, the
   * resulting PolicyResult model type contains a PolicyTokenHash field. This field consists of the
   * SHA256 hash of the policy document sent to the attestation service.
   *
   * In order to verify that the attestation service correctly received the attestation policy sent
   * by the client, the AttestationAdministrationClient::CreateAttestationPolicyToken API can be
   * used to create an AttestationToken object which is not specialized on any type
   * (`AttestationToken<>`). The RawToken field in that can be used to calculate the hash which was
   * sent to the service.
   *
   * Similarly, the AttestationTokenValidationOptions object has a TokenValidationCallback method.
   * This callback is called to allow the client to perform additional validations of the
   * attestation token beyond those normally performed by the attestation service. This callback
   * should not know the model type associated with the token, so it receives an AttestationToken<>
   * object.
   */
  template <typename T> struct AttestationToken final : public AttestationTokenOptional<T>
  {
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
  };

  struct AttestationResult final
  {

    /** @brief The nonce provided by the client in the attestation operation.
     */
    Azure::Nullable<std::string> Nonce;

    /** @brief  The version of this attestation response. */
    Azure::Nullable<std::string> Version;

    /** @brief JSON encoded runtime claims - this will be the input RunTimeData
     * parameter decoded and interpreted as JSON.
     */
    Azure::Nullable<std::string> RunTimeClaims;

    /** @brief InitTime claims
     *    - this will be the InitTimeData parameter
     *  decoded and interpreted as JSON.
     */
    Azure::Nullable<std::string> InitTimeClaims;

    /** @brief PolicyClaims is the JSON encoded values of all the claims created
     * by attestation policies on this instance.
     */
    Azure::Nullable<std::string> PolicyClaims;

    /** @brief If the RunTimeData parameter is specified as being of
     * DataType::Binary, this will be the
     * value of the RunTimeData input.
     */
    Azure::Nullable<std::vector<uint8_t>> EnclaveHeldData;

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

  /** @brief The result of a call to AttestTpm.
   */
  struct TpmAttestationResult final
  {
    /** @brief Attestation response data.
     * 
     * The TPM attestation protocol is defined
     * [here](https://docs.microsoft.com/azure/attestation/virtualization-based-security-protocol')
     *
     */
    std::vector<uint8_t> TpmResult;
  };

  /**
   * @brief The PolicyModification enumeration represents the result of an attestation
   * policy modification.
   *
   */
  class PolicyModification final
      : public Azure::Core::_internal::ExtendableEnumeration<PolicyModification> {
  public:
    /**
     * @brief Construct a new PolicyModification object
     *
     * @param modification The string resolution used for the result of an attestation policy
     * operation.
     */
    explicit PolicyModification(std::string modification)
        : ExtendableEnumeration(std::move(modification))
    {
    }
    PolicyModification() = default;

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
    PolicyModification PolicyResolution;

    /**
     * @brief The SHA256 hash of the policy object which was received by the service.
     */
    std::vector<uint8_t> PolicyTokenHash;

    /**
     * @brief A JSON Web Key containing the signer of the policy token. If not present, the token
     * was unsecured.
     */
    Azure::Nullable<AttestationSigner> PolicySigner;
  };

  /**
   * @brief Represents the result of a policy certificate modification.
   */
  class PolicyCertificateModification final
      : public Azure::Core::_internal::ExtendableEnumeration<PolicyCertificateModification> {
  public:
    /**
     * @brief Construct a new PolicyResolution object
     *
     * @param modification The string resolution used for the result of an attestation policy
     * operation.
     */
    explicit PolicyCertificateModification(std::string modification)
        : ExtendableEnumeration(std::move(modification))
    {
    }

    PolicyCertificateModification() = default;

    /**
     * @brief After the operation was performed, the certificate is in the set of
     * certificates.
     *
     */
    AZ_ATTESTATION_DLLEXPORT static const PolicyCertificateModification IsPresent;

    /**
     * @brief After the operation was performed, the certificate is no longer present in the set of
     * certificates.
     *
     */
    AZ_ATTESTATION_DLLEXPORT static const PolicyCertificateModification IsAbsent;
  };

  /**
   * @brief Represents the result of an Isolated Mode certificate modification API.
   */
  struct IsolatedModeCertificateModificationResult final
  {
    /**
     */
    std::string CertificateThumbprint;
    PolicyCertificateModification CertificateModification;
  };

  /**
   * @brief Represents a set of Isolated Mode certificates for the current attestation instance.
   */
  struct IsolatedModeCertificateListResult final
  {
    /**
     * @brief The current set of policy management certificates.
     */
    std::vector<AttestationSigner> Certificates;
  };

}}}} // namespace Azure::Security::Attestation::Models

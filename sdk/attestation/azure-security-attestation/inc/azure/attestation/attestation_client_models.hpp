// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the Azure Attestatoin API types.
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
namespace Azure { namespace Security { namespace Attestation {
  /**
   * @brief Contains information about this instance of the attestation service, which can be used
   * to validate attestation service responses.
   *
   */
  struct AttestationOpenIdMetadata final
  {
    /// The issuer which will be used for tokens generated by this instance.
    std::string Issuer;

    /// A URI which can be used to retrieve the AttestationSigner
    /// objects returned by the attestation service.
    std::string JsonWebKeySetUrl;

    /// The response types that are supported by the service.
    std::vector<std::string> SupportedResponseTypes;

    /// The algorithms which can be used
    /// to sign attestation tokens.
    std::vector<std::string> SupportedTokenSigningAlgorithms;

    /// A list of claims which may be returned by the attestation service.
    std::vector<std::string> SupportedClaims;
  };

  /** @brief An AttestationSigner represents an X .509 certificate and KeyID pair.
   *
   * There are two use scenarios for an AttestationSigner:
   * -# The certificate in an AttestationSigner can be usedto sign a token generated
   * by the attestation service.
   * -# The certificate which is used to sign an attestation policy.
   */
  struct AttestationSigner final
  {
    /// The KeyID associated with the Certificate Chain.
    std::string KeyId;

    /// An array of PEM encoded X.509 certificates. The
    /// first certificate in the array
    /// will be used to sign an attestation token or policy.
    std::vector<std::string> CertificateChain;
  };

  /** @brief An AttestationResult reflects the result of an Attestation operation.
   *
   * The fields in the AttestationResult represent the claims in the AttestationToken returned by
   * the attestation service.
   */
  struct AttestationResult final
  {
    /// The issuer of the attestation token (the instance of the attestation service)
    std::string Issuer;

    /// An identifier which uniquely identifies this {@link AttestationResult}
    std::string UniqueIdentifier;

    /// The nonce provided by the client in the attestation operation.
    std::string Nonce;

    /// The version of this attestation response.
    std::string Version;

    /// JSON encoded runtime claims - this will be the input RuntimeData
    /// parameter decoded and interpreted as JSON.
    std::string RuntimeClaims;

    /// InitTime claims - this will be the InitTimeData parameter
    /// decoded and interpreted as JSON.
    std::string InitTimeClaims;

    /// PolicyClaims - the JSON encoded values of all the claims created
    /// by attestation policies on this instance.
    std::string PolicyClaims;

    /// If the RuntimeData parameter is specified as being of
    /// {@link Models::DataType}::Binary, this will be the
    /// value of the RuntimeData input.
    std::vector<uint8_t> EnclaveHeldData;

    /// The verifier which generated this AttestationResult.
    std::string VerifierType;

    /// If the attestation policy is signed, this will be the signing chain used
    /// to sign the policy.
    Azure::Nullable<AttestationSigner> PolicySigner;

    /// The SHA256 hash of the policy which was used generating the
    /// attestation result.
    std::vector<uint8_t> PolicyHash;

    /// If present, reflects that the enclave being attestated can be debugged.
    Azure::Nullable<bool> IsDebuggable;

    /// If present, the ProductId for the enclave being attested.
    Azure::Nullable<int> ProductId{0};

    /// If present, the contents of the MRENCLAVE register for the SGX enclave being
    /// attested - this reflects the hash of the binary being run in the enclave.
    std::vector<uint8_t> MrEnclave;

    /// If present, the contents of the MRSIGNER register for the SGX
    /// enclave being attested - this reflects the key which was used
    /// to sign the enclave image being run in the enclave.
    std::vector<uint8_t> MrSigner;

    /// The security version number of the SGX enclave.
    Azure::Nullable<int> Svn;

    /// A JSON encoded string representing the collateral which was used
    /// to perform the attestation operation.
    std::string SgxCollateral;
  };

  /** @brief An AttestationTokenHeader represents common properties in an the RFC 7515 JSON Web
   * Token.
   */
  struct AttestationTokenHeader
  {
    /// The ""alg" token header property. See
    ///  <a href='https://datatracker.ietf.org/doc/html/rfc7515#section-4.1.1'>RFC 7515
    /// section 4.1.1</a>
    std::string Algorithm;

    /// The "kid" token header property See 
    /// <a href='https://datatracker.ietf.org/doc/html/rfc7515#section-4.1.4'>RFC 7515
    /// section 4.1.4</a>
    std::string KeyId;

    /**
     * The expiration time after which the token is no longer valid. The
     * ExpiresOn property corresponds to the "exp" claim in a Json Web Token.  See
     * <a href="https://datatracker.ietf.org/doc/html/rfc7519#section-4.1.4">RFC 7519 section 4.1.4</a>
     *
     */
    Azure::Nullable<Azure::DateTime> ExpiresOn;

    /**
     * The time before which a token cannot be considered valid. The
     * ExpiresOn property corresponds to the "exp" claim in a Json Web Token.  See 
     * <a href="https://datatracker.ietf.org/doc/html/rfc7519#section-4.1.4">RFC 7519 section 4.1.4</a>
     *
     */
    Azure::Nullable<Azure::DateTime> NotBefore;

    /**
     * The time at which the token was issued. The IssuedAt property
     * corresponds to the "iat" claim in a Json Web Token. See 
     * <a href="https://datatracker.ietf.org/doc/html/rfc7519#section-4.1.6">RFC 7519 section 4.1.6</a>
     * for more information.
     *
     */
    Azure::Nullable<Azure::DateTime> IssuedOn;

    /**
     * The "cty" header property of the JWS.
     *
     * See <a href='https://datatracker.ietf.org/doc/html/rfc7515#section-4.1.10'>RFC 7515
     * section 4.1.10</a> for more information.
     *
     */
    std::string ContentType;

    /**
     * A URI which can be used to retrieve a JSON Web Key which can verify the signature on
     * this token.
     *
     * See <a href='https://datatracker.ietf.org/doc/html/rfc7515#section-4.1.5'>RFC 7515
     * section 4.1.5</a> for more information.
     *
     */
    std::string KeyURL;

    /**
     * Returns the "crit" header property from the JSON Web Signature object.
     *
     * See <a href='https://datatracker.ietf.org/doc/html/rfc7515#section-4.1.11'>RFC 7515
     * section 4.1.11</a> for more information.
     *
     */
    std::vector<std::string> Critical;

    /**
     * Returns a URI which can be used to retrieve an X.509 certificate which can verify the
     * signature on this token.
     *
     * See <a href='https://datatracker.ietf.org/doc/html/rfc7515#section-4.1.5'>RFC 7515
     * section 4.1.5</a> for more information.
     *
     */

    std::string X509Url;

    /**
     * Returns the "typ" header property from the JWS.
     *
     * See <a href='https://datatracker.ietf.org/doc/html/rfc7515#section-4.1.9'>RFC 7515
     * section 4.1.9</a> for more information.
     *
     */

    std::string Type;

    /**
     * Returns the SHA-1 thumbprint of the leaf certificate in the getCertificateChain.
     *
     * See <a href='https://datatracker.ietf.org/doc/html/rfc7515#section-4.1.7'>RFC 7515
     * section 4.1.7</a> for more information.
     *
     */
    std::string CertificateThumbprint;

    /**
     * Returns the SHA-256 thumbprint of the leaf certificate in the getCertificateChain.
     *
     * See <a href='https://datatracker.ietf.org/doc/html/rfc7515#section-4.1.8'>RFC 7515
     * section 4.1.8</a> for more information.
     *
     */
    std::string CertificateSha256Thumbprint;

    /**
     * Retrieve the issuer of the attestation token. The issuer corresponds to the "iss" claim
     * in a Json Web Token. See 
     * <a href="https://datatracker.ietf.org/doc/html/rfc7519#section-4.1.1">RFC 7519 section 4.1.1</a>
     * for more information.
     *
     * The issuer will always be the same as the attestation service instance endpoint URL.
     */

    std::string Issuer;

    /**
     * Returns the signing certificate chain as an AttestationSigner.
     *
     * See <a href='https://datatracker.ietf.org/doc/html/rfc7515#section-4.1.6'>RFC 7515
     * section 4.1.6</a> for more information.
     *
     */
    std::vector<std::string> X509CertificateChain;

    /**
     * Returns the signer for this token if the caller provided a JSON Web Key.
     *
     * See <a href='https://datatracker.ietf.org/doc/html/rfc7515#section-4.1.3'>RFC 7515
     * section 4.1.3</a> for more information.
     *
     */
  };

  /** An AttestationToken represents an RFC 7515 JSON Web Token returned from the attestation
   * service.
   * <typeparam name="T"></typeparam> The type which represents the body of the attestation token.
   */
  template <typename T> struct AttestationToken
  {
    /// The raw token returned by the attestation service.
    std::string RawToken;
    /// The decoded header of the raw token returned by the attestation service.
    std::string RawHeader;
    /// The decoded body of the raw token returned by the attestation service.
    std::string RawBody;
    T Body;
    AttestationTokenHeader Header;
  };

  /// An AttestationSigningKey represents a pair of signing keys and certificates.
  struct AttestationSigningKey
  {
    std::string SigningPrivateKey; /// A PEM encoded RSA or ECDSA private key which will be used to
                                   /// sign an attestation token.
    std::string SigningCertificate; /// A PEM encoded X.509 certificate which will be sent to the
                                    /// attestation service to validate an attestation token. The
                                    /// public key embedded in the certificate MUST be the public
                                    /// key of the SigningPrivateKey.
  };
}}} // namespace Azure::Security::Attestation
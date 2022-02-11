// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/attestation/attestation_client_models.hpp"
#include "azure/attestation/attestation_client_options.hpp"
#include <azure/core/context.hpp>
#include <azure/core/url.hpp>
#include <shared_mutex>
#include <string>

namespace Azure { namespace Core { namespace Http { namespace _internal {
  class HttpPipeline;
}}}} // namespace Azure::Core::Http::_internal

namespace Azure { namespace Security { namespace Attestation {
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
    Azure::Nullable<Models::AttestationSigner> Key;

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
     * @brief      The elements of the raw token which will be signed by the Signature.
     */
    std::string SignedElements;

    /**
     * @brief  Signature (if present) for the attestation token.
     */
    std::vector<uint8_t> Signature;

    /**
     * @brief RFC 7515 header properties.
     */
    AttestationTokenHeader Header;

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
     * @brief     The deserialized body of the attestation token.
     *
     */
    T Body;
  };

  /**
   *
   * The AttestationAsyncClient implements the functionality required by the "Attest" family of
   * APIs.
   *
   * An enclave (or Trusted Execution Environment) is a chunk of code that is isolated from the host
   * (think: "encrypted VM" or "encrypted container"). But there's one key attribute of the enclave:
   * It is encrypted.That means that
   * if data is sent from the enclave, there is no way of knowing that the data came from the
   * enclave.
   *
   * And even worse, there is no way of securely communicating with the enclave (since the enclave
   * is fully isolated from the host, all information passed into the enclave has to go through its
   * host first).
   *
   * To solve the communication problem, the Attest API can be used to facilitate what is
   * known as the "Secure Key Release" (SKR) protocol.
   *
   * There are 4 parties involved in an attestation operation:
   *
   * - The host (which hosts the enclave)
   * - The enclave (which is the enclave :) - encrypted, nobody can see what goes on inside it),
   * - The "verifier" which verifies the evidence from the enclave (this is the attestation service)
   * and generates a token which can be received by a relying party, and
   * - The "relying party" which will interpret the token from the service. For the Secure Key
   * Release Protocol, this is the entity which wishes to communicate with the enclave.
   *
   *   It's possible that all these parties are on the same computer, it's possible they're on
   * multiple computers.<br> It's possible that the host is also the relying party. It's possible
   * that the relying party is a component like Azure Managed HSM.
   *
   * There are three primary pieces of data received by the service for the Attest family of APIs.
   * All of them are arrays of bytes, and all of them originate from code running in the enclave
   * (thus they need to be treated as opaque arrays of bytes by the SDK):
   *
   * -# Evidence. For Intel SGX enclaves, this has two forms, either an SGX 'Quote' or an
   * OpenEnclave 'Report'. It is required for attestation operations.
   * -# InitTimeData - This is data which is specified at Initialization Time. It is optional
   * (and not currently supported on all enclave types in Azure)
   * -# RunTimeData - this is data which is specified at the time the quote is generated (at
   * "runtime"). It is optional, but required for the Secure Key Release protocol.
   *
   * The Evidence is cryptographically signed by a known authority (for Intel SGX Quotes or
   * OpenEnclave reports, this is a key owned by Intel which represents that the SGX enclave is
   * valid and can be trusted).<br> The core idea for all attestation operations is to take
   * advantage of a region within the Evidence which is controlled by enclave. For SGX Enclaves,
   * this is the 64 bytes of "user data" contained within SGX quote.
   *
   * For the Secure Key Release protocol, code inside the enclave generates an asymmetric key and
   * serializes the public key into a byte buffer. It then calculates the SHA256 hash of the
   * serialized key and creates a quote containing that SHA256 hash. We now have a cryptographically
   * validated indication that the contents of the byte buffer was known inside the enclave.
   *
   * The enclave then hands the byte buffer and the quote to its host. The host sends the quote and
   * byte buffer as the "RunTime Data" to the via the {@link
   * AttestationAsyncClient#attestSgxEnclave(BinaryData)}  or
   * {@link AttestationAsyncClient#attestOpenEnclave} API. Assuming the byte buffer and quote are
   * valid, and the quote contains the hash of the byte buffer, the attestation service responds
   * with an {@link AttestationToken} signed by the attestation service, whose body is an {@link
   * AttestationResult}.
   *
   * The token generated also includes the contents of the InitTimeData and/or RunTimeData if it was
   * provided in the Attest API call.
   *
   * The host then sends the token to the relying party.  The relying party verifies the token
   * and verifies the claims within the token indicate that the enclave is the correct enclave.
   * It then takes the key from the token and uses it to encrypt the data to be sent to the
   * enclave and sends that back to the host, which passes it into the enclave.
   *
   *
   * That completes the secure key release protocol.
   *
   * When the Attestation Token is generated by the attestation service, as mentioned, it contains
   * the InitTime and RunTime data.
   *
   * There are two possible representations for RunTime Data in the attestation token, depending on
   * the requirements of the relying party:<br> The first is as JSON formatted data. That can be
   * convenient if the relying party expects to receive its public key as a JSON Web Key The second
   * is as a binary blob of data. That is needed if either the data sent by the enclave isn't a JSON
   * object - for instance, if the RunTime data contained an asymmetric key which is formatted as a
   * PEM encoded key, it should be interpreted as a binary blob
   *
   * If you ask for the RunTime data to be included in the token as binary, then it will be
   * base64url encoded in the "x-ms-maa-enclavehelddata" claim in the output token (the
   * {@link AttestationResult#getEnclaveHeldData()} property).
   *
   * If you ask for the RunTime data to be included in the token as JSON, then it will be included
   * in the "x-ms-maa-runtimeClaims" claim in the output token (the {@link
   * AttestationResult#getRuntimeClaims()} property).
   *
   * In addition to the Attest APIs, the {@link AttestationClient} object also contains helper APIs
   * which can be used to retrieve the OpenId Metadata document and signing keys from the service.
   *
   *
   * The OpenId Metadata document contains properties which describe the attestation service.
   *
   * The Attestation Signing Keys describe the keys which will be used to sign tokens generated by
   * the attestation service. All tokens emitted by the attestation service will be signed by one
   * of the certificates listed in the attestation signing keys.
   *
   */

  class AttestationClient final {
  private:
    Azure::Core::Url m_endpoint;
    std::string m_apiVersion;
    std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> m_pipeline;
    AttestationTokenValidationOptions m_tokenValidationOptions;

    mutable std::shared_timed_mutex m_sharedStateLock;
    mutable std::vector<Models::AttestationSigner> m_attestationSigners;

    void CacheAttestationSigners(Azure::Core::Context const& context) const;

  public:
    /**
     * @brief Destructor.
     *
     */
    virtual ~AttestationClient() = default;

    /**
     * @brief Construct a new Attestation Client object
     *
     * @param endpoint The URL address where the client will send the requests to.
     * @param credential OPTIONAL The authentication method to use (required for TPM attestation).
     * @param options The options to customize the client behavior.
     */
    explicit AttestationClient(
        std::string const& endpoint,
        std::shared_ptr<Core::Credentials::TokenCredential const> credential,
        AttestationClientOptions options = AttestationClientOptions());

    explicit AttestationClient(
        std::string const& endpoint,
        AttestationClientOptions options = AttestationClientOptions())
        : AttestationClient(endpoint, nullptr, options)
    {
    }

    /**
     * @brief Returns the API version the client was configured with.
     *
     * @returns The API version used when communicating with the attestation service.
     */
    std::string const& ClientVersion() const { return m_apiVersion; }

    /**
     * @brief Construct a new Key Client object from another key client.
     *
     * @param attestationClient An existing attestation client.
     */
    explicit AttestationClient(AttestationClient const& attestationClient)
        : m_endpoint(attestationClient.m_endpoint), m_apiVersion(attestationClient.m_apiVersion),
          m_pipeline(attestationClient.m_pipeline),
          m_tokenValidationOptions(attestationClient.m_tokenValidationOptions){};

    /**
     * Retrieves metadata about the attestation signing keys in use by the attestation service.
     *
     * Retrieve the OpenID metadata for this attestation service instance..
     *
     * @return an {@link AttestationOpenIdMetadata} containing metadata about the specified service
     * instance.
     */
    Response<Models::AttestationOpenIdMetadata> GetOpenIdMetadata(
        Azure::Core::Context const& context = Azure::Core::Context::ApplicationContext) const;

    /**
     * @brief Retrieve the attestation signing certificates for this attestation instance.
     *
     * @returns Attestation Metadata.
     */
    Response<std::vector<Models::AttestationSigner>> GetAttestationSigningCertificates(
        Azure::Core::Context const& context = Azure::Core::Context::ApplicationContext) const;

    /**
     * @brief Attest an SGX enclave, returning an attestation token representing the result
     * of the attestation operation.
     *
     * @param sgxQuoteToAttest - SGX Quote to be validated by the attestation service.
     * @param options - Options to the attestation request (runtime data, inittime data, etc).
     * @param context - Context for the operation.
     *
     * @returns Response<{@link AttestationToken}<{@link AttestationResult}>> - The result of the
     * attestation operation
     */
    Response<AttestationToken<Models::AttestationResult>> AttestSgxEnclave(
        std::vector<uint8_t> const& sgxQuoteToAttest,
        AttestOptions options = AttestOptions(),
        Azure::Core::Context const& context = Azure::Core::Context::ApplicationContext) const;

    /**
     * @brief Attest an OpenEnclave report, returning an attestation token representing the result
     * of the attestation operation.
     *
     * @param openEnclaveReportToAttest - OpenEnclave Report to be validated by the attestation
     * service.
     * @param options - Options to the attestation request (runtime data, inittime data, etc).
     * @param context - Context for the operation.
     *
     * @returns Response<AttestationToken<AttestationResult>> - The result of the attestation
     * operation
     */
    Response<AttestationToken<Models::AttestationResult>> AttestOpenEnclave(
        std::vector<uint8_t> const& openEnclaveReportToAttest,
        AttestOptions options = AttestOptions(),
        Azure::Core::Context const& context = Azure::Core::Context::ApplicationContext) const;
  };

}}} // namespace Azure::Security::Attestation

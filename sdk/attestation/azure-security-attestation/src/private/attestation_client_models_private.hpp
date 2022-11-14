//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Attestation client model support classes and functions.
 *
 * This file contains private classes used to support public model types.
 *
 */

#pragma once

#include "azure/attestation/attestation_client_models.hpp"
#include "azure/attestation/attestation_client_options.hpp"
#include <azure/core/nullable.hpp>
#include <memory>
#include <string>
#include <vector>

namespace Azure {
  namespace Security {
    namespace Attestation {
      namespace Models {
        namespace _detail {

  /**
   * @brief Represents an <a href='https://datatracker.ietf.org/doc/html/rfc7517'>RFC 7517</a> JSON
   * Web Key.
   */
  struct JsonWebKey final
  {
    /**
     * @brief JWK 'kid'. See <a href='https://datatracker.ietf.org/doc/html/rfc7517#section-4.5'>RFC
     * 7517 section 4.5</a>
     */
    Azure::Nullable<std::string> Kid;
    /**
     * @brief JWK 'kty'. See <a href='https://datatracker.ietf.org/doc/html/rfc7517#section-4.1'>RFC
     * 7517 section 4.1</a>
     */
    Azure::Nullable<std::string> Kty;
    /**
     * @brief JWK 'alg'. See <a href='https://datatracker.ietf.org/doc/html/rfc7517#section-4.4'>RFC
     * 7517 section 4.4</a>
     */
    Azure::Nullable<std::string> Alg;
    /**
     * @brief JWK 'x5c'. See <a href='https://datatracker.ietf.org/doc/html/rfc7517#section-4.7'>RFC
     * 7517 section 4.7</a>
     */
    Azure::Nullable<std::vector<std::string>> X5c;
    /**
     * @brief JWK 'x5t'. See <a href='https://datatracker.ietf.org/doc/html/rfc7517#section-4.8'>RFC
     * 7517 section 4.8</a>
     */
    Azure::Nullable<std::string> X5t;
    /**
     * @brief JWK 'use'. See <a href='https://datatracker.ietf.org/doc/html/rfc7517#section-4.2'>RFC
     * 7517 section 4.2</a>
     */
    Azure::Nullable<std::string> Use;
    /**
     * @brief JWK 'x5t#S256'. See
     * <a href='https://datatracker.ietf.org/doc/html/rfc7517#section-4.9'>RFC 7517 section 4.9</a>
     */
    Azure::Nullable<std::string> X5t256;
    /**
     * @brief JWK 'x5u'. See <a href='https://datatracker.ietf.org/doc/html/rfc7517#section-4.6'>RFC
     * 7517 section 4.6</a>
     */
    Azure::Nullable<std::string> X5u;
    /**
     * @brief JWK 'key_ops'. See
     * <a href='https://datatracker.ietf.org/doc/html/rfc7517#section-4.3'>RFC 7517 section 4.3</a>
     */
    Azure::Nullable<std::vector<std::string>> KeyOps;

    // RSA Public Keys (alg == 'RS256' | 'RS384' | 'RS512').
    /**
     * @brief RFC 7518 "n" (modulus) parameter. See
     * <a href='https://rfc-editor.org/rfc/rfc7518#section-6.3.1.1'>RFC 7518 section 6.3.1.1</a>
     *
     */
    Azure::Nullable<std::string> N;
    /**
     * @brief RFC 7518 "e" (exponent) parameter. See
     * <a href='https://rfc-editor.org/rfc/rfc7518#section-6.3.1.2'>RFC 7518 section 6.3.1.2</a>
     *
     */
    Azure::Nullable<std::string> E;
    // Private key

    /**
     * @brief RFC 7518 "p" (First Prime factor) parameter. See
     * <a href='https://rfc-editor.org/rfc/rfc7518#section-6.3.2.2'>RFC 7518 section 6.3.2.2</a>
     *
     */
    Azure::Nullable<std::string> P;
    /**
     * @brief RFC 7518 "q" (Second Prime factor) parameter. See
     * <a href='https://rfc-editor.org/rfc/rfc7518#section-6.3.2.3'>RFC 7518 section 6.3.2.3</a>
     */
    Azure::Nullable<std::string> Q;
    /**
     * @brief RFC 7518 "dp" (First Factor CRT Exponent) parameter. See
     * <a href='https://rfc-editor.org/rfc/rfc7518#section-6.3.2.4'>RFC 7518 section 6.3.2.4</a>
     *
     */
    Azure::Nullable<std::string> Dp;
    /**
     * @brief RFC 7518 "dq" (Second Factor CRT Exponent) parameter. See
     * <a href='https://rfc-editor.org/rfc/rfc7518#section-6.3.2.5'>RFC 7518 section 6.3.2.5</a>
     */
    Azure::Nullable<std::string> Dq;
    /**
     * @brief RFC 7518 "qi" (First CRT Coefficient) parameter. See
     * <a href='https://rfc-editor.org/rfc/rfc7518#section-6.3.2.6'>RFC 7518 section 6.3.2.6</a>
     */
    Azure::Nullable<std::string> Qi;
    /**
     * @brief RFC 7518 "oth" (First CRT Coefficient) parameter. See
     * <a href='https://rfc-editor.org/rfc/rfc7518#section-6.3.2.7'>RFC 7518 section 6.3.2.7</a>
     */
    Azure::Nullable<std::string> Oth;

    // ECDSA Public Keys (alg == 'ES256' | 'ES384' | 'ES512').
    /**
     * @brief RFC 7518 "crv" (Curve) parameter. See
     * <a href='https://rfc-editor.org/rfc/rfc7518#section-6.3.1.1'>RFC 7518 section 6.3.1.1</a>
     */
    Azure::Nullable<std::string> Crv;
    /**
     * @brief RFC 7518 "x" (X Coordinate) parameter. See
     * <a href='https://rfc-editor.org/rfc/rfc7518#section-6.3.1.2'>RFC 7518 section 6.3.1.2</a>
     */
    Azure::Nullable<std::string> X;
    /**
     * @brief RFC 7518 "y" (Y Coordinate) parameter. See
     * <a href='https://rfc-editor.org/rfc/rfc7518#section-6.3.1.3'>RFC 7518 section 6.3.1.3</a>
     */
    Azure::Nullable<std::string> Y;

    /**
     * @brief RFC 7518 "d" (Private Exponent) parameter, or (ECC Private Key) parameter
     *
     * See
     * <a href='https://rfc-editor.org/rfc/rfc7518#section-6.3.2.1'>RFC 7518 section 6.3.2.1</a>
     * Or
     * <a href='https://rfc-editor.org/rfc/rfc7518#section-6.2.2.1'>RFC 7518 section 6.2.2.1</a>
     *
     */
    Azure::Nullable<std::string> D; // Shared with RSA Public Key.
  };

  /**
   * @brief Represents a <a href='https://datatracker.ietf.org/doc/html/rfc7517'>RFC 7517</a> JSON
   * Web Key Set.
   */
  struct JsonWebKeySet final
  {
    /**
     * @brief RFC 7517 "keys" parameter.
     * See <a href='https://datatracker.ietf.org/doc/html/rfc7517#section-5.1'>RFC 7517
     * Section 5.1</a> for more information.
     *
     */
    std::vector<JsonWebKey> Keys;
  };

  /**
   * @brief Internal implementation class implementing private methods for public model type
   * AttestationSigner.
   */
  class AttestationSignerInternal {
    AttestationSigner m_signer;

  public:
    AttestationSignerInternal(JsonWebKey const& jwk);

    operator AttestationSigner&&() { return std::move(m_signer); }
    static std::string SerializeToJson(AttestationSigner const& signer);
  };

  /**
   * @brief Internal model type representing the payload sent to the attestation service for the
   * AttestSgxEnclave API call.
   *
   */
  struct AttestSgxEnclaveRequest
  {
    /**
     * @brief SGX Quote to be attested.
     */
    std::vector<uint8_t> Quote;
    /**
     * @brief Data presented at the time that the SGX enclave was created (not supported on
     * Coffeelake processors.)
     */
    Azure::Nullable<AttestationData> InitTimeData;
    /**
     * @brief Data presented at the time that the quote was generated.
     */
    Azure::Nullable<AttestationData> RunTimeData;
    /**
     * @brief Draft policy used during attestation calls.
     */
    Azure::Nullable<std::string> DraftPolicyForAttestation;
    /**
     * @brief Nonce used in attestation operation.
     */
    Azure::Nullable<std::string> Nonce;
  };
  /**
   * @brief Internal model type representing the payload sent to the attestation service for the
   * AttestOpenEnclave API call.
   *
   */
  struct AttestOpenEnclaveRequest
  {
    /**
     * @brief OpenEnclave report to be attested.
     */
    std::vector<uint8_t> Report;
    /**
     * @brief Data presented at the time that the SGX enclave was created (not supported on
     * Coffeelake processors.)
     */
    Azure::Nullable<AttestationData> InitTimeData;
    /**
     * @brief Data presented at the time that the report was generated.
     */
    Azure::Nullable<AttestationData> RunTimeData;
    /**
     * @brief Draft policy used during attestation calls.
     */
    Azure::Nullable<std::string> DraftPolicyForAttestation;
    /**
     * @brief Nonce used in attestation operation.
     */
    Azure::Nullable<std::string> Nonce;
  };

  /**
   * @brief Result of a GetPolicy, SetPolicy, or ResetPolicy operation.
   */
  struct PolicyResult
  {
    /**
     * @brief Result of a modification.
     */
    Azure::Nullable<std::string> PolicyResolution;

    /**
     * @brief The SHA256 hash of the policy object which was received by the service.
     */
    Azure::Nullable<std::string> PolicyTokenHash;
    /**
     * @brief A JSON Web Key containing the signer of the policy token.
     */
    Azure::Nullable<JsonWebKey> PolicySigner;
    /**
     * @brief The policy token returned by the service.
     */
    Azure::Nullable<std::string> PolicyToken;
  };

  /**
   * @brief A StoredAttestationPolicy is a JWS object which represents a (possibly signed)
   * attestation policy received by the attestation service.
   */
  struct StoredAttestationPolicy
  {
    /** @brief UTF-8 encoded representation of the attestation policy.
     */
    Azure::Nullable<std::vector<uint8_t>> AttestationPolicy;
  };

  /**
   * @brief Result of a GetPolicy, SetPolicy, or ResetPolicy operation.
   */
  struct GetIsolatedModeCertificatesResult
  {
    /**
     * @brief Result of a modification.
     */
    Azure::Nullable<JsonWebKeySet> PolicyCertificates;
  };

  struct IsolatedModeCertificateBody
  {
    JsonWebKey policyCertificate;
  };

  struct ModifyIsolatedModeCertificatesResult
  {
    Azure::Nullable<std::string> CertificateThumbprint;
    Azure::Nullable<std::string> CertificateResolution;
  };

}}}}} // namespace Azure::Security::Attestation::Models::_detail

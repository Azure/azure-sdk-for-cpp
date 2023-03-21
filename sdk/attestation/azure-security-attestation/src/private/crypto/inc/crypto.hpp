// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once
#include <ctime>
#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Security { namespace Attestation { namespace _detail {
  /**
   * @brief THe Cryptography class provides a set of basic cryptographic functions required
   * by the attestation service client implementation and test collateral.
   *
   * It contains two subclasses: {@link Cryptography::AsymmetricKey}, which represents an Asymmetric
   * key and
   * {@link Cryptography::X509Certificate} which represents an X.509 certificate.
   *
   *
   */
  class Cryptography {
  public:
    /**
     * @brief The AsymmetricKey class expresses a set of basic functionality asymmetric key
     * signatures.
     *
     */
    class AsymmetricKey {

    public:
      virtual ~AsymmetricKey() {}

      /**
       * @brief Verifies an Asymmetric Key signature. Valid for all asymmetric keys.
       *
       * @param payload The payload to verify.
       * @param signature The signature of this payload, signed with a private key
       * @return true The signature was valid.
       * @return false The signature did not match the payload.
       */
      virtual bool VerifySignature(
          std::vector<uint8_t> const& payload,
          std::vector<uint8_t> const& signature) const = 0;
      /**
       * @brief Signs a buffer with an Asymmetric private key. Only valid for private asymmetric
       * keys.
       *
       * @param bufferToSign The buffer to be signed.
       * @return std::vector<uint8_t> Returns the signature of that buffer, signed with the private
       * key.
       */
      virtual std::vector<uint8_t> SignBuffer(std::vector<uint8_t> const& bufferToSign) const = 0;
      /**
       * @brief Exports the current asymmetric key as a private key (only valid for private
       * asymmetric keys)
       *
       * @return std::string The PEM encoded private key for this key.
       */
      virtual std::string ExportPrivateKey() = 0;
      /**
       * @brief Exports the current asymmetric key as a public key (valid for all  asymmetric keys).
       *
       * @return std::string The PEM encoded public key for this key.
       */
      virtual std::string ExportPublicKey() = 0;
    };

    /**
     * @brief Represents an X.509 certificate.
     *
     */
    class X509Certificate {
    private:
      // Delete the copy constructor and assignment operator for this certificate.
      X509Certificate(const X509Certificate&) = delete;
      X509Certificate& operator=(const X509Certificate&) = delete;

    protected:
      X509Certificate() {}

    public:
      virtual ~X509Certificate() {}
      /**
       * @brief Returns the public key associated with this X.509 certificate.
       *
       * @return std::unique_ptr<AsymmetricKey> The public key for the certificate.
       */
      virtual std::unique_ptr<AsymmetricKey> GetPublicKey() const = 0;
      /**
       * @brief Exports the current certificate as a PEM encoded string.
       *
       * @return std::string PEM encoded representation of the X.509 certificate.
       */
      virtual std::string ExportAsPEM() const = 0;
      /**
       * @brief Exports the current certificate as a Base64 encoded string.
       *
       * @return std::string Base64 encoded representation of the X.509 certificate.
       */
      virtual std::string ExportAsBase64() const = 0;
      /**
       * @brief Get the Subject Name of the X.509 certificate.
       *
       * @return std::string Certificate subject name.
       */
      virtual std::string GetSubjectName() const = 0;
      /**
       * @brief Get the Issuer Name of the X.509 certificate
       *
       * @return std::string Certificate issuer name.
       */
      virtual std::string GetIssuerName() const = 0;
      /**
       * @brief Get the Algorithm for this certificate, either "RSA" or "EC".
       *
       * @return std::string String representation of the certificate key algorithm, suitable for
       * use within a JSON Web Key.
       */
      virtual std::string GetAlgorithm() const = 0;
      /**
       * @brief Get the Key Type for this certificate
       *
       * @return std::string String representation of the key type for the certificate, suitable for
       * use within a JSON Web Key
       */
      virtual std::string GetKeyType() const = 0;
      /**
       * @brief Get the Thumbprint for this certificate.
       *
       * @details The Thumbprint of a certificate is a hex encoded SHA1 hash of the DER
       * serialization of the certificate. It can be used to uniquely identify a certificate.
       *
       * @return std::string Certificate thumbprint.
       */
      virtual std::string GetThumbprint() const = 0;
    };

    /**
     * @brief Convert a base64 encoded value to the PEM encoded equivalent.
     *
     * @param base64 base64 encoded value.
     * @param pemType Type of the object to be converted - typically "CERTIFICATE", "PRIVATE KEY",
     * or "PUBLIC KEY".
     * @return std::string PEM encoded representation of the base64 value.
     */
    static std::string PemFromBase64(std::string const& base64, std::string const& pemType)
    {
      std::string rv;
      rv += "-----BEGIN ";
      rv += pemType;
      rv += "-----\r\n ";
      std::string encodedValue(base64);

      // Insert crlf characters every 80 characters into the base64 encoded key to make it
      // prettier.
      size_t insertPos = 80;
      while (insertPos < encodedValue.length())
      {
        encodedValue.insert(insertPos, "\r\n");
        insertPos += 82; /* 80 characters plus the \r\n we just inserted */
      }

      rv += encodedValue;
      rv += "\r\n-----END ";
      rv += pemType;
      rv += "-----\r\n ";
      return rv;
    }

    /**
     * @brief Create an asymmetric key implementing the RSA asymmetric algorithm
     *
     * @param keySizeInBytes Specifies the size of the key to be created.
     * @return std::unique_ptr<AsymmetricKey> Returns the newly created asymmetric key.
     */
    static std::unique_ptr<AsymmetricKey> CreateRsaKey(size_t keySizeInBytes);
    /**
     * @brief Create an asymmetric key implementing the ECDSA asymmetric algorithm.
     *
     * @return std::unique_ptr<AsymmetricKey> Returns the newly created asymmetric key.
     */
    static std::unique_ptr<AsymmetricKey> CreateEcdsaKey();
    /**
     * @brief Imports a PEM encoded public key (either RSA or ECDSA).
     *
     * @param pemEncodedString The PEM encoded serialized key.
     * @return std::unique_ptr<AsymmetricKey> Returns an asymmetric key corresponding to the
     * pemEncodedString.
     */
    static std::unique_ptr<AsymmetricKey> ImportPublicKey(std::string const& pemEncodedString);
    /**
     * @brief Imports a PEM encoded private key (either RSA or ECDSA).
     *
     * @param pemEncodedString The PEM encoded serialized key.
     * @return std::unique_ptr<AsymmetricKey> Returns an asymmetric key corresponding to the
     * pemEncodedString.
     */
    static std::unique_ptr<AsymmetricKey> ImportPrivateKey(std::string const& pemEncodedString);

    /**
     * @brief Creates a self-signed X.509 certificate associated with the specified private key.
     *
     * @param key Asymmetric private key to sign the self-signed X.509 certificate.
     * @param certificateSubject The Subject DN (and Issuer DN) for the specified certificate.
     * @return std::unique_ptr<X509Certificate> Returns an X.509 certificate for the specified key.
     */
    static std::unique_ptr<X509Certificate> CreateX509CertificateForPrivateKey(
        std::unique_ptr<AsymmetricKey> const& key,
        std::string const& certificateSubject);
    /**
     * @brief Imports a PEM encoded X.509 certificate.
     *
     * @param pemEncodedCertificate PEM encoded X.509 certificate.
     * @return std::unique_ptr<X509Certificate> Returns an X.509 certificate decoded from the
     * pemEncodedCertificate parameter.
     */
    static std::unique_ptr<X509Certificate> ImportX509Certificate(
        std::string const& pemEncodedCertificate);
  };
}}}} // namespace Azure::Security::Attestation::_detail

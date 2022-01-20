// Copyright (c) Microsoft Corp. All Rights Reserved
#pragma once
#include <azure/core/internal/json/json.hpp>
#include "cryptokeys.hpp"

namespace Azure {
  namespace Security {
    namespace Attestation {
      namespace _private {
        namespace Cryptography {

  class X509Certificate {
  private:
    X509Certificate(const X509Certificate&) = delete;
    X509Certificate& operator=(const X509Certificate&) = delete;

  protected:
    X509Certificate() {}

  public:
    virtual ~X509Certificate() {}
    virtual bool ComparePublicKey(_In_z_ const char* expectedRootCertificate) const = 0;
    virtual std::unique_ptr<AsymmetricKey> GetPublicKey() const = 0;

    // X.509 Extension support.
    virtual bool HasExtension(_In_z_ const char* extensionOid) const = 0;
    virtual std::unique_ptr<std::vector<uint8_t>> FindExtension(
        _In_z_ const char* extensionOid) const = 0;
    virtual std::vector<uint8_t> FindOctetStringOidsInExtension(
        _In_ std::vector<uint8_t> const & extensionBuffer,
        _In_z_ char const* elementOid) const = 0;
    virtual uint32_t FindIntegerOidInExtension(
        _In_ std::vector<uint8_t> const& extensionBuffer,
        _In_z_ const char* elementOid) const = 0;
    virtual std::unique_ptr<std::vector<uint8_t>> FindOidSequenceInExtension(
        _In_ std::vector<uint8_t> const& extensionBuffer,
        _In_z_ const char* elementOid) const = 0;
    virtual bool GetExtensionBoolValue(
        _In_ std::vector<uint8_t> const& asn1encodedextensionvalue) const = 0;
    virtual uint32_t GetExtensionIntValue(
        _In_ std::vector<uint8_t> const & asn1encodedextensionvalue) const = 0;
    virtual std::string GetExtensionStringValue(
        _In_ std::vector<uint8_t> const & asn1encodedextensionvalue) const = 0;
    virtual std::unique_ptr<std::vector<uint8_t>> GetExtensionOctetStringValue(
        _In_ std::vector<uint8_t> const & asn1encodedextensionvalue) const = 0;

    // Certificate export support.
    virtual std::string ExportAsPEM() const = 0;
    virtual std::string ExportAsBase64() const = 0;
    virtual std::vector<uint8_t> ExportAsBinary() const = 0;
    virtual Azure::Core::Json::_internal::json ExportAsJwk() const = 0;

    // Certificate derivation support
    virtual void SetSigningKey(_In_ const AsymmetricKey& signingKey) = 0;
    virtual std::unique_ptr<X509Certificate> DeriveNewCertificate(
        _In_ const AsymmetricKey& newCertificateSigningKey,
        _In_z_ const char* newCertificateSubjectName,
        bool isLeafCertificate) const = 0;

    // Certificate Information.

    // Returns the RFC 7517 "kty" value for this certificate
    // (https://datatracker.ietf.org/doc/html/rfc7517#section-4.1).
    virtual std::string GetKeyType() const = 0;
    // Returns the RFC 7517 "alg" value for this certificate
    // (https://datatracker.ietf.org/doc/html/rfc7517#section-4.4).
    virtual std::string GetKeyAlgorithm() const = 0;
    // Returns the RFC 7517 "use" value for this certificate
    // (https://datatracker.ietf.org/doc/html/rfc7517#section-4.2).
    virtual std::string GetKeyUse() const = 0;
    virtual std::string GetCrlUrl() const = 0;
    virtual std::string GetSubjectName() const = 0;
    virtual std::string GetIssuerName() const = 0;
    // Returns the SHA1 thumbprint of the specified certificate.
    virtual std::string GetThumbprint() const = 0;
  };

  class X509CertificateExtension {
  private:
    X509CertificateExtension(const X509CertificateExtension&) = delete;
    X509CertificateExtension& operator=(const X509CertificateExtension&) = delete;

  protected:
    X509CertificateExtension() {}

  public:
    virtual ~X509CertificateExtension() {}

    virtual std::vector<uint8_t> const & GetExtensionOid() const = 0;
    virtual std::vector<uint8_t> const & GetExtensionData() const = 0;
    virtual bool IsCritical() const = 0;
  };

}}}}} // namespace Azure::Security::Attestation::_private::Crypto

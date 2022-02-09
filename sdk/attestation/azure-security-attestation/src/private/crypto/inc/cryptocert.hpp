// Copyright (c) Microsoft Corp. All Rights Reserved
#pragma once
#include "cryptokeys.hpp"

namespace Azure { namespace Security { namespace Attestation { namespace _detail {
  namespace Cryptography {

    class X509Certificate {
    private:
      X509Certificate(const X509Certificate&) = delete;
      X509Certificate& operator=(const X509Certificate&) = delete;

    protected:
      X509Certificate() {}

    public:
      virtual ~X509Certificate() {}
      virtual std::unique_ptr<AsymmetricKey> GetPublicKey() const = 0;
      virtual std::string ExportAsPEM() const = 0;
      virtual std::string ExportAsBase64() const = 0;
      virtual std::string GetSubjectName() const = 0;
      virtual std::string GetIssuerName() const = 0;
      virtual std::string GetAlgorithm() const = 0;
      virtual std::string GetKeyType() const = 0;
      virtual std::string GetThumbprint() const = 0;
    };
}}}}} // namespace Azure::Security::Attestation::_detail::Cryptography

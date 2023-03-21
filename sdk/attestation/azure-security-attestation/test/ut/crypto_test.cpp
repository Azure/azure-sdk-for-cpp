// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "../../src/private/crypto/inc/crypto.hpp"
#include "crypto_test_collateral.hpp"
#include <azure/core/test/test_base.hpp>
#include <gtest/gtest.h>
#include <random>

namespace Azure { namespace Security { namespace Attestation { namespace Test {
  using namespace Azure::Security::Attestation::_detail;
  using namespace Azure::Core::Diagnostics::_internal;
  using namespace Azure::Core::Diagnostics;

  void ImportKeyTest(std::string const& pemPrivateKey, std::string const& pemPublicKey)
  {
    // Verify we can round trip the private key.
    {
      auto privateKey = Cryptography::ImportPrivateKey(pemPrivateKey);
      auto exportedPrivateKey = privateKey->ExportPrivateKey();

      EXPECT_EQ(exportedPrivateKey, pemPrivateKey);
    }
    // Verify we can round trip the public key.
    {
      auto publicKey = Cryptography::ImportPublicKey(pemPublicKey);
      auto exportedPublicKey = publicKey->ExportPublicKey();

      EXPECT_EQ(exportedPublicKey, pemPublicKey);
    }
    // Exported public key of private key matches.
    {
      auto privateKey = Cryptography::ImportPrivateKey(pemPrivateKey);
      auto exportedPublicKey = privateKey->ExportPublicKey();

      EXPECT_EQ(exportedPublicKey, pemPublicKey);
    }
  }
  TEST(CryptoTests, ImportKeyTest)
  {
    ImportKeyTest(
        CryptoTestCollateral::TestRsaPrivateKey(), CryptoTestCollateral::TestRsaPublicKey());
    ImportKeyTest(
        CryptoTestCollateral::TestEcdsPrivateKey(), CryptoTestCollateral::TestEcdsPublicKey());
  }
  TEST(CryptoTests, CreateRsaKey)
  {
    auto privateKey = Cryptography::CreateRsaKey(2048);
    std::string exportedPrivateKey = privateKey->ExportPrivateKey();

    EXPECT_EQ(0ul, exportedPrivateKey.find("-----BEGIN PRIVATE KEY-----"));

    auto importedKey = Cryptography::ImportPrivateKey(exportedPrivateKey);

    std::string exportedPublicKey = privateKey->ExportPublicKey();

    EXPECT_EQ(0ul, exportedPublicKey.find("-----BEGIN PUBLIC KEY-----"));
    auto importedPublicKey = Cryptography::ImportPublicKey(exportedPublicKey);

    EXPECT_THROW(Cryptography::ImportPrivateKey(exportedPublicKey), std::runtime_error);

    Azure::Core::Diagnostics::_internal::Log::Write(
        Logger::Level::Informational, exportedPrivateKey);

    Azure::Core::Diagnostics::_internal::Log::Write(
        Logger::Level::Informational, exportedPublicKey);
  }

  TEST(CryptoTests, SignRsaBuffer)
  {
    auto privateKey = Cryptography::CreateRsaKey(2048);

    std::vector<uint8_t> signaturePayload{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    std::vector<uint8_t> signature = privateKey->SignBuffer(signaturePayload);

    EXPECT_TRUE(privateKey->VerifySignature(signaturePayload, signature));

    auto exportedPublicKey = privateKey->ExportPublicKey();
    auto publicKey = Cryptography::ImportPublicKey(exportedPublicKey);
    EXPECT_TRUE(publicKey->VerifySignature(signaturePayload, signature));

    // Tamper the signature, the validation should fail.
    signature[5] ^= 0x55;
    EXPECT_FALSE(publicKey->VerifySignature(signaturePayload, signature));
    // Undo tampering the signature.
    signature[5] ^= 0x55;

    // Now tamper the payload, it should also fail.
    signaturePayload[5] ^= 0x55;
    EXPECT_FALSE(publicKey->VerifySignature(signaturePayload, signature));
  }

  TEST(CryptoTests, CreateEcdsaKey)
  {
    auto privateKey = Cryptography::CreateEcdsaKey();
    std::string exportedPrivateKey = privateKey->ExportPrivateKey();

    EXPECT_EQ(0ul, exportedPrivateKey.find("-----BEGIN PRIVATE KEY-----"));
    auto importedKey = Cryptography::ImportPrivateKey(exportedPrivateKey);

    std::string exportedPublicKey = privateKey->ExportPublicKey();

    EXPECT_EQ(0ul, exportedPublicKey.find("-----BEGIN PUBLIC KEY-----"));
    auto importedPublicKey = Cryptography::ImportPublicKey(exportedPublicKey);

    EXPECT_THROW(Cryptography::ImportPrivateKey(exportedPublicKey), std::runtime_error);

    Azure::Core::Diagnostics::_internal::Log::Write(
        Logger::Level::Informational, exportedPrivateKey);

    Azure::Core::Diagnostics::_internal::Log::Write(
        Logger::Level::Informational, exportedPublicKey);
  }

  TEST(CryptoTests, SignEcdaBuffer)
  {
    auto privateKey = Cryptography::CreateEcdsaKey();

    auto exportedPublicKey = privateKey->ExportPublicKey();
    auto publicKey = Cryptography::ImportPublicKey(exportedPublicKey);

    std::random_device rd;
    std::uniform_int_distribution<> payloaddist(0, 255);
    std::uniform_int_distribution<> countdist(1, 1024);

    auto signIterations(countdist(rd));
    GTEST_LOG_(INFO) << "Signing for " << signIterations << " iterations" << std::endl;

    // Iterate over signing for signIterations signing operations.
    for (auto i = 0; i < signIterations; i += 1)
    {
      // Create a random payload to be signed.
      auto payloadSize(countdist(rd));
      std::vector<uint8_t> signaturePayload(payloadSize);
      for (auto j = 0; j < payloadSize; j += 1)
      {
        signaturePayload[j] = static_cast<uint8_t>(payloaddist(rd));
      }

      std::vector<uint8_t> signature = privateKey->SignBuffer(signaturePayload);

      // Verify the signature using the private key.
      EXPECT_TRUE(privateKey->VerifySignature(signaturePayload, signature));

      // Verify the signature using the public key.
      EXPECT_TRUE(publicKey->VerifySignature(signaturePayload, signature));

      // Tamper the signature, the validation should fail.
      signature[5] ^= 0x55;
      EXPECT_FALSE(publicKey->VerifySignature(signaturePayload, signature));
      // Undo tampering the signature.
      signature[5] ^= 0x55;

      // Now tamper the payload, it should also fail.
      signaturePayload[5 % payloadSize] ^= 0x55;
      EXPECT_FALSE(publicKey->VerifySignature(signaturePayload, signature));
    }
  }

  TEST(CryptoTests, ImportBogusKey)
  {
    const std::string pemEncodedGarbage =
        R"(-----BEGIN UNKNOWN-----
MIIEejCCBCCgAwIBAgIVAKL12jjpSW7HPPHpJIYhFhGrJxJTMAoGCCqGSM49BAMC
MHExIzAhBgNVBAMMGkludGVsIFNHWCBQQ0sgUHJvY2Vzc29yIENBMRowGAYDVQQK
-----END UNKNOWN-----)";

    EXPECT_THROW(Cryptography::ImportPrivateKey(pemEncodedGarbage), std::runtime_error);
    EXPECT_THROW(Cryptography::ImportPublicKey(pemEncodedGarbage), std::runtime_error);
  }

  const std::string pemEncodedCertificate1 =
      R"(-----BEGIN CERTIFICATE-----
MIIEejCCBCCgAwIBAgIVAKL12jjpSW7HPPHpJIYhFhGrJxJTMAoGCCqGSM49BAMC
MHExIzAhBgNVBAMMGkludGVsIFNHWCBQQ0sgUHJvY2Vzc29yIENBMRowGAYDVQQK
DBFJbnRlbCBDb3Jwb3JhdGlvbjEUMBIGA1UEBwwLU2FudGEgQ2xhcmExCzAJBgNV
BAgMAkNBMQswCQYDVQQGEwJVUzAeFw0xODA1MzAxMTMzMDVaFw0yNTA1MzAxMTMz
MDVaMHAxIjAgBgNVBAMMGUludGVsIFNHWCBQQ0sgQ2VydGlmaWNhdGUxGjAYBgNV
BAoMEUludGVsIENvcnBvcmF0aW9uMRQwEgYDVQQHDAtTYW50YSBDbGFyYTELMAkG
A1UECAwCQ0ExCzAJBgNVBAYTAlVTMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE
nXeExAS/8EPvOlT4wrWpL6gLjVJBmodnXK3pSfEPGx3cgSi8s6SQb/uPvYfYVrnf
iIvaIJm0RQA3d8lHEXpZb6OCApQwggKQMB8GA1UdIwQYMBaAFOW7Uo+A+eMzrhms
+mNGeBHzYbukMFgGA1UdHwRRME8wTaBLoEmGR2h0dHBzOi8vY2VydGlmaWNhdGVz
LnRydXN0ZWRzZXJ2aWNlcy5pbnRlbC5jb20vSW50ZWxTR1hQQ0tQcm9jZXNzb3Iu
Y3JsMB0GA1UdDgQWBBSc0sICBn019udzsho6JHcSDAG7FzAOBgNVHQ8BAf8EBAMC
BsAwDAYDVR0TAQH/BAIwADCCAdQGCSqGSIb4TQENAQSCAcUwggHBMB4GCiqGSIb4
TQENAQEEEPN23WzgJbuS+wQQ/edzQHowggFkBgoqhkiG+E0BDQECMIIBVDAQBgsq
hkiG+E0BDQECAQIBBDAQBgsqhkiG+E0BDQECAgIBBDAQBgsqhkiG+E0BDQECAwIB
AjAQBgsqhkiG+E0BDQECBAIBBDAQBgsqhkiG+E0BDQECBQIBATARBgsqhkiG+E0B
DQECBgICAIAwEAYLKoZIhvhNAQ0BAgcCAQAwEAYLKoZIhvhNAQ0BAggCAQAwEAYL
KoZIhvhNAQ0BAgkCAQAwEAYLKoZIhvhNAQ0BAgoCAQAwEAYLKoZIhvhNAQ0BAgsC
AQAwEAYLKoZIhvhNAQ0BAgwCAQAwEAYLKoZIhvhNAQ0BAg0CAQAwEAYLKoZIhvhN
AQ0BAg4CAQAwEAYLKoZIhvhNAQ0BAg8CAQAwEAYLKoZIhvhNAQ0BAhACAQAwEAYL
KoZIhvhNAQ0BAhECAQUwHwYLKoZIhvhNAQ0BAhIEEAQEAgQBgAAAAAAAAAAAAAAw
EAYKKoZIhvhNAQ0BAwQCAAAwFAYKKoZIhvhNAQ0BBAQGAJBuoQAAMA8GCiqGSIb4
TQENAQUKAQAwCgYIKoZIzj0EAwIDSAAwRQIhALfuvysSitsNa18TSDKfBIwXWXFy
qQKwhjIj5sw3iOCKAiAUEIuF2ylJk2KDexNEW7t/zGmnBT0FgCRwdvKAh8S2EQ==
-----END CERTIFICATE-----)";

  TEST(CryptoTests, ImportX509Certificate)
  {
    auto x509cert(Cryptography::ImportX509Certificate(pemEncodedCertificate1));
    EXPECT_NE(nullptr, x509cert);

    EXPECT_EQ(
        "CN=Intel SGX PCK Certificate, O=Intel Corporation, L=Santa Clara, ST=CA, C=US",
        x509cert->GetSubjectName());
    EXPECT_EQ(
        "CN=Intel SGX PCK Processor CA, O=Intel Corporation, L=Santa Clara, ST=CA, C=US",
        x509cert->GetIssuerName());

    std::string exportedCert = x509cert->ExportAsPEM();
    EXPECT_EQ(0ul, exportedCert.find("-----BEGIN CERTIFICATE-----"));

    auto publicKey = x509cert->GetPublicKey();
    EXPECT_NE(nullptr, publicKey.get());
  }

  TEST(CryptoTests, CreateRsaX509Certificate)
  {
    auto privateKey = Cryptography::CreateRsaKey(2048);
    auto x509cert = Cryptography::CreateX509CertificateForPrivateKey(
        privateKey, "CN=Test\\Subject1, O=Microsoft Corporation, L=Redmond, ST=WA, C=US");

    EXPECT_EQ(
        "CN=TestSubject1, O=Microsoft Corporation, L=Redmond, ST=WA, C=US",
        x509cert->GetSubjectName());
    EXPECT_EQ(
        "CN=TestSubject1, O=Microsoft Corporation, L=Redmond, ST=WA, C=US",
        x509cert->GetIssuerName());

    std::string certThumbprint(x509cert->GetThumbprint());
    EXPECT_FALSE(certThumbprint.empty());

    EXPECT_EQ("RSA", x509cert->GetKeyType());
    EXPECT_EQ("RS256", x509cert->GetAlgorithm());
  }
  TEST(CryptoTests, CreateEcdsX509Certificate)
  {
    auto privateKey = Cryptography::CreateEcdsaKey();
    auto x509cert = Cryptography::CreateX509CertificateForPrivateKey(
        privateKey, "CN=ECDSATest\\Subject1, O=Microsoft Corporation, L=Redmond, ST=WA, C=US");

    EXPECT_EQ(
        "CN=ECDSATestSubject1, O=Microsoft Corporation, L=Redmond, ST=WA, C=US",
        x509cert->GetSubjectName());
    EXPECT_EQ(
        "CN=ECDSATestSubject1, O=Microsoft Corporation, L=Redmond, ST=WA, C=US",
        x509cert->GetIssuerName());

    std::string certThumbprint(x509cert->GetThumbprint());
    EXPECT_FALSE(certThumbprint.empty());
    EXPECT_EQ("EC", x509cert->GetKeyType());
    EXPECT_EQ("EC", x509cert->GetAlgorithm());
  }

}}}} // namespace Azure::Security::Attestation::Test

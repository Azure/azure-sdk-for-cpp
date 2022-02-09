// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/test/test_base.hpp"
#include <gtest/gtest.h>

#include "../../src/private/crypto/inc/crypto.hpp"
#include <random>

// cspell: words ECDS
namespace Azure { namespace Security { namespace Attestation { namespace Test {
  using namespace Azure::Security::Attestation::_internal::Cryptography;
  using namespace Azure::Core::Diagnostics::_internal;
  using namespace Azure::Core::Diagnostics;

  // Test ephemeral ECDS public and private keys for test purposes.

  constexpr char const* testEcdsPrivateKey(R"(-----BEGIN PRIVATE KEY-----
MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQg6g5VraUfx16neNxT
UodoJBPFB3WzspMM5icOPLnd9h6hRANCAATWmDp5WrcaZZQQZPhI2asPDYJFbnY5
MTfWq57zhkm3+wrn3ch6yUg6JZT+OwKhTf3i0FX+IWcPB1iFDQ2Vy3zh
-----END PRIVATE KEY-----
)");
  constexpr char const* testEcdsPublicKey(R"(-----BEGIN PUBLIC KEY-----
MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE1pg6eVq3GmWUEGT4SNmrDw2CRW52
OTE31que84ZJt/sK593IeslIOiWU/jsCoU394tBV/iFnDwdYhQ0Nlct84Q==
-----END PUBLIC KEY-----
)");

  constexpr char const* testRsaPrivateKey(R"(-----BEGIN PRIVATE KEY-----
MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDOqtQaML1iiA2j
YmrePlsDjPOH429ZAXMQNPCWWqEkNY1T5HckA9dW81P43w8/k81tPuC2eLVBD/2o
zaRIZpWikbwsvN4uoQhrJDpuQyJ2V681fRWj8Dinqcd2Ccar8igwvE9UJMGDkX+q
Wti+BwA4dJ56LzxbQHYc39mVcmosfpu4ihifMj2U4bNA+e0s8ft/VEm2UG4hWgzK
NMrGvqkusAGvWs0/UPthb7HJEVZaHX5ebX2LgedUgSl88oe4P5tNR8XSbTvvp3oP
x4EkM63S4wp4A2JsfpNgYPT1oNRB+/VHjW+OZjKaYjoTqs++M7KnHaUn+qUwCuMY
meVI2cEZAgMBAAECggEALf1l5c4i1uJf5pPoJDzMFCYxq3O5O51O9bRSNaNFaMFi
CeE1ghY4vWi4rxE0W3mQpVbwuqpx62CbmgzpGhN6CQtVTL9a0hWTwgP13MOjz6ID
o2uKfUjf0q+9a08RnwHsX6wIGzlytsySFF7TDLaSHf4VpisMy2G05wgJa3BioDPG
IjFjJNR9jB+Ql9IYpnJh30m3sLAsmj2I2UvHbBtTmjE9/He0W4M4HvJZ9+Mm0BLZ
53apdBAne2axgxoESWy6kPZB/Fcvwn3zIKJQ2rwdN3mhZb/tVw7t5DV9ASmTOr6L
62qedFGSrAQoAjrB9oxFd+DLACVmJebdIWsQzcDeYQKBgQDo1Ii37UKOCtBp7SPr
Sa0d9ngqX7+w/AK95yylKOU/mDQA9v/vDVD4XmQI9ZOYzA/xM09SWm7HTmbJDK7U
EqLrtdx6j2itjf4M3X+zYCxbxsT93D7Sq/zQUnnNcUGuo98CjayMEVWDRDF+CZTG
X0W3OZUAU9D1SvJ8hNhulK2WzwKBgQDjO8f9Ta1crRWyhMxPQQWG3H3pyvuC92I1
EZKYoQfmMN64cKMIjE12062Oht48udZUHTSe/zHRCn0C4c8EVSEKEUvJUgXjc4lP
c1ftRfpC9WNnwczu1uaAk6vfXF/plEB/9knBYhpAcORpWxB8f64oSrz5EanJiQbv
NWJ+1ZCjlwKBgQDiJraZuKph390qVn4CJ7EwnluABTrjxRVAshAqaHusdsFkgoZ8
Azo31S9jiG2SB/wgM8+DVXWuv9eUx231bhizzRTYMv3hPj+a7XcBm5PanUpwroKT
DR1mmAXZaH39DQ0rpMMJ1jhyZUWRf+rzeEz2OMci50bbS64XBs5XMrEd/wKBgQC4
tu3ZMP2N6n1Kwry6aCav/Ci2lfRh/+rrLL+4Jp6fNna2A4nj9vk5cNUSmPuq7X4W
ni8aWGQMg7QfVaPM586Vun2ax3xV6qNh3GdLT6kiKQuHWnjWZga12lTKmvK0k3jj
DDfkZXTlkV97bTU3nyrZQfffl8YnN6ZVaVYJuF19PQKBgAXShPkDmWPV3EwinvlK
Aak2GBskgGpbpsYb0pUDn/PRpk/5Y4TC+Pi8mDXllUeAK8T52+BtgOwqRv4rMc6o
cpjbImp0NonsrJjYgBlqvSmGCVFD8Fs/X3bGU/eMTR223jQt68ZZgf0uEFwJiApW
TBZ3pOmBQcldj1vo3JmpQvcT
-----END PRIVATE KEY-----
)");

  constexpr char const* testRsaPublicKey(R"(-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAzqrUGjC9YogNo2Jq3j5b
A4zzh+NvWQFzEDTwllqhJDWNU+R3JAPXVvNT+N8PP5PNbT7gtni1QQ/9qM2kSGaV
opG8LLzeLqEIayQ6bkMidlevNX0Vo/A4p6nHdgnGq/IoMLxPVCTBg5F/qlrYvgcA
OHSeei88W0B2HN/ZlXJqLH6buIoYnzI9lOGzQPntLPH7f1RJtlBuIVoMyjTKxr6p
LrABr1rNP1D7YW+xyRFWWh1+Xm19i4HnVIEpfPKHuD+bTUfF0m0776d6D8eBJDOt
0uMKeANibH6TYGD09aDUQfv1R41vjmYymmI6E6rPvjOypx2lJ/qlMArjGJnlSNnB
GQIDAQAB
-----END PUBLIC KEY-----
)");

  void ImportKeyTest(std::string const& pemPrivateKey, std::string const& pemPublicKey)
  {
    // Verify we can round trip the private key.
    {
      auto privateKey = Crypto::ImportPrivateKey(pemPrivateKey);
      auto exportedPrivateKey = privateKey->ExportPrivateKey();

      EXPECT_EQ(exportedPrivateKey, pemPrivateKey);
    }
    // Verify we can round trip the public key.
    {
      auto publicKey = Crypto::ImportPublicKey(pemPublicKey);
      auto exportedPublicKey = publicKey->ExportPublicKey();

      EXPECT_EQ(exportedPublicKey, pemPublicKey);
    }
    // Exported public key of private key matches.
    {
      auto privateKey = Crypto::ImportPrivateKey(pemPrivateKey);
      auto exportedPublicKey = privateKey->ExportPublicKey();

      EXPECT_EQ(exportedPublicKey, pemPublicKey);
    }
  }
  TEST(CryptoTests, ImportKeyTest)
  {
    ImportKeyTest(testRsaPrivateKey, testRsaPublicKey);
    ImportKeyTest(testEcdsPrivateKey, testEcdsPublicKey);
  }
  TEST(CryptoTests, CreateRsaKey)
  {
    auto privateKey = Crypto::CreateRsaKey(2048);
    std::string exportedPrivateKey = privateKey->ExportPrivateKey();

    EXPECT_EQ(0ul, exportedPrivateKey.find("-----BEGIN PRIVATE KEY-----"));

    auto importedKey = Crypto::ImportPrivateKey(exportedPrivateKey);

    std::string exportedPublicKey = privateKey->ExportPublicKey();

    EXPECT_EQ(0ul, exportedPublicKey.find("-----BEGIN PUBLIC KEY-----"));
    auto importedPublicKey = Crypto::ImportPublicKey(exportedPublicKey);

    EXPECT_THROW(Crypto::ImportPrivateKey(exportedPublicKey), std::runtime_error);

    Azure::Core::Diagnostics::_internal::Log::Write(
        Logger::Level::Informational, exportedPrivateKey);

    Azure::Core::Diagnostics::_internal::Log::Write(
        Logger::Level::Informational, exportedPublicKey);
  }

  TEST(CryptoTests, SignRsaBuffer)
  {
    auto privateKey = Crypto::CreateRsaKey(2048);

    std::vector<uint8_t> signaturePayload{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    std::vector<uint8_t> signature = privateKey->SignBuffer(signaturePayload);

    EXPECT_TRUE(privateKey->VerifySignature(signaturePayload, signature));

    auto exportedPublicKey = privateKey->ExportPublicKey();
    auto publicKey = Crypto::ImportPublicKey(exportedPublicKey);
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
    auto privateKey = Crypto::CreateEcdsaKey();
    std::string exportedPrivateKey = privateKey->ExportPrivateKey();

    EXPECT_EQ(0ul, exportedPrivateKey.find("-----BEGIN PRIVATE KEY-----"));
    auto importedKey = Crypto::ImportPrivateKey(exportedPrivateKey);

    std::string exportedPublicKey = privateKey->ExportPublicKey();

    EXPECT_EQ(0ul, exportedPublicKey.find("-----BEGIN PUBLIC KEY-----"));
    auto importedPublicKey = Crypto::ImportPublicKey(exportedPublicKey);

    EXPECT_THROW(Crypto::ImportPrivateKey(exportedPublicKey), std::runtime_error);

    Azure::Core::Diagnostics::_internal::Log::Write(
        Logger::Level::Informational, exportedPrivateKey);

    Azure::Core::Diagnostics::_internal::Log::Write(
        Logger::Level::Informational, exportedPublicKey);
  }

  TEST(CryptoTests, SignEcdaBuffer)
  {
    auto privateKey = Crypto::CreateEcdsaKey();

    auto exportedPublicKey = privateKey->ExportPublicKey();
    auto publicKey = Crypto::ImportPublicKey(exportedPublicKey);

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

    EXPECT_THROW(Crypto::ImportPrivateKey(pemEncodedGarbage), std::runtime_error);
    EXPECT_THROW(Crypto::ImportPublicKey(pemEncodedGarbage), std::runtime_error);
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
    auto x509cert(Crypto::ImportX509Certificate(pemEncodedCertificate1));
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
    auto privateKey = Crypto::CreateRsaKey(2048);
    auto x509cert = Crypto::CreateX509CertificateForPrivateKey(
        privateKey, "CN=Test\\Subject1, O=Microsoft Corporation, L=Redmond, ST=WA, C=US");

    EXPECT_EQ(
        "CN=TestSubject1, O=Microsoft Corporation, L=Redmond, ST=WA, C=US",
        x509cert->GetSubjectName());
    EXPECT_EQ(
        "CN=TestSubject1, O=Microsoft Corporation, L=Redmond, ST=WA, C=US",
        x509cert->GetIssuerName());

    std::string certThumbprint(x509cert->GetThumbprint());
    EXPECT_FALSE(certThumbprint.empty());
  }
  TEST(CryptoTests, CreateEcdsX509Certificate)
  {
    auto privateKey = Crypto::CreateEcdsaKey();
    auto x509cert = Crypto::CreateX509CertificateForPrivateKey(
        privateKey, "CN=ECDSATest\\Subject1, O=Microsoft Corporation, L=Redmond, ST=WA, C=US");

    EXPECT_EQ(
        "CN=ECDSATestSubject1, O=Microsoft Corporation, L=Redmond, ST=WA, C=US",
        x509cert->GetSubjectName());
    EXPECT_EQ(
        "CN=ECDSATestSubject1, O=Microsoft Corporation, L=Redmond, ST=WA, C=US",
        x509cert->GetIssuerName());

    std::string certThumbprint(x509cert->GetThumbprint());
    EXPECT_FALSE(certThumbprint.empty());
  }

}}}} // namespace Azure::Security::Attestation::Test

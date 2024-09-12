// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/identity/client_certificate_credential.hpp"
#include "credential_test_helper.hpp"

#include <azure/core/base64.hpp>

#include <cstdio>
#include <fstream>

#include <gtest/gtest.h>

using Azure::Core::Http::HttpMethod;
using Azure::Identity::ClientCertificateCredential;
using Azure::Identity::ClientCertificateCredentialOptions;
using Azure::Identity::Test::_detail::CredentialTestHelper;

namespace {
enum CertFormat
{
  RsaPkcs,
  RsaRaw,
  RsaRawCertChain
};

enum TestType
{
  Regular,
  AzureStack,
  Authority
};

struct TempCertFile final
{
  static const char* const Path;
  ~TempCertFile();
  TempCertFile(CertFormat algorithm = RsaPkcs);
};

std::vector<std::string> SplitString(const std::string& s, char separator);

std::string ToString(std::vector<uint8_t> const& vec);
} // namespace

class GetCredentialName : public ::testing::TestWithParam<CertFormat> {
  TempCertFile m_certFile{GetParam()};
};

class GetToken : public ::testing::TestWithParam<std::tuple<TestType, CertFormat, bool>> {
public:
  TestType GetTestType() { return std::get<0>(GetParam()); }

  CertFormat GetCertFormat() { return std::get<1>(GetParam()); }

  bool GetSendCertChain() { return std::get<2>(GetParam()); }

  std::string GetTenantId()
  {
    return GetTestType() == TestType::AzureStack ? "adfs" : "01234567-89ab-cdef-fedc-ba8976543210";
  }

  std::string GetRequestUri()
  {
    switch (GetTestType())
    {
      case TestType::Regular:
        return "https://login.microsoftonline.com/01234567-89ab-cdef-fedc-ba8976543210/oauth2/v2.0/"
               "token";
      case TestType::AzureStack:
        return "https://login.microsoftonline.com/adfs/oauth2/token";
      case TestType::Authority:
        return "https://microsoft.com/01234567-89ab-cdef-fedc-ba8976543210/oauth2/v2.0/token";
    }
    AZURE_UNREACHABLE_CODE();
  }

  std::string GetBodyStart0()
  {
    switch (GetTestType())
    {
      case TestType::Regular:
      case TestType::Authority: // cspell:disable
        return "grant_type=client_credentials"
               "&client_assertion_type=urn%3Aietf%3Aparams%3Aoauth%3Aclient-assertion-type%3Ajwt-"
               "bearer"
               "&client_id=fedcba98-7654-3210-0123-456789abcdef"
               "&scope=https%3A%2F%2Fazure.com%2F.default"
               "&client_assertion="; // cspell:enable
      case TestType::AzureStack: // cspell:disable
        return "grant_type=client_credentials"
               "&client_assertion_type=urn%3Aietf%3Aparams%3Aoauth%3Aclient-assertion-type%3Ajwt-"
               "bearer"
               "&client_id=fedcba98-7654-3210-0123-456789abcdef"
               "&scope=https%3A%2F%2Fazure.com"
               "&client_assertion="; // cspell:enable
    }
    AZURE_UNREACHABLE_CODE();
  }

  std::string GetBodyStart1()
  {
    // cspell:disable
    return "grant_type=client_credentials"
           "&client_assertion_type=urn%3Aietf%3Aparams%3Aoauth%3Aclient-assertion-type%3Ajwt-bearer"
           "&client_id=fedcba98-7654-3210-0123-456789abcdef"
           "&client_assertion="; // cspell:enable
  }

  std::string GetHeader()
  {
    std::string x5t = "\"V0pIIQwSzNn6vfSTPv-1f7Vt_Pw\"";
    std::string kid = "\"574A48210C12CCD9FABDF4933EFFB57FB56DFCFC\"";
    if (GetCertFormat() == RsaRawCertChain)
    {
      x5t = "\"p9oEWkxGpqXnMIswfxzUPBnwMdY\"";
      kid = "\"A7DA045A4C46A6A5E7308B307F1CD43C19F031D6\"";
    }
    if (GetSendCertChain())
    {
      // cspell:disable
      return "{\"x5t\":" + x5t + ",\"kid\":" + kid
          + ",\"alg\":\"RS256\",\"typ\":\"JWT\","
            "\"x5c\":[]}";
    }
    // cspell:disable
    return "{\"x5t\":" + x5t + ",\"kid\":" + kid + ",\"alg\":\"RS256\",\"typ\":\"JWT\"}";
  }

  std::string GetPayloadStart()
  {
    switch (GetTestType())
    {
      case TestType::Regular:
        return "{\"aud\":\"https://login.microsoftonline.com/01234567-89ab-cdef-fedc-ba8976543210/"
               "oauth2/v2.0/token\","
               "\"iss\":\"fedcba98-7654-3210-0123-456789abcdef\","
               "\"sub\":\"fedcba98-7654-3210-0123-456789abcdef\",\"jti\":\"";
      case TestType::AzureStack:
        return "{\"aud\":\"https://login.microsoftonline.com/adfs/oauth2/token\","
               "\"iss\":\"fedcba98-7654-3210-0123-456789abcdef\","
               "\"sub\":\"fedcba98-7654-3210-0123-456789abcdef\",\"jti\":\"";
      case TestType::Authority:
        return "{\"aud\":\"https://microsoft.com/01234567-89ab-cdef-fedc-ba8976543210/"
               "oauth2/v2.0/token\","
               "\"iss\":\"fedcba98-7654-3210-0123-456789abcdef\","
               "\"sub\":\"fedcba98-7654-3210-0123-456789abcdef\",\"jti\":\"";
    }
    AZURE_UNREACHABLE_CODE();
  }

  size_t GetSignatureSize() { return 256; }

private:
  TempCertFile m_certFile{GetCertFormat()};
};

TEST_P(GetCredentialName, )
{
  ClientCertificateCredential const cred(
      "01234567-89ab-cdef-fedc-ba8976543210",
      "fedcba98-7654-3210-0123-456789abcdef",
      TempCertFile::Path);

  EXPECT_EQ(cred.GetCredentialName(), "ClientCertificateCredential");
}

TEST(ClientCertificateCredential, UnsupportedExtension)
{
  try
  {
    ClientCertificateCredential const cred(
        "01234567-89ab-cdef-fedc-ba8976543210", "fedcba98-7654-3210-0123-456789abcdef", "file.pfx");

    EXPECT_TRUE(
        !"ClientCertificateCredential with unsupported extension (.pfx) is supposed to throw.");
  }
  catch (Azure::Core::Credentials::AuthenticationException const& ex)
  {
    EXPECT_EQ(
        ex.what(),
        std::string("Identity: ClientCertificateCredential: "
                    "Certificate format ('.pfx') is not supported. "
                    "Please convert your certificate to '.pem'."));
  }

  try
  {
    ClientCertificateCredential const cred(
        "01234567-89ab-cdef-fedc-ba8976543210",
        "fedcba98-7654-3210-0123-456789abcdef",
        "file.cert");

    EXPECT_TRUE(
        !"ClientCertificateCredential with unsupported extension (.cert) is supposed to throw.");
  }
  catch (Azure::Core::Credentials::AuthenticationException const& ex)
  {
    EXPECT_EQ(
        ex.what(),
        std::string("Identity: ClientCertificateCredential: "
                    "Certificate format ('.cert') is not supported. "
                    "Please convert your certificate to '.pem'."));
  }

  try
  {
    ClientCertificateCredential const cred(
        "01234567-89ab-cdef-fedc-ba8976543210",
        "fedcba98-7654-3210-0123-456789abcdef",
        "noextension");

    EXPECT_TRUE(!"ClientCertificateCredential without an extension is supposed to throw.");
  }
  catch (Azure::Core::Credentials::AuthenticationException const& ex)
  {
    EXPECT_EQ(
        ex.what(),
        std::string("Identity: ClientCertificateCredential: "
                    "Certificate format is not supported. "
                    "Please convert your certificate to '.pem'."));
  }

  try
  {
    ClientCertificateCredential const cred(
        "01234567-89ab-cdef-fedc-ba8976543210", "fedcba98-7654-3210-0123-456789abcdef", "");

    EXPECT_TRUE(!"ClientCertificateCredential with an empty path is supposed to throw.");
  }
  catch (Azure::Core::Credentials::AuthenticationException const& ex)
  {
    EXPECT_EQ(
        ex.what(),
        std::string("Identity: ClientCertificateCredential: Certificate file path is empty."));
  }
}

TEST(ClientCertificateCredential, GetOptionsFromEnvironment)
{
  {
    std::map<std::string, std::string> envVars = {{"AZURE_AUTHORITY_HOST", ""}};
    CredentialTestHelper::EnvironmentOverride const env(envVars);

    ClientCertificateCredentialOptions options;
    EXPECT_EQ(options.AuthorityHost, "https://login.microsoftonline.com/");
  }

  {
    std::map<std::string, std::string> envVars
        = {{"AZURE_AUTHORITY_HOST", "https://microsoft.com/"}};
    CredentialTestHelper::EnvironmentOverride const env(envVars);

    ClientCertificateCredentialOptions options;
    EXPECT_EQ(options.AuthorityHost, "https://microsoft.com/");
  }
}

TEST_P(GetToken, )
{
  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [this](auto transport) {
        ClientCertificateCredentialOptions options;
        if (GetTestType() == TestType::Authority)
        {
          options.AuthorityHost = "https://microsoft.com/";
        }
        options.Transport.Transport = transport;
        options.SendCertificateChain = GetSendCertChain();

        return std::make_unique<ClientCertificateCredential>(
            GetTenantId(), "fedcba98-7654-3210-0123-456789abcdef", TempCertFile::Path, options);
      },
      {{{"https://azure.com/.default"}}, {{}}},
      std::vector<std::string>{
          "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}",
          "{\"expires_in\":7200, \"access_token\":\"ACCESSTOKEN2\"}"});

  EXPECT_EQ(actual.Requests.size(), 2U);
  EXPECT_EQ(actual.Responses.size(), 2U);

  auto const& request0 = actual.Requests.at(0);
  auto const& request1 = actual.Requests.at(1);

  auto const& response0 = actual.Responses.at(0);
  auto const& response1 = actual.Responses.at(1);

  EXPECT_EQ(request0.HttpMethod, HttpMethod::Post);
  EXPECT_EQ(request1.HttpMethod, HttpMethod::Post);

  EXPECT_EQ(request0.AbsoluteUrl, GetRequestUri());

  EXPECT_EQ(request1.AbsoluteUrl, GetRequestUri());

  {
    auto expectedBodyStart0 = GetBodyStart0();
    auto expectedBodyStart1 = GetBodyStart1();

    EXPECT_GT(request0.Body.size(), expectedBodyStart0.size());
    EXPECT_GT(request1.Body.size(), expectedBodyStart1.size());

    EXPECT_EQ(request0.Body.substr(0, expectedBodyStart0.size()), expectedBodyStart0);
    EXPECT_EQ(request1.Body.substr(0, expectedBodyStart1.size()), expectedBodyStart1);

    EXPECT_NE(request0.Headers.find("Content-Length"), request0.Headers.end());
    EXPECT_GT(
        std::stoi(request0.Headers.at("Content-Length")),
        static_cast<int>(expectedBodyStart0.size()));

    EXPECT_NE(request1.Headers.find("Content-Length"), request1.Headers.end());
    EXPECT_GT(
        std::stoi(request1.Headers.at("Content-Length")),
        static_cast<int>(expectedBodyStart1.size()));

    {
      using Azure::Core::_internal::Base64Url;

      const auto assertion0 = request0.Body.substr(expectedBodyStart0.size());
      const auto assertion1 = request1.Body.substr(expectedBodyStart1.size());

      const auto assertion0Parts = SplitString(assertion0, '.');
      const auto assertion1Parts = SplitString(assertion1, '.');

      EXPECT_EQ(assertion0Parts.size(), 3U);
      EXPECT_EQ(assertion1Parts.size(), 3U);

      const auto header0Vec = Base64Url::Base64UrlDecode(assertion0Parts[0]);
      const auto header1Vec = Base64Url::Base64UrlDecode(assertion1Parts[0]);

      const auto payload0Vec = Base64Url::Base64UrlDecode(assertion0Parts[1]);
      const auto payload1Vec = Base64Url::Base64UrlDecode(assertion1Parts[1]);

      const auto signature0 = assertion0Parts[2];
      const auto signature1 = assertion1Parts[2];

      const auto header0 = ToString(header0Vec);
      const auto header1 = ToString(header1Vec);

      const auto payload0 = ToString(payload0Vec);
      const auto payload1 = ToString(payload1Vec);

      auto ExpectedHeader = GetHeader();

      EXPECT_EQ(header0, ExpectedHeader);
      EXPECT_EQ(header1, ExpectedHeader);

      auto ExpectedPayloadStart = GetPayloadStart();

      EXPECT_EQ(payload0.substr(0, ExpectedPayloadStart.size()), ExpectedPayloadStart);
      EXPECT_EQ(payload1.substr(0, ExpectedPayloadStart.size()), ExpectedPayloadStart);

      EXPECT_EQ(Base64Url::Base64UrlDecode(signature0).size(), GetSignatureSize());
      EXPECT_EQ(Base64Url::Base64UrlDecode(signature1).size(), GetSignatureSize());
    }
  }

  EXPECT_NE(request0.Headers.find("Content-Type"), request0.Headers.end());
  EXPECT_EQ(request0.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_NE(request1.Headers.find("Content-Type"), request1.Headers.end());
  EXPECT_EQ(request1.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_EQ(response0.AccessToken.Token, "ACCESSTOKEN1");
  EXPECT_EQ(response1.AccessToken.Token, "ACCESSTOKEN2");

  using namespace std::chrono_literals;
  EXPECT_GE(response0.AccessToken.ExpiresOn, response0.EarliestExpiration + 3600s);
  EXPECT_LE(response0.AccessToken.ExpiresOn, response0.LatestExpiration + 3600s);

  EXPECT_GE(response1.AccessToken.ExpiresOn, response1.EarliestExpiration + 7200s);
  EXPECT_LE(response1.AccessToken.ExpiresOn, response1.LatestExpiration + 7200s);
}

INSTANTIATE_TEST_SUITE_P(
    ClientCertificateCredential,
    GetCredentialName,
    testing::Values(RsaPkcs, RsaRaw));

INSTANTIATE_TEST_SUITE_P(
    ClientCertificateCredential,
    GetToken,
    testing::Combine(
        testing::Values(Regular, AzureStack, Authority),
        testing::Values(RsaPkcs, RsaRaw, RsaRawCertChain),
        testing::Values(true, false)));

namespace {
const char* const TempCertFile::Path = "azure-identity-test.pem";

TempCertFile::~TempCertFile() { std::remove(Path); }

TempCertFile::TempCertFile(CertFormat format)
{
  std::ofstream cert(Path, std::ios_base::out | std::ios_base::trunc);

  if (format == RsaPkcs)
    cert << // cspell:disable
        "Bag Attributes\n"
        "    Microsoft Local Key set: <No Values>\n"
        "    localKeyID: 01 00 00 00 \n"
        "    friendlyName: te-66f5c973-4fc8-4cd3-8acc-64964d79b693\n"
        "    Microsoft CSP Name: Microsoft Software Key Storage Provider\n"
        "Key Attributes\n"
        "    X509v3 Key Usage: 90 \n"
        "-----BEGIN PRIVATE KEY-----\n"
        "MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDPdm4pukO7ugEx\n"
        "8wXrmo4VIEoicp7w3QsEJGA2bMx9nHMvwugG54t14QpfqBQYQWLeL1HmpcDeivVD\n"
        "+15ZXeGLCPVZBHhoY8ZWGibfhAAzqQ0P9Ca1kydjvB4uJcEnF/RYtQv6n6OwmdO1\n"
        "wJ22JNcRlMtZqmnb/Q0In2fjXEbdl85/GZlYzMQRdyfI0yriSRBcYV2kg0zeXCxf\n"
        "mCvB3rb6I1KpoUFHlkeHtkeDwm0VHUEt4Hz8ghcB00tI5eS2fH2rPkINQKc6+0QU\n"
        "C2KICQC+GzJsYDbwQOao5Vhk80H5LRuM9Ndzv+fU3lLnktYCgXgL9AX4L/R9Z4Pz\n"
        "tuao/qbRAgMBAAECggEBAMQZIrooiTuZ7uVC3Ja96Y1IjyqOg3QSzAXnSFZJcuVM\n"
        "i4hayC02khkjVUXjvtLKg2SW/+hvRqZUXM8cfCsm1Tkxh4/T7OhnXyMl5xahU/uA\n"
        "0IsC8c/xv2rDdxeRskh8mQd8Yk1MtlIIpRgIcEqp+exxY+FmdldtkvNSkcVUBNwQ\n"
        "nXi+oWPhE2guo2g1BPk2gbF0+3FvSrQ8QwGHg+uQJwrQpJ+SB9TyuQFauGR5/wSq\n"
        "H93cFH5YC/+v5I7qW6ZQe0f7rEKQDybGVzkBlKJyGCVYmPn7Xa/wJriws+FZIfHz\n"
        "f3m0kJigxJd/HwTrnKSg+H8oBgng7lZLdBYWHMGJhA0CgYEA48moW7szegvfLuUF\n"
        "a0sHfyKuNyvOv7Wud4sa0lwdKPHS+atwL6TNUWCAGkomYADEe3qiYgMXDX9U3hlW\n"
        "6zktYFj03tnRg4iBjp8nchLBVLf3Wd5TPRw1VKu4ZW43y8BRhYWV+3Z4s1nyMEDA\n"
        "NFbKRmL7LDB05oWHdJMjFK/L6YcCgYEA6ShV4v2RQiXzkW6GHSBZDIVHCeWwvIld\n"
        "OlEfG7wzZW4e8wNDhfSMtXyJrzfbEyXBtVKoESdP6Nnm9W7ftcynW965S94THuy7\n"
        "+ofvHo6JAm8g/0uX70wZ26LU8qhkJMTWmsONBNKLwUzkFT7VGsdaBliam1RLvjeT\n"
        "URdQgnftIucCgYEA4FYamT0k1W4bv/OOAr1CBNQDABME64ni6Zj2MXbGwSxou7s8\n"
        "IbANBbgkcb/VS3d2CqYchqrEaWaeDp6mG8OUDO+POmsLDJ/D+NKF5rLR9L25vahY\n"
        "EjdVzq3QTRTfnqspnnaR37Yt6XUMMLmUkfdn/yo8dKjEeMPJQ+YlBpqcGMECgYBZ\n"
        "rmIaxV2yC9b8AX8khOS7pCgG7opkepGZdMp6aJF8WjcdUgwO4lmdFSIAe4OQgd1Y\n"
        "WUq8Dlr2PZpQnSz/SJC3DZxISksggf5sBw06u6iHfyc6C2GNccAgcylljM+4NN42\n"
        "+TCswi9vUpwIb/qYKkW+WyZcyLe5mrbXYhhdlrNn0QKBgDe8aRG+MOSUTTXjAVss\n"
        "bDY0Us943FN91qBmagNqDyozKAAqDoKvdRxM0IlIDnOptj4AfbpJ1JThNOJDYBpU\n"
        "+Azo8UoedANgndtZ2n11RSjmlQ6TE/WGlsirHExqr6y/l71znoQm1y3E2cArbsmy\n"
        "hp0P5v42PKxmAx4pR0EjNKsd\n"
        "-----END PRIVATE KEY-----\n"
        "Bag Attributes\n"
        "    localKeyID: 01 00 00 00 \n"
        "    1.3.6.1.4.1.311.17.3.71: 61 00 6E 00 74 00 6B 00 2D 00 6C 00 61 00 70 00 "
        "74 00 6F 00 70 00 00 00 \n"
        "subject=CN = azure-identity-test\n"
        "\n"
        "issuer=CN = azure-identity-test\n"
        "\n"
        "-----BEGIN CERTIFICATE-----\n"
        "MIIDODCCAiCgAwIBAgIQNqa9U3MBxqBF7ksWk+XRkzANBgkqhkiG9w0BAQsFADAe\n"
        "MRwwGgYDVQQDDBNhenVyZS1pZGVudGl0eS10ZXN0MCAXDTIyMDQyMjE1MDYwNloY\n"
        "DzIyMjIwMTAxMDcwMDAwWjAeMRwwGgYDVQQDDBNhenVyZS1pZGVudGl0eS10ZXN0\n"
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAz3ZuKbpDu7oBMfMF65qO\n"
        "FSBKInKe8N0LBCRgNmzMfZxzL8LoBueLdeEKX6gUGEFi3i9R5qXA3or1Q/teWV3h\n"
        "iwj1WQR4aGPGVhom34QAM6kND/QmtZMnY7weLiXBJxf0WLUL+p+jsJnTtcCdtiTX\n"
        "EZTLWapp2/0NCJ9n41xG3ZfOfxmZWMzEEXcnyNMq4kkQXGFdpINM3lwsX5grwd62\n"
        "+iNSqaFBR5ZHh7ZHg8JtFR1BLeB8/IIXAdNLSOXktnx9qz5CDUCnOvtEFAtiiAkA\n"
        "vhsybGA28EDmqOVYZPNB+S0bjPTXc7/n1N5S55LWAoF4C/QF+C/0fWeD87bmqP6m\n"
        "0QIDAQABo3AwbjAOBgNVHQ8BAf8EBAMCBaAwHQYDVR0lBBYwFAYIKwYBBQUHAwIG\n"
        "CCsGAQUFBwMBMB4GA1UdEQQXMBWCE2F6dXJlLWlkZW50aXR5LXRlc3QwHQYDVR0O\n"
        "BBYEFCoJ5tInmafyNuR0tGxZOz522jlWMA0GCSqGSIb3DQEBCwUAA4IBAQBzLXpw\n"
        "Xmrg1sQTmzMnS24mREKxj9B3YILmgsdBMrHkH07QUROee7IbQ8gfBKeln0dEcfYi\n"
        "Jyh42jn+fmg9AR17RP80wPthD2eKOt4WYNkNM3H8U4JEo+0ML0jZyswynpR48h/E\n"
        "m96sm/NUeKUViD5iVTb1uHL4j8mQAN1IbXcunXvrrek1CzFVn5Rpah0Tn+6cYVKd\n"
        "Jg531i53udzusgZtV1NPZ82tzYkPQG1vxB//D9vd0LzmcfCvT50MKhz0r/c5yJYk\n"
        "i9q94DBuzMhe+O9j+Ob2pVQt5akVFJVtIVSfBZzRBAd66u9JeADlT4sxwS4QAUHi\n"
        "RrCsEpJsnJXkx/6O\n"
        "-----END CERTIFICATE-----\n";
  // cspell:enable
  else if (format == RsaRaw)
    cert << // cspell:disable
        "-----BEGIN RSA PRIVATE KEY-----\n"
        "MIIEpAIBAAKCAQEAz3ZuKbpDu7oBMfMF65qOFSBKInKe8N0LBCRgNmzMfZxzL8Lo\n"
        "BueLdeEKX6gUGEFi3i9R5qXA3or1Q/teWV3hiwj1WQR4aGPGVhom34QAM6kND/Qm\n"
        "tZMnY7weLiXBJxf0WLUL+p+jsJnTtcCdtiTXEZTLWapp2/0NCJ9n41xG3ZfOfxmZ\n"
        "WMzEEXcnyNMq4kkQXGFdpINM3lwsX5grwd62+iNSqaFBR5ZHh7ZHg8JtFR1BLeB8\n"
        "/IIXAdNLSOXktnx9qz5CDUCnOvtEFAtiiAkAvhsybGA28EDmqOVYZPNB+S0bjPTX\n"
        "c7/n1N5S55LWAoF4C/QF+C/0fWeD87bmqP6m0QIDAQABAoIBAQDEGSK6KIk7me7l\n"
        "QtyWvemNSI8qjoN0EswF50hWSXLlTIuIWsgtNpIZI1VF477SyoNklv/ob0amVFzP\n"
        "HHwrJtU5MYeP0+zoZ18jJecWoVP7gNCLAvHP8b9qw3cXkbJIfJkHfGJNTLZSCKUY\n"
        "CHBKqfnscWPhZnZXbZLzUpHFVATcEJ14vqFj4RNoLqNoNQT5NoGxdPtxb0q0PEMB\n"
        "h4PrkCcK0KSfkgfU8rkBWrhkef8Eqh/d3BR+WAv/r+SO6lumUHtH+6xCkA8mxlc5\n"
        "AZSichglWJj5+12v8Ca4sLPhWSHx8395tJCYoMSXfx8E65ykoPh/KAYJ4O5WS3QW\n"
        "FhzBiYQNAoGBAOPJqFu7M3oL3y7lBWtLB38irjcrzr+1rneLGtJcHSjx0vmrcC+k\n"
        "zVFggBpKJmAAxHt6omIDFw1/VN4ZVus5LWBY9N7Z0YOIgY6fJ3ISwVS391neUz0c\n"
        "NVSruGVuN8vAUYWFlft2eLNZ8jBAwDRWykZi+ywwdOaFh3STIxSvy+mHAoGBAOko\n"
        "VeL9kUIl85Fuhh0gWQyFRwnlsLyJXTpRHxu8M2VuHvMDQ4X0jLV8ia832xMlwbVS\n"
        "qBEnT+jZ5vVu37XMp1veuUveEx7su/qH7x6OiQJvIP9Ll+9MGdui1PKoZCTE1prD\n"
        "jQTSi8FM5BU+1RrHWgZYmptUS743k1EXUIJ37SLnAoGBAOBWGpk9JNVuG7/zjgK9\n"
        "QgTUAwATBOuJ4umY9jF2xsEsaLu7PCGwDQW4JHG/1Ut3dgqmHIaqxGlmng6ephvD\n"
        "lAzvjzprCwyfw/jSheay0fS9ub2oWBI3Vc6t0E0U356rKZ52kd+2Lel1DDC5lJH3\n"
        "Z/8qPHSoxHjDyUPmJQaanBjBAoGAWa5iGsVdsgvW/AF/JITku6QoBu6KZHqRmXTK\n"
        "emiRfFo3HVIMDuJZnRUiAHuDkIHdWFlKvA5a9j2aUJ0s/0iQtw2cSEpLIIH+bAcN\n"
        "Oruoh38nOgthjXHAIHMpZYzPuDTeNvkwrMIvb1KcCG/6mCpFvlsmXMi3uZq212IY\n"
        "XZazZ9ECgYA3vGkRvjDklE014wFbLGw2NFLPeNxTfdagZmoDag8qMygAKg6Cr3Uc\n"
        "TNCJSA5zqbY+AH26SdSU4TTiQ2AaVPgM6PFKHnQDYJ3bWdp9dUUo5pUOkxP1hpbI\n"
        "qxxMaq+sv5e9c56EJtctxNnAK27JsoadD+b+NjysZgMeKUdBIzSrHQ==\n"
        "-----END RSA PRIVATE KEY-----\n"
        "-----BEGIN CERTIFICATE-----\n"
        "MIIDODCCAiCgAwIBAgIQNqa9U3MBxqBF7ksWk+XRkzANBgkqhkiG9w0BAQsFADAe\n"
        "MRwwGgYDVQQDDBNhenVyZS1pZGVudGl0eS10ZXN0MCAXDTIyMDQyMjE1MDYwNloY\n"
        "DzIyMjIwMTAxMDcwMDAwWjAeMRwwGgYDVQQDDBNhenVyZS1pZGVudGl0eS10ZXN0\n"
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAz3ZuKbpDu7oBMfMF65qO\n"
        "FSBKInKe8N0LBCRgNmzMfZxzL8LoBueLdeEKX6gUGEFi3i9R5qXA3or1Q/teWV3h\n"
        "iwj1WQR4aGPGVhom34QAM6kND/QmtZMnY7weLiXBJxf0WLUL+p+jsJnTtcCdtiTX\n"
        "EZTLWapp2/0NCJ9n41xG3ZfOfxmZWMzEEXcnyNMq4kkQXGFdpINM3lwsX5grwd62\n"
        "+iNSqaFBR5ZHh7ZHg8JtFR1BLeB8/IIXAdNLSOXktnx9qz5CDUCnOvtEFAtiiAkA\n"
        "vhsybGA28EDmqOVYZPNB+S0bjPTXc7/n1N5S55LWAoF4C/QF+C/0fWeD87bmqP6m\n"
        "0QIDAQABo3AwbjAOBgNVHQ8BAf8EBAMCBaAwHQYDVR0lBBYwFAYIKwYBBQUHAwIG\n"
        "CCsGAQUFBwMBMB4GA1UdEQQXMBWCE2F6dXJlLWlkZW50aXR5LXRlc3QwHQYDVR0O\n"
        "BBYEFCoJ5tInmafyNuR0tGxZOz522jlWMA0GCSqGSIb3DQEBCwUAA4IBAQBzLXpw\n"
        "Xmrg1sQTmzMnS24mREKxj9B3YILmgsdBMrHkH07QUROee7IbQ8gfBKeln0dEcfYi\n"
        "Jyh42jn+fmg9AR17RP80wPthD2eKOt4WYNkNM3H8U4JEo+0ML0jZyswynpR48h/E\n"
        "m96sm/NUeKUViD5iVTb1uHL4j8mQAN1IbXcunXvrrek1CzFVn5Rpah0Tn+6cYVKd\n"
        "Jg531i53udzusgZtV1NPZ82tzYkPQG1vxB//D9vd0LzmcfCvT50MKhz0r/c5yJYk\n"
        "i9q94DBuzMhe+O9j+Ob2pVQt5akVFJVtIVSfBZzRBAd66u9JeADlT4sxwS4QAUHi\n"
        "RrCsEpJsnJXkx/6O\n"
        "-----END CERTIFICATE-----";
  // cspell:enable
  // Borrowed from
  // https://github.com/Azure/azure-sdk-for-go/blob/139b3a3e151f6452a7c2714b0ebeb835036b1a06/sdk/azidentity/testdata/certificate-with-chain.pem#L1
  else if (format == RsaRawCertChain)
    cert << // cspell:disable
        "-----BEGIN RSA PRIVATE KEY-----\n"
        "MIIEowIBAAKCAQEAunkGHWyBYbIp6G97dwFeMhB/7c/y1SPlABi6cUJ6hp7gFeRm\n"
        "Nwl4gDvBmY8e8t6ANQxn3vv3HOp/QZmFl7Cr8aSjvD0JAT2CBbQ/O/Lgzb+5FaGR\n"
        "vBFbBJ4AcXeHnzJ4ilsCrTJXtIWfo497uAHePQ7F3AtC9vLlf3kOoc7EIkdJ00Cf\n"
        "+EKjTbU4UhgBUq+zqPMc8QTUyYXvgb8AxPCTJAktL9tiVpsthmK0SsOEZUiscL/U\n"
        "Ga/N4EonCklD1AAgWHye0bl0kDhzjJSHAuKBrQ6zLIRs6+9OB6Pg4gcmH+Rup5H2\n"
        "dSO09N/YBCiiJZTSlqockB3oym2t5z9et2SiNwIDAQABAoIBAQCKzivPG0X0AztO\n"
        "2i19mHcVrVKNI44POnjsaXvfcyzhqMIFic7MiTA5xEGInRDcmOO2mVV4lvaLf8La\n"
        "gfz/vXNAnN2E8aoSUkbHGDU52sGcZmrPv0VMSV8HQNXzoJZD2r3/v19urVq79fuv\n"
        "NM9TWZCkwqpl8bwXNxe+m85YhCFboY9G543qmuXzKAQLoSupT0e4eIo2IGp7eJYK\n"
        "5J/wtlEumUdhsKo1ajLojDgsgPKfrCyvsmO+bj1dRKGXVLO2SL2pFVCjjHF4SP3q\n"
        "1WX39beu61Zu+kGthDgj5muHgH06FtnWoHLIUrRmYpM+ezCxQHdRWz7AYjheeE7q\n"
        "QqJv1PqBAoGBAOlb/gzsps+rInE+LQoEzVj8osILI4NxIpNc6+iG81dEi+zQABX/\n"
        "bHV6hXGGceozVcX4B+V7f08PlZIAgM3IDqfy0fH2pwEQahJ8a3MwzCgR66RxYlkX\n"
        "E8czkoz0pcHW58FnLLlWXpHRALTtqoPP5LnWs0SmoNvcHZ9yjJ6tvpRlAoGBAMyQ\n"
        "fytsyla1ujO0l/kuLFG7gndeOc96SutH3V17lZ1pN0efHyk2aglOnl6YsdPKLZvZ\n"
        "3ghj01HV0Q0f//xpftduuA7gdgDzSG1irXsxEidfVxX7RsPxX6cx8dhYnuk5rz5E\n"
        "XyTko7zTpr+A4XMnq6+JNSSCIE+CVYcYf/hyemxrAoGAeC9py4xCaWgxR/OGzMcm\n"
        "X3NV++wysSqebRkJYuvF/icOjbuen7W6TVL50Ts2BjHENj6FCpqtObHEDbr2m4Uy\n"
        "jysPF7g50OF8T+MGkAAM1YJNQ5cl2M564DhefPwvNoMRP1l8/kNOV3k2DPjuvg5f\n"
        "NZsvHudWp4VZOFqNs9e19MUCgYAjewCDoKfrqDN2mmEtmAOZ3YMAfzhZsyVhb6KG\n"
        "f1Pw7HnpE0FNXaHAoYE4eRWG3W9Rs9Ud8WqKrCJJO36j4gxdA1grRGVTPt8WEeJz\n"
        "FozGhXPOXTnl7GyhzDjdRGmznAy4KRWziXCY5MDsQEdaOMw/cvXjsio2gC2jc+1m\n"
        "QzzWpwKBgHzszJ5s6vcWElox4Yc1elQ8xniPpo3RtfXZOLX8xA4eR9yQawah1zd6\n"
        "ChfeYbHVfq007s+RWGTb+KYQ6ic9nkW464qmVxHGBatUo9+MR4Gk8blANoAfHxdV\n"
        "g6JNgT2kIGu9IEwoD6XQldC/v24bvFSesyGRHNdI4mUG+hhU4aNw\n"
        "-----END RSA PRIVATE KEY-----\n"
        "-----BEGIN CERTIFICATE-----\n"
        "MIID7zCCAdcCAQEwDQYJKoZIhvcNAQEFBQAwPjELMAkGA1UEBhMCVVMxDDAKBgNV\n"
        "BAoMA3h5ejEMMAoGA1UECwwDYWJjMRMwEQYDVQQDDApJTlRFUklNLUNOMCAXDTIw\n"
        "MDgyMTE3MTA0M1oYDzMzODkwODA0MTcxMDQzWjA7MQswCQYDVQQGEwJVUzEMMAoG\n"
        "A1UECgwDeHl6MQwwCgYDVQQLDANhYmMxEDAOBgNVBAMMB1VTRVItQ04wggEiMA0G\n"
        "CSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC6eQYdbIFhsinob3t3AV4yEH/tz/LV\n"
        "I+UAGLpxQnqGnuAV5GY3CXiAO8GZjx7y3oA1DGfe+/cc6n9BmYWXsKvxpKO8PQkB\n"
        "PYIFtD878uDNv7kVoZG8EVsEngBxd4efMniKWwKtMle0hZ+jj3u4Ad49DsXcC0L2\n"
        "8uV/eQ6hzsQiR0nTQJ/4QqNNtThSGAFSr7Oo8xzxBNTJhe+BvwDE8JMkCS0v22JW\n"
        "my2GYrRKw4RlSKxwv9QZr83gSicKSUPUACBYfJ7RuXSQOHOMlIcC4oGtDrMshGzr\n"
        "704Ho+DiByYf5G6nkfZ1I7T039gEKKIllNKWqhyQHejKba3nP163ZKI3AgMBAAEw\n"
        "DQYJKoZIhvcNAQEFBQADggIBADfitSfjlYa2inBKlpWN8VT0DPm5uw8EHuwLymCM\n"
        "WYrQMCuQVE2xYoqCSmXj6KLFt8ycgxHsthdkAzXxDhawaKjz2UFp6nszmUA4xfvS\n"
        "mxLSajwzK/KMBkjdFL7TM+TTBJ1bleDbmoJvDiUeQwisbb1Uh8b3v/jpBwoiamm8\n"
        "Y4Ca5A15SeBUvAt0/Mc4XJfZ/Ts+LBAPevI9ZyU7C5JZky1q41KPklEHfFZKQRfP\n"
        "cTyTYYvlPoq57C8XPDs6r50EV3B6Z8MN21OB6MVGi8BOY/c7a2h1ZOhxNyBnJuQX\n"
        "w4meJthoKcHUnAs8YCrEoQKayMqPH0Vdhaii/gx4jAgh4PNyIZz5cAst+ybPtQj4\n"
        "i7LFEWjxis+NLQMHhyE4fIGIkEjzU0uGDugifheIwKALqYEgMDrcoolwvGMdPxGo\n"
        "Qps7tkad5vZV9d9+tTbI+DMB16Y51S04/u1dGFz3jSrDVF08PznJc99VB69OReiC\n"
        "K17n8Xyox/VAaYsRFbOAJpLRWwcnotDpFQbgiLrmXxNOoiWPNbQsQzaQx7cR9okQ\n"
        "v5RTpFAkrdjadhMsXFFiQh+axlaGD368ZGAj5ZoyOiXkV88tNCtyP/RDgW5ftQQ7\n"
        "fdv05bNXhDfLgEgQvVSDfClDL1hKukLmLQS3ILfB4FlM/XmE+FW/qgo9aSx2XIbx\n"
        "E4ie\n"
        "-----END CERTIFICATE-----\n"
        "-----BEGIN CERTIFICATE-----\n"
        "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\n"
        "wOnOIqg0ZZcqfvpxXA==\n"
        "-----END CERTIFICATE-----";
  // cspell:enable
}

std::vector<std::string> SplitString(const std::string& s, char separator)
{
  std::vector<std::string> result;

  const auto len = s.size();
  size_t start = 0;
  while (start < len)
  {
    auto end = s.find(separator, start);
    if (end == std::string::npos)
    {
      end = len;
    }

    result.push_back(s.substr(start, end - start));

    start = end + 1;
  }

  return result;
}

std::string ToString(std::vector<uint8_t> const& vec)
{
  const size_t size = vec.size();
  std::string str(size, '\0');
  for (size_t i = 0; i < size; ++i)
  {
    str[i] = static_cast<std::string::value_type>(vec[i]);
  }

  return str;
}
} // namespace

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/client_certificate_credential.hpp"

#include "credential_test_helper.hpp"

#include <azure/core/base64.hpp>

#include <cstdio>
#include <fstream>

#include <gtest/gtest.h>

using Azure::Core::Credentials::TokenRequestContext;
using Azure::Core::Http::HttpMethod;
using Azure::Identity::ClientCertificateCredential;
using Azure::Identity::ClientCertificateCredentialOptions;
using Azure::Identity::Test::_detail::CredentialTestHelper;

namespace {
struct TempCertFile final
{
  static const char* const Path;
  ~TempCertFile();
  TempCertFile();
};

std::vector<std::string> SplitString(const std::string& s, char separator);

std::string ToString(std::vector<uint8_t> const& vec);
} // namespace

TEST(ClientCertificateCredential, Regular)
{
  TempCertFile tempCertFile;

  TokenRequestContext tokenRequestContext;
  tokenRequestContext.Scopes = {"https://azure.com/.default"};

      auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        ClientCertificateCredentialOptions options;
        options.Transport.Transport = transport;

        return std::make_unique<ClientCertificateCredential>(
            "01234567-89ab-cdef-fedc-ba8976543210",
            "fedcba98-7654-3210-0123-456789abcdef",
            TempCertFile::Path,
            options);
      },
      {tokenRequestContext, {{}}},
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

  EXPECT_EQ(
      request0.AbsoluteUrl,
      "https://login.microsoftonline.com/01234567-89ab-cdef-fedc-ba8976543210/oauth2/v2.0/token");

  EXPECT_EQ(
      request1.AbsoluteUrl,
      "https://login.microsoftonline.com/01234567-89ab-cdef-fedc-ba8976543210/oauth2/v2.0/token");

  {
    constexpr char expectedBodyStart0[] // cspell:disable
        = "grant_type=client_credentials"
          "&client_assertion_type=urn%3Aietf%3Aparams%3Aoauth%3Aclient-assertion-type%3Ajwt-bearer"
          "&client_id=fedcba98-7654-3210-0123-456789abcdef"
          "&scope=https%3A%2F%2Fazure.com%2F.default"
          "&client_assertion="; // cspell:enable

    constexpr char expectedBodyStart1[] // cspell:disable
        = "grant_type=client_credentials"
          "&client_assertion_type=urn%3Aietf%3Aparams%3Aoauth%3Aclient-assertion-type%3Ajwt-bearer"
          "&client_id=fedcba98-7654-3210-0123-456789abcdef"
          "&client_assertion="; // cspell:enable

    EXPECT_GT(request0.Body.size(), (sizeof(expectedBodyStart0) - 1));
    EXPECT_GT(request1.Body.size(), (sizeof(expectedBodyStart1) - 1));

    EXPECT_EQ(request0.Body.substr(0, (sizeof(expectedBodyStart0) - 1)), expectedBodyStart0);
    EXPECT_EQ(request1.Body.substr(0, (sizeof(expectedBodyStart1) - 1)), expectedBodyStart1);

    EXPECT_NE(request0.Headers.find("Content-Length"), request0.Headers.end());
    EXPECT_GT(
        std::stoi(request0.Headers.at("Content-Length")),
        static_cast<int>(sizeof(expectedBodyStart0) - 1));

    EXPECT_NE(request1.Headers.find("Content-Length"), request1.Headers.end());
    EXPECT_GT(
        std::stoi(request1.Headers.at("Content-Length")),
        static_cast<int>(sizeof(expectedBodyStart1) - 1));

    {
      using Azure::Core::_internal::Base64Url;

      const auto assertion0 = request0.Body.substr((sizeof(expectedBodyStart0) - 1));
      const auto assertion1 = request1.Body.substr((sizeof(expectedBodyStart1) - 1));

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

      constexpr auto ExpectedHeader
          = "{\"x5t\":\"V0pIIQwSzNn6vfSTPv-1f7Vt_Pw\",\"kid\":"
            "\"574A48210C12CCD9FABDF4933EFFB57FB56DFCFC\",\"alg\":\"RS256\",\"typ\":\"JWT\"}";

      EXPECT_EQ(header0, ExpectedHeader);
      EXPECT_EQ(header1, ExpectedHeader);

      constexpr char ExpectedPayloadStart[]
          = "{\"aud\":\"https://login.microsoftonline.com/01234567-89ab-cdef-fedc-ba8976543210/"
            "oauth2/v2.0/token\","
            "\"iss\":\"fedcba98-7654-3210-0123-456789abcdef\","
            "\"sub\":\"fedcba98-7654-3210-0123-456789abcdef\",\"jti\":\"";

      EXPECT_EQ(payload0.substr(0, (sizeof(ExpectedPayloadStart) - 1)), ExpectedPayloadStart);
      EXPECT_EQ(payload1.substr(0, (sizeof(ExpectedPayloadStart) - 1)), ExpectedPayloadStart);

      EXPECT_EQ(Base64Url::Base64UrlDecode(signature0).size(), 256U);
      EXPECT_EQ(Base64Url::Base64UrlDecode(signature1).size(), 256U);
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

namespace {
const char* const TempCertFile::Path = "azure-identity-test.pem";

TempCertFile::~TempCertFile() { std::remove(Path); }

TempCertFile::TempCertFile()
{
  std::ofstream cert(Path, std::ios_base::out | std::ios_base::trunc);

  cert << // cspell:disable
      "Bag Attributes\n"
      "    Microsoft Local Key set: <No Values>\n"
      "    localKeyID: 01 00 00 00 \n"
      "    friendlyName: te-66f5c973-4fc8-4cd3-8acc-64964d79b693\n"
      "    Microsoft CSP Name: Microsoft Software Key Storage Provider\n"
      "Key Attributes\n"
      "    X509v3 Key Usage: 90 \n"
      "-----BEGIN PRIVATE KEY-----\n";
  // cspell:enable

  cert << // cspell:disable
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
      "a0sHfyKuNyvOv7Wud4sa0lwdKPHS+atwL6TNUWCAGkomYADEe3qiYgMXDX9U3hlW\n";
  // cspell:enable

  cert << // cspell:disable
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
      "hp0P5v42PKxmAx4pR0EjNKsd\n";
  // cspell:enable

  cert << // cspell:disable
      "-----END PRIVATE KEY-----\n"
      "Bag Attributes\n"
      "    localKeyID: 01 00 00 00 \n"
      "    1.3.6.1.4.1.311.17.3.71: 61 00 6E 00 74 00 6B 00 2D 00 6C 00 61 00 70 00 "
      "74 00 6F 00 70 00 00 00 \n"
      "subject=CN = azure-identity-test\n"
      "\n"
      "issuer=CN = azure-identity-test\n"
      "\n"
      "-----BEGIN CERTIFICATE-----\n";
  // cspell:enable

  cert << // cspell:disable
      "MIIDODCCAiCgAwIBAgIQNqa9U3MBxqBF7ksWk+XRkzANBgkqhkiG9w0BAQsFADAe\n"
      "MRwwGgYDVQQDDBNhenVyZS1pZGVudGl0eS10ZXN0MCAXDTIyMDQyMjE1MDYwNloY\n"
      "DzIyMjIwMTAxMDcwMDAwWjAeMRwwGgYDVQQDDBNhenVyZS1pZGVudGl0eS10ZXN0\n"
      "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAz3ZuKbpDu7oBMfMF65qO\n"
      "FSBKInKe8N0LBCRgNmzMfZxzL8LoBueLdeEKX6gUGEFi3i9R5qXA3or1Q/teWV3h\n"
      "iwj1WQR4aGPGVhom34QAM6kND/QmtZMnY7weLiXBJxf0WLUL+p+jsJnTtcCdtiTX\n"
      "EZTLWapp2/0NCJ9n41xG3ZfOfxmZWMzEEXcnyNMq4kkQXGFdpINM3lwsX5grwd62\n"
      "+iNSqaFBR5ZHh7ZHg8JtFR1BLeB8/IIXAdNLSOXktnx9qz5CDUCnOvtEFAtiiAkA\n"
      "vhsybGA28EDmqOVYZPNB+S0bjPTXc7/n1N5S55LWAoF4C/QF+C/0fWeD87bmqP6m\n";
  // cspell:enable

  cert << // cspell:disable
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

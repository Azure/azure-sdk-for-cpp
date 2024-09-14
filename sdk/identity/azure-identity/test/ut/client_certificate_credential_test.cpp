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
  RsaRawReverse
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
    // cspell:disable
    std::string x5t = "\"V0pIIQwSzNn6vfSTPv-1f7Vt_Pw\"";
    std::string kid = "\"574A48210C12CCD9FABDF4933EFFB57FB56DFCFC\"";
    std::string x5c
        = "\"MIIDODCCAiCgAwIBAgIQNqa9U3MBxqBF7ksWk+"
          "XRkzANBgkqhkiG9w0BAQsFADAeMRwwGgYDVQQDDBNhenVyZS1pZGVudGl0eS10ZXN0MCAXDTIyMDQyMjE1MDYw"
          "NloYDzIyMjIwMTAxMDcwMDAwWjAeMRwwGgYDVQQDDBNhenVyZS1pZGVudGl0eS10ZXN0MIIBIjANBgkqhkiG9w"
          "0BAQEFAAOCAQ8AMIIBCgKCAQEAz3ZuKbpDu7oBMfMF65qOFSBKInKe8N0LBCRgNmzMfZxzL8LoBueLdeEKX6gU"
          "GEFi3i9R5qXA3or1Q/teWV3hiwj1WQR4aGPGVhom34QAM6kND/"
          "QmtZMnY7weLiXBJxf0WLUL+p+jsJnTtcCdtiTXEZTLWapp2/"
          "0NCJ9n41xG3ZfOfxmZWMzEEXcnyNMq4kkQXGFdpINM3lwsX5grwd62+iNSqaFBR5ZHh7ZHg8JtFR1BLeB8/"
          "IIXAdNLSOXktnx9qz5CDUCnOvtEFAtiiAkAvhsybGA28EDmqOVYZPNB+S0bjPTXc7/n1N5S55LWAoF4C/QF+C/"
          "0fWeD87bmqP6m0QIDAQABo3AwbjAOBgNVHQ8BAf8EBAMCBaAwHQYDVR0lBBYwFAYIKwYBBQUHAwIGCCsGAQUFB"
          "wMBMB4GA1UdEQQXMBWCE2F6dXJlLWlkZW50aXR5LXRlc3QwHQYDVR0OBBYEFCoJ5tInmafyNuR0tGxZOz522jl"
          "WMA0GCSqGSIb3DQEBCwUAA4IBAQBzLXpwXmrg1sQTmzMnS24mREKxj9B3YILmgsdBMrHkH07QUROee7IbQ8gfB"
          "Keln0dEcfYiJyh42jn+fmg9AR17RP80wPthD2eKOt4WYNkNM3H8U4JEo+0ML0jZyswynpR48h/Em96sm/"
          "NUeKUViD5iVTb1uHL4j8mQAN1IbXcunXvrrek1CzFVn5Rpah0Tn+"
          "6cYVKdJg531i53udzusgZtV1NPZ82tzYkPQG1vxB//D9vd0LzmcfCvT50MKhz0r/"
          "c5yJYki9q94DBuzMhe+O9j+Ob2pVQt5akVFJVtIVSfBZzRBAd66u9JeADlT4sxwS4QAUHiRrCsEpJsnJXkx/"
          "6O\"";

    if (GetSendCertChain())
    {
      return "{\"x5t\":" + x5t + ",\"kid\":" + kid
          + ",\"alg\":\"RS256\",\"typ\":\"JWT\","
            "\"x5c\":"
          + x5c + "}";
    }
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

TEST(ClientCertificateCredential, GetTokenFromCertInMemory)
{
  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [this](auto transport) {
        ClientCertificateCredentialOptions options;
        options.Transport.Transport = transport;

        std::vector<uint8_t> pkey{
            48,  130, 4,   164, 2,   1,   0,   2,   130, 1,   1,   0,   207, 118, 110, 41,  186,
            67,  187, 186, 1,   49,  243, 5,   235, 154, 142, 21,  32,  74,  34,  114, 158, 240,
            221, 11,  4,   36,  96,  54,  108, 204, 125, 156, 115, 47,  194, 232, 6,   231, 139,
            117, 225, 10,  95,  168, 20,  24,  65,  98,  222, 47,  81,  230, 165, 192, 222, 138,
            245, 67,  251, 94,  89,  93,  225, 139, 8,   245, 89,  4,   120, 104, 99,  198, 86,
            26,  38,  223, 132, 0,   51,  169, 13,  15,  244, 38,  181, 147, 39,  99,  188, 30,
            46,  37,  193, 39,  23,  244, 88,  181, 11,  250, 159, 163, 176, 153, 211, 181, 192,
            157, 182, 36,  215, 17,  148, 203, 89,  170, 105, 219, 253, 13,  8,   159, 103, 227,
            92,  70,  221, 151, 206, 127, 25,  153, 88,  204, 196, 17,  119, 39,  200, 211, 42,
            226, 73,  16,  92,  97,  93,  164, 131, 76,  222, 92,  44,  95,  152, 43,  193, 222,
            182, 250, 35,  82,  169, 161, 65,  71,  150, 71,  135, 182, 71,  131, 194, 109, 21,
            29,  65,  45,  224, 124, 252, 130, 23,  1,   211, 75,  72,  229, 228, 182, 124, 125,
            171, 62,  66,  13,  64,  167, 58,  251, 68,  20,  11,  98,  136, 9,   0,   190, 27,
            50,  108, 96,  54,  240, 64,  230, 168, 229, 88,  100, 243, 65,  249, 45,  27,  140,
            244, 215, 115, 191, 231, 212, 222, 82,  231, 146, 214, 2,   129, 120, 11,  244, 5,
            248, 47,  244, 125, 103, 131, 243, 182, 230, 168, 254, 166, 209, 2,   3,   1,   0,
            1,   2,   130, 1,   1,   0,   196, 25,  34,  186, 40,  137, 59,  153, 238, 229, 66,
            220, 150, 189, 233, 141, 72,  143, 42,  142, 131, 116, 18,  204, 5,   231, 72,  86,
            73,  114, 229, 76,  139, 136, 90,  200, 45,  54,  146, 25,  35,  85,  69,  227, 190,
            210, 202, 131, 100, 150, 255, 232, 111, 70,  166, 84,  92,  207, 28,  124, 43,  38,
            213, 57,  49,  135, 143, 211, 236, 232, 103, 95,  35,  37,  231, 22,  161, 83,  251,
            128, 208, 139, 2,   241, 207, 241, 191, 106, 195, 119, 23,  145, 178, 72,  124, 153,
            7,   124, 98,  77,  76,  182, 82,  8,   165, 24,  8,   112, 74,  169, 249, 236, 113,
            99,  225, 102, 118, 87,  109, 146, 243, 82,  145, 197, 84,  4,   220, 16,  157, 120,
            190, 161, 99,  225, 19,  104, 46,  163, 104, 53,  4,   249, 54,  129, 177, 116, 251,
            113, 111, 74,  180, 60,  67,  1,   135, 131, 235, 144, 39,  10,  208, 164, 159, 146,
            7,   212, 242, 185, 1,   90,  184, 100, 121, 255, 4,   170, 31,  221, 220, 20,  126,
            88,  11,  255, 175, 228, 142, 234, 91,  166, 80,  123, 71,  251, 172, 66,  144, 15,
            38,  198, 87,  57,  1,   148, 162, 114, 24,  37,  88,  152, 249, 251, 93,  175, 240,
            38,  184, 176, 179, 225, 89,  33,  241, 243, 127, 121, 180, 144, 152, 160, 196, 151,
            127, 31,  4,   235, 156, 164, 160, 248, 127, 40,  6,   9,   224, 238, 86,  75,  116,
            22,  22,  28,  193, 137, 132, 13,  2,   129, 129, 0,   227, 201, 168, 91,  187, 51,
            122, 11,  223, 46,  229, 5,   107, 75,  7,   127, 34,  174, 55,  43,  206, 191, 181,
            174, 119, 139, 26,  210, 92,  29,  40,  241, 210, 249, 171, 112, 47,  164, 205, 81,
            96,  128, 26,  74,  38,  96,  0,   196, 123, 122, 162, 98,  3,   23,  13,  127, 84,
            222, 25,  86,  235, 57,  45,  96,  88,  244, 222, 217, 209, 131, 136, 129, 142, 159,
            39,  114, 18,  193, 84,  183, 247, 89,  222, 83,  61,  28,  53,  84,  171, 184, 101,
            110, 55,  203, 192, 81,  133, 133, 149, 251, 118, 120, 179, 89,  242, 48,  64,  192,
            52,  86,  202, 70,  98,  251, 44,  48,  116, 230, 133, 135, 116, 147, 35,  20,  175,
            203, 233, 135, 2,   129, 129, 0,   233, 40,  85,  226, 253, 145, 66,  37,  243, 145,
            110, 134, 29,  32,  89,  12,  133, 71,  9,   229, 176, 188, 137, 93,  58,  81,  31,
            27,  188, 51,  101, 110, 30,  243, 3,   67,  133, 244, 140, 181, 124, 137, 175, 55,
            219, 19,  37,  193, 181, 82,  168, 17,  39,  79,  232, 217, 230, 245, 110, 223, 181,
            204, 167, 91,  222, 185, 75,  222, 19,  30,  236, 187, 250, 135, 239, 30,  142, 137,
            2,   111, 32,  255, 75,  151, 239, 76,  25,  219, 162, 212, 242, 168, 100, 36,  196,
            214, 154, 195, 141, 4,   210, 139, 193, 76,  228, 21,  62,  213, 26,  199, 90,  6,
            88,  154, 155, 84,  75,  190, 55,  147, 81,  23,  80,  130, 119, 237, 34,  231, 2,
            129, 129, 0,   224, 86,  26,  153, 61,  36,  213, 110, 27,  191, 243, 142, 2,   189,
            66,  4,   212, 3,   0,   19,  4,   235, 137, 226, 233, 152, 246, 49,  118, 198, 193,
            44,  104, 187, 187, 60,  33,  176, 13,  5,   184, 36,  113, 191, 213, 75,  119, 118,
            10,  166, 28,  134, 170, 196, 105, 102, 158, 14,  158, 166, 27,  195, 148, 12,  239,
            143, 58,  107, 11,  12,  159, 195, 248, 210, 133, 230, 178, 209, 244, 189, 185, 189,
            168, 88,  18,  55,  85,  206, 173, 208, 77,  20,  223, 158, 171, 41,  158, 118, 145,
            223, 182, 45,  233, 117, 12,  48,  185, 148, 145, 247, 103, 255, 42,  60,  116, 168,
            196, 120, 195, 201, 67,  230, 37,  6,   154, 156, 24,  193, 2,   129, 128, 89,  174,
            98,  26,  197, 93,  178, 11,  214, 252, 1,   127, 36,  132, 228, 187, 164, 40,  6,
            238, 138, 100, 122, 145, 153, 116, 202, 122, 104, 145, 124, 90,  55,  29,  82,  12,
            14,  226, 89,  157, 21,  34,  0,   123, 131, 144, 129, 221, 88,  89,  74,  188, 14,
            90,  246, 61,  154, 80,  157, 44,  255, 72,  144, 183, 13,  156, 72,  74,  75,  32,
            129, 254, 108, 7,   13,  58,  187, 168, 135, 127, 39,  58,  11,  97,  141, 113, 192,
            32,  115, 41,  101, 140, 207, 184, 52,  222, 54,  249, 48,  172, 194, 47,  111, 82,
            156, 8,   111, 250, 152, 42,  69,  190, 91,  38,  92,  200, 183, 185, 154, 182, 215,
            98,  24,  93,  150, 179, 103, 209, 2,   129, 128, 55,  188, 105, 17,  190, 48,  228,
            148, 77,  53,  227, 1,   91,  44,  108, 54,  52,  82,  207, 120, 220, 83,  125, 214,
            160, 102, 106, 3,   106, 15,  42,  51,  40,  0,   42,  14,  130, 175, 117, 28,  76,
            208, 137, 72,  14,  115, 169, 182, 62,  0,   125, 186, 73,  212, 148, 225, 52,  226,
            67,  96,  26,  84,  248, 12,  232, 241, 74,  30,  116, 3,   96,  157, 219, 89,  218,
            125, 117, 69,  40,  230, 149, 14,  147, 19,  245, 134, 150, 200, 171, 28,  76,  106,
            175, 172, 191, 151, 189, 115, 158, 132, 38,  215, 45,  196, 217, 192, 43,  110, 201,
            178, 134, 157, 15,  230, 254, 54,  60,  172, 102, 3,   30,  41,  71,  65,  35,  52,
            171, 29,  171};

        std::string certString
            = "-----BEGIN CERTIFICATE-----\n"
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

        std::vector<uint8_t> cert(certString.begin(), certString.end());

        return std::make_unique<ClientCertificateCredential>(
            "01234567-89ab-cdef-fedc-ba8976543210",
            "fedcba98-7654-3210-0123-456789abcdef",
            cert,
            pkey,
            options);
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

  EXPECT_EQ(
      request0.AbsoluteUrl,
      "https://login.microsoftonline.com/01234567-89ab-cdef-fedc-ba8976543210/oauth2/v2.0/token");

  EXPECT_EQ(
      request1.AbsoluteUrl,
      "https://login.microsoftonline.com/01234567-89ab-cdef-fedc-ba8976543210/oauth2/v2.0/token");

  {
    std::string expectedStr1
        = "grant_type=client_credentials"
          "&client_assertion_type=urn%3Aietf%3Aparams%3Aoauth%3Aclient-assertion-type%3Ajwt-"
          "bearer"
          "&client_id=fedcba98-7654-3210-0123-456789abcdef"
          "&scope=https%3A%2F%2Fazure.com%2F.default"
          "&client_assertion="; // cspell:enable

    std::string expectedStr2
        = "grant_type=client_credentials"
          "&client_assertion_type=urn%3Aietf%3Aparams%3Aoauth%3Aclient-assertion-type%3Ajwt-bearer"
          "&client_id=fedcba98-7654-3210-0123-456789abcdef"
          "&client_assertion=";

    auto expectedBodyStart0 = expectedStr1;
    auto expectedBodyStart1 = expectedStr2;

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

      std::string ExpectedHeader
          = "{\"x5t\":\"V0pIIQwSzNn6vfSTPv-1f7Vt_Pw\",\"kid\":"
            "\"574A48210C12CCD9FABDF4933EFFB57FB56DFCFC\",\"alg\":\"RS256\",\"typ\":\"JWT\"}";

      EXPECT_EQ(header0, ExpectedHeader);
      EXPECT_EQ(header1, ExpectedHeader);

      std::string ExpectedPayloadStart
          = "{\"aud\":\"https://login.microsoftonline.com/01234567-89ab-cdef-fedc-ba8976543210/"
            "oauth2/v2.0/token\","
            "\"iss\":\"fedcba98-7654-3210-0123-456789abcdef\","
            "\"sub\":\"fedcba98-7654-3210-0123-456789abcdef\",\"jti\":\"";

      EXPECT_EQ(payload0.substr(0, ExpectedPayloadStart.size()), ExpectedPayloadStart);
      EXPECT_EQ(payload1.substr(0, ExpectedPayloadStart.size()), ExpectedPayloadStart);

      EXPECT_EQ(Base64Url::Base64UrlDecode(signature0).size(), 256);
      EXPECT_EQ(Base64Url::Base64UrlDecode(signature1).size(), 256);
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
        testing::Values(RsaPkcs, RsaRaw, RsaRawReverse),
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
  else if (format == RsaRawReverse)
    cert << // cspell:disable
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
        "-----END CERTIFICATE-----\n"
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
        "-----END RSA PRIVATE KEY-----";
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

// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @brief This sample provides the code implementation to use the Key Vault Certificates SDK client
 * for C++ to import a certificate.
 *
 * @remark The following environment variables must be set before running the sample.
 * - AZURE_KEYVAULT_URL:  To the Key Vault account URL.
 *
 */

#include <azure/core.hpp>
#include <azure/identity.hpp>
#include <azure/keyvault/certificates.hpp>

#include <chrono>
#include <iostream>
#include <thread>

using namespace Azure::Security::KeyVault::Certificates;
using namespace std::chrono_literals;

std::string GetPemCertificate();
std::string GetPkcsCertificate();
void PurgeCertificate(
    std::string const& certificateName,
    CertificateClient const& certificateClient);

int main()
{
  auto const keyVaultUrl = std::getenv("AZURE_KEYVAULT_URL");
  auto credential = std::make_shared<Azure::Identity::DefaultAzureCredential>();
  std::chrono::milliseconds defaultWait(10s);
  // create client
  CertificateClient certificateClient(keyVaultUrl, credential);

  try
  {
    // certificate names
    std::string const pemName = "Pem1";
    std::string const pkcsName = "Pkcs1";
    // import pem certificate
    {
      // prepare the parameters
      ImportCertificateOptions options;
      options.Certificate = GetPemCertificate();

      options.Policy.Enabled = true;
      options.Policy.KeyType = CertificateKeyType::Rsa;
      options.Policy.KeySize = 2048;
      options.Policy.ContentType = CertificateContentType::Pem;
      options.Policy.Exportable = true;

      // call import API
      auto imported = certificateClient.ImportCertificate(pemName, options).Value;
      // get some value from the certificate
      std::cout << "Imported pem certificate with name " << imported.Name();
    }
    // import pkcs certificate
    {
      // prepare the parameters
      ImportCertificateOptions options;
      options.Certificate = GetPkcsCertificate();

      options.Policy.Enabled = true;
      options.Policy.KeyType = CertificateKeyType::Rsa;
      options.Policy.KeySize = 2048;
      options.Policy.ContentType = CertificateContentType::Pkcs12;
      options.Policy.Exportable = true;

      // call the import API
      auto imported = certificateClient.ImportCertificate(pkcsName, options).Value;
      // read something from the certificate
      std::cout << "Imported pkcs certificate with name " << imported.Name();
    }
    // delete the certificates, and get deleted
    {
      // delete the certificates
      auto response1 = certificateClient.StartDeleteCertificate(pemName);
      auto response2 = certificateClient.StartDeleteCertificate(pkcsName);
      response1.PollUntilDone(defaultWait);
      response2.PollUntilDone(defaultWait);
      // purge the certificates
      PurgeCertificate(pkcsName, certificateClient);
      PurgeCertificate(pemName, certificateClient);
    }
  }
  catch (Azure::Core::Credentials::AuthenticationException const& e)
  {
    std::cout << "Authentication Exception happened:" << std::endl << e.what() << std::endl;
    return 1;
  }
  catch (Azure::Core::RequestFailedException const& e)
  {
    std::cout << "Key Vault Certificate Client Exception happened:" << std::endl
              << e.Message << std::endl;
    return 1;
  }

  return 0;
}

/* cSpell:disable */
std::string GetPemCertificate()
{
  static std::string pemCertificate
      = "-----BEGIN CERTIFICATE-----\n"
        "MIIDazCCAlOgAwIBAgIUJTCBp8Wq5B9zMXrKG00mIPose70wDQYJKoZIhvcNAQEN\n"
        "BQAwRTELMAkGA1UEBhMCQVUxEzARBgNVBAgMClNvbWUtU3RhdGUxITAfBgNVBAoM\n"
        "GEludGVybmV0IFdpZGdpdHMgUHR5IEx0ZDAeFw0yNTAyMTgxOTMzNDZaFw0zNTAy\n"
        "MTYxOTMzNDZaMEUxCzAJBgNVBAYTAkFVMRMwEQYDVQQIDApTb21lLVN0YXRlMSEw\n"
        "HwYDVQQKDBhJbnRlcm5ldCBXaWRnaXRzIFB0eSBMdGQwggEiMA0GCSqGSIb3DQEB\n"
        "AQUAA4IBDwAwggEKAoIBAQCUrvQCG0+gyNc0nRx3KofL8smE5aIGRvEqn5N1K4Su\n"
        "0tsUKP6f1xBf+mE4Gn436Dw5OfJQLU2sjsL/n5nKEF8g/OmzXwtBSFHwJe5eOuxO\n"
        "YWR/6zB+N/TGyuOGa/sGfKXt8KikjPKem20Ts5OeVE0QIfZgMc0MupYx+ZpDx4DE\n"
        "qJ/FANbkrWtwjHCPUouqaA58AouuLuRSCHhixAz9uYRGiIJQHH1tGV1M15N4DU8m\n"
        "blYb/B2tiEjNOzsc9ATxDJ/N/RlkgXj4nGr0o6w1VT1d2XYc7XJrHbImiyyluFzw\n"
        "RjkoniBp8Grb7zQTeOS/mHtkTvo2ddwS4Ak1FeVClUTxAgMBAAGjUzBRMB0GA1Ud\n"
        "DgQWBBSzxOWPBC6/L1CKVHOrYCEtwNrVQDAfBgNVHSMEGDAWgBSzxOWPBC6/L1CK\n"
        "VHOrYCEtwNrVQDAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3DQEBDQUAA4IBAQBY\n"
        "SSU5F21v0Vby0oked/p6FeqTdDPeubRdtTXJ1wnuMtn51p3KwxlgAK/3hfO+dbHZ\n"
        "a0tYRckd4qwZD/XuZ4RoBOzg4iFQ2KnVNyOJcX3VBRvc4yklRgCO30OXZZ0fcyo5\n"
        "JttsKbjUCYUHGXVKh05p9iXoDuAloq1Pd/GRYoQhvv9M0qyGtZEdvofKRalk0Cm8\n"
        "U1m1gkbP8F6BhNmmYnONoVvJdnhfMTFVfd82VBXUWNhFiy8bP1YcHb1FI64K69oq\n"
        "LJxM7NiG7BURxJM0Psg/5yq9k/DOzcGjwFE0Jk/obkUxc5i36W+zbokAgFJ0xewy\n"
        "79A48BGxjof2aNj5B63z\n"
        "-----END CERTIFICATE-----\n"
        "-----BEGIN PRIVATE KEY-----\n"
        "MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQCUrvQCG0+gyNc0\n"
        "nRx3KofL8smE5aIGRvEqn5N1K4Su0tsUKP6f1xBf+mE4Gn436Dw5OfJQLU2sjsL/\n"
        "n5nKEF8g/OmzXwtBSFHwJe5eOuxOYWR/6zB+N/TGyuOGa/sGfKXt8KikjPKem20T\n"
        "s5OeVE0QIfZgMc0MupYx+ZpDx4DEqJ/FANbkrWtwjHCPUouqaA58AouuLuRSCHhi\n"
        "xAz9uYRGiIJQHH1tGV1M15N4DU8mblYb/B2tiEjNOzsc9ATxDJ/N/RlkgXj4nGr0\n"
        "o6w1VT1d2XYc7XJrHbImiyyluFzwRjkoniBp8Grb7zQTeOS/mHtkTvo2ddwS4Ak1\n"
        "FeVClUTxAgMBAAECggEAP0rCgGsT2IPmaHVA+tFWAVGWy0XdVQK6AsTvRn+B+dG0\n"
        "hL52taAdTl4NKI7OkAoCKPIr3kmKuRwEkkBkfQm/FRHUghDBf8bDapEbCW3Xzf3T\n"
        "hEif2fsD7ePMOxqYP0iscb9yaqm9VhsfsbVN76Y/L9DlSkmo2fRLn3Z7XsL8Y7om\n"
        "Pt3bFTQLpp8tVoItHhCfIC3ghW0fb6E7uHgDCbIxel8pNklAZ9cl3h41R31Nc3Qx\n"
        "tL5F/fbIj2qV4JoWwqUG2LsA03A30zl2indCsg1WAhbHatdik7KyX7iiqIhj96nw\n"
        "Qbee1k0RE542h04GSgsWf+PFGpzSldkkI/KHc8GwgQKBgQDDaYijxXzTDjozTI7t\n"
        "Zcd/5SFnjBqoHJQ3miV2CTKpMfsU44PJp2Sezo4NTYp3RMAXvr+9dpC3AVo5VHoU\n"
        "ptZsvM5u14dHzj1uxfJQj1WPC/3NM+5Fr2QigGgaIx5iDgOpApw55HnVgYTmgSv4\n"
        "aH+9ulKjO1BVvDj+OD1VYTT+KQKBgQDCyGjSGbXdIG2MXP6R84eRdp/yRoyCkM6b\n"
        "1WCB6E+fQ5pCe/jA1Ezob4fyW3O7AYtAwoFpVXGN4q5AD5MRtAQoHOSB7rH2jXHa\n"
        "ivP3mLYRdzLEAIR02VWOrCW63az4Z94aEjl3i0goOPcMKteLPHRVZOLD+TOulFs6\n"
        "gsbgK5VZiQKBgH+mYc7xw/+u7I/0M+2aLf86815L5GHrAVttA4meqoxiDO1xTfad\n"
        "tZoTvjGsdIZCz+TzAxfF6vTRGloW+ASjk7DGiWdYvZNHg1PoVMkRSMfAApRYlw7I\n"
        "avYqwVZJmKBfHT77aIhc2sWA7QsSWGHWCZQrsa3Voj3PTb+rh01Nyas5AoGAYJBO\n"
        "Nup8GJX8+TsZEW4z5U9z1/5Vplz3sJXF9GEhKAmxrggbuC0Hu6ufL+1aoHkRehdz\n"
        "ZK7xTq/0RtnxxTMDZId+WVPmch6JNRjx/et9R4daaYUAJZWhLsv1IIDJQZTsrOwg\n"
        "BT3NlfM3tpZ+qQ5ddgA/03v1vTTbTVSMF5JDxlECgYEAvT2dRagwVczAerumNOhV\n"
        "14KNvsr1NR4c+7V3Ndzhp5UD6GrrnDsWewjeNluutZld8Z5w7sz6MUWF9NYQzyyO\n"
        "rY3m+BKePySPJn5AhtBNeIEs8WbJ+SX26kUPOKZ+vpY2jVOmKXd4jxDxbcacEP3X\n"
        "ZcDkLvRyz3zf6yUI/xiLI5c=\n"
        "-----END PRIVATE KEY-----\n";
  return pemCertificate;
}

std::string GetPkcsCertificate()
{
  static std::string pkcsCertificate
      = "MIIJqQIBAzCCCW8GCSqGSIb3DQEHAaCCCWAEgglcMIIJWDCCBA8GCSqGSIb3DQEHBqCCBAAwggP8AgEAMIID9QYJKo"
        "ZIhvcNAQcBMBwGCiqGSIb3DQEMAQYwDgQIgod/G5XinpsCAggAgIIDyIJWY71/"
        "sB+SAfDuggNL5+ZwETocCSQKg1BENtkrJ/"
        "CN1MkG8wHn1yzGWhG0sE+tnB4z0uK3SEgFnznp85FPcB58WFDNV1UNRgMESaDP0sdMwjDrh5+/"
        "7zNfXBGqGQrmZar4eWa40XyaZJyZire7bYenFbL9rl+f+23yNcHYkyAEFTKYqhq0jxXyR5CcYUhXvMcxaoL+EujfY/"
        "hQGAsgJX+xQEsOhwzpk2mu3eb/60DL0uoEKR5aAVFq0L/"
        "oHvZmOWw2gDR0AzU5+rd+gHOn2piXMYEqhM5lWtv5wfvKZ+dSPGVV2D9oZ5QGe8isyH7sUj8ACWTV5ETG3K+WIFE/"
        "L3eIFa7ecUJCVzEsjwdxCLTKPHTK4pkxqKm0g1bRjRWCTw+"
        "EsespifpZWZW1dC8DxzMrzTgMAzFCBBqzUyCDzU1ujyyXAfJ6WzG9MsMcG5w/"
        "EQ5WsWyGKv28MNh9jKGsPMIdlfaOUxs5JYPVbIH3eFrF1nN6Pk2YDu2pk5nQvO42wLqCIrOCLbcMHSSSl+"
        "eX9RY5naiMPxa3JRhbyxe8ARhZQLbLekAn6wVTerCkF7K0G06k4eI25ZJrd3TAzgqyubQlNqyFKochb4XNQwvPm9mF"
        "nsnVcRXmgFJZ2kWyS1+xaD52OqLxCk06tUKyjJKor9uw7uVdXEanBVtm9NYjREx2MK8Kh5kjRGTsyDdpjei2EgT+"
        "wBDuYm1vpsVu2cfsvOl4tiA0jDCqKNBUC3r7r3LaABOewXwPj/"
        "T79y4yvgd+"
        "8sgt12uBV1Hsh1TNxBmV8ekGp1I8IV6VBiQ3KD60mMcqbzJK6Qoanyigo5rSiHAOtwAKWyJWareAgEdK2LuOViUWTN"
        "2FfVT807XEdEhIGD3CJqzPWb63fRXXnlr0GWFyTJzyFnyGQMCiKWbYL4XAcLPb53ti6HStdx0bsbCFe0/"
        "ZBvGYNnnEEO+"
        "elv4UTSG1hXaRh183zQeIIDbFKQEPW0cvjxXtnBQcUQtBTN7ttdOlYFKvjiNsUzHjZgmDS4X05FTAk0wUfWqMBjRJV"
        "3CmUCUo/c8sfLQb7h7X8+dYTon73zmGAZ9rixZfy0fr3pEG1ienDo6fodytav/euk/ZiiHw/+B9g34T1xF/"
        "VX2iQa1YmMbMLFQR3CsNNgPx6uveptNyFAtFaXAtnPuRN4QbrtugXggF7EyNWLoIL+"
        "l6Ab1Dp5CnCIFWxWppVfifnsUehmb254s8E4a2wucy8M58J0DVWdX7JP/"
        "ZFYQhMtfvX48auDoKp9NwDaj4hulFqMImHahpk+"
        "G0bzUvMIIFQQYJKoZIhvcNAQcBoIIFMgSCBS4wggUqMIIFJgYLKoZIhvcNAQwKAQKgggTuMIIE6jAcBgoqhkiG9w0B"
        "DAEDMA4ECDccMkcYSf/"
        "AAgIIAASCBMhsCZB7YoWzbRYnhEgHnfRQSLUiXeEiyew4Zne1ROQWbHBXzugn+"
        "Gmmh9OdR2QpAVLkKE25rC4ac9KieifcSYMUyZ8XV6Bu43RIQUGKQiNeIe9ea0/D5XhyfeDeaTuT+lqzZ6doew/B/"
        "ahN/"
        "U3+l2mbR6JmVmFmLswGX3mpfaiKHJSFQNdJxRollN2oBG5YxwW8m+"
        "6XiJXnwiZ9Fw96FpK8PZncSTLuU2xcMORIcme6LF3XZkH5NULahFwaNrDFziyVzh14mUEhsM9h81HZkn4ZavvwMBhs"
        "khlBtgjvFHydmlvJG36YqrzPSXhPJByGqXlZlFElHWSa+"
        "ypSVvVsbDznmbu8QhmqBj5k6tT9NkqkHCXvBS3GZVOMlcmIiQDRE7dmrKATdaEQUZprCv5SYj/"
        "BabP2U57HnBfc4efUGxxO2i5Ib77WuuPj2omc5K5oUCKHz4LMe4mELUB7aam9guh01E7IKnXmZYkG3qRSUPHJYRadD"
        "QeKHDi98DGg2J6oKJTVVrD8cCJFNKm11t9MoN71KSLOHUMZrNhUBW5teXgMbYuFu/Eh/9HlDJidT1+fzGXFD/"
        "FZS7mjGa2KNfrbJsgEfV4w0Lum98b1rV0cuZNs6OGpesrU0FVf146Yxc1v9+8xs+"
        "v42pJ8XmZKn9ovWyRySxihpIBAB1kd2pnVcwc8IMI6I3WFvALQ95hTX8/fjp0C/"
        "ePY1tBPryK1NFIJpNsyA61lBVMMdFoGEKiIJVK5Qn1vzg0p7Ex2d7PxgktWGfsnTNf7vI34MbZXOQ+vwHGun9SQ3/"
        "7OuS6HhXodJFyPvV00FeaUpw398eHfmbKUXWurpuJoUc48aN19ElELBcp85XVVTIi9RjcBNrLLlvfa7GBLSGK7Cu45"
        "Vqkd60L78ZF5zP1B9MCN8m82x5AbcRi4HkZylgE4kGf8LdQ7bPkmNTrYlRHPkpKK+dkd/PDkdaRaDSdqU/"
        "304RzLsg7+71QObDEhe3LGVtrQkdXIck1fCQ+oTpmooa8DM2Yyoe6y0XO9Ok7MNuoII2H/5dWN0Mp8eKvN7/"
        "CBMX5J9F9tyInp44B4mk1tT4FXuNona2Sx916KtAp2O71TbtM4stCrqMhr06HCi1ZwLQUGKDRW6fQ/"
        "pb8Ifmpkf1UOP3Vy6O7KN/1UDzE6MXCeyuDC8JGN5JFDCxmInIX/clOcF/"
        "6Bw+JTxcTNh6rY5gxIp1W5IWTx3TKD5mhFMtygmBnCkkPBo6NcE+"
        "zUsrBBaSibJOouZUJCxT8iOVHMHI0lKXvb9ReAM6txUILD4GIHNbDTy91HSrewUNlh9nIyWz4v+pTbN6+"
        "Ixd8gssdWVJB7bpuU5+"
        "HnB3NHeNWp1eHAyXlmApWV19ZPjgicrvFBrZZHinKNa7QeUopMSOQJ85P2a1pJZj3vOsehkSmulhd6qUmgOHwaZGyz"
        "8HODXj+7dS8vHHHnANUwojjxoAR80D88Cqlx+vZcguBZJVY1r6L5r81dExQbh0DRRtQR0IJBcm30xdA4c/"
        "bAhDOFAkTEg/"
        "Bjb4Lp2LA+"
        "3XY0fJOSDSbqMWQVkwvwbAmiXjCGdOjtlZFFlkkCrhCFpBh4HOWC2Xi3oelNfCrNJKAJwDO7QQnKZhRafKPCIH2vKp"
        "QuCzX7G9YxJTAjBgkqhkiG9w0BCRUxFgQUkyBBb/sxRs/"
        "XUVmSy2vdQDLfhoIwMTAhMAkGBSsOAwIaBQAEFML9i8YEH4sB00IWeGvF/wz1x/ziBAhT5brnwDl93QICCAA=";

  return pkcsCertificate;
}

void PurgeCertificate(
    std::string const& certificateName,
    CertificateClient const& certificateClient)
{
  bool retry = true;
  int retries = 5;
  while (retries > 0 && retry)
  {
    try
    {
      retries--;
      certificateClient.PurgeDeletedCertificate(certificateName);
      retry = false;
    }
    catch (Azure::Core::RequestFailedException const& e)
    {
      retry = (e.StatusCode == Azure::Core::Http::HttpStatusCode::Conflict);
      if (!retry)
      {
        throw;
      }
      std::this_thread::sleep_for(std::chrono::seconds(15));
    }
  }
}
/* cSpell:enable */

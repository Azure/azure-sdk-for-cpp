// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "transport_adapter_base_test.hpp"

#include <azure/core/http/policies/policy.hpp>
#include <azure/core/http/transport.hpp>

#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
#include "azure/core/http/curl_transport.hpp"
#endif

#if defined(BUILD_TRANSPORT_WINHTTP_ADAPTER)
#include "azure/core/http/win_http_transport.hpp"

#include <Windows.h>

#include <wincrypt.h>

#include <wil/resource.h>

#endif

#include <string>

using testing::ValuesIn;

namespace Azure { namespace Core { namespace Test {

  /**********************   Define the parameters for the base test and a suffix  ***************/
  namespace {
    // Produces a parameter for the transport adapters tests based in a suffix and an specific
    // adapter implementation
    static TransportAdaptersTestParameter GetTransportOptions(
        std::string suffix,
        std::shared_ptr<Azure::Core::Http::HttpTransport> adapter)
    {
      Azure::Core::Http::Policies::TransportOptions options;
      options.Transport = adapter;
      return TransportAdaptersTestParameter(std::move(suffix), options);
    }

    // When adding more than one parameter, this function should return a unique string.
    static std::string GetSuffix(const testing::TestParamInfo<TransportAdapter::ParamType>& info)
    {
      // Can't use empty spaces or underscores (_) as per google test documentation
      // https://github.com/google/googletest/blob/master/googletest/docs/advanced.md#specifying-names-for-value-parameterized-test-parameters
      return info.param.Suffix;
    }
  } // namespace

  /*********************** Transporter Adapter Tests ******************************/
  /*
   * MSVC does not support adding the pre-processor `#if` condition inside the MACRO parameters,
   * this is why we need to duplicate each case based on the transport adapters built.
   */
#if defined(BUILD_TRANSPORT_WINHTTP_ADAPTER) && defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
  /* WinHTTP + libcurl */
  INSTANTIATE_TEST_SUITE_P(
      Test,
      TransportAdapter,
      testing::Values(
          GetTransportOptions("winHttp", std::make_shared<Azure::Core::Http::WinHttpTransport>()),
          GetTransportOptions("libCurl", std::make_shared<Azure::Core::Http::CurlTransport>())),
      GetSuffix);

#elif defined(BUILD_TRANSPORT_WINHTTP_ADAPTER)
  /* WinHTTP */
  INSTANTIATE_TEST_SUITE_P(
      Test,
      TransportAdapter,
      testing::Values(
          GetTransportOptions("winHttp", std::make_shared<Azure::Core::Http::WinHttpTransport>())),
      GetSuffix);

#elif defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
  /* libcurl */
  INSTANTIATE_TEST_SUITE_P(
      Test,
      TransportAdapter,
      testing::Values(
          GetTransportOptions("libCurl", std::make_shared<Azure::Core::Http::CurlTransport>())),
      GetSuffix);
#else
  /* Custom adapter. Not adding tests */
#endif

#if defined(BUILD_TRANSPORT_WINHTTP_ADAPTER)
  namespace {

    struct CertName : public CERT_NAME_BLOB
    {
      CertName(std::string const& x500dn) : CERT_NAME_BLOB{0}
      {
        // Determine the size needed to encode the buffer.
        CertStrToName(
            X509_ASN_ENCODING,
            x500dn.c_str(),
            CERT_X500_NAME_STR,
            nullptr,
            pbData,
            &cbData,
            nullptr);
        pbData = new BYTE[cbData];
        if (!CertStrToName(
                X509_ASN_ENCODING,
                x500dn.c_str(),
                CERT_X500_NAME_STR,
                nullptr,
                pbData,
                &cbData,
                nullptr))
        {
          throw std::runtime_error("Failed to convert string to name blob");
        }
      }
      ~CertName() { delete pbData; }
    };
  } // namespace
#endif
  std::unique_ptr<Azure::Core::Http::_internal::HttpPipeline>
  TransportAdapter::CreateTlsClientAuthPipelineForTest()
  {
#if defined(BUILD_TRANSPORT_WINHTTP_ADAPTER)
    if (GetParam().Suffix == "winHttp")
    {
      Azure::Core::Http::WinHttpTransportOptions options;
      CertName certName("cn=Test;cn=TlsClient");
      wil::unique_cert_context certContext{CertCreateSelfSignCertificate(
          0, &certName, 0, nullptr, nullptr, nullptr, nullptr, nullptr)};

      options.TlsClientCertificate = certContext.get();
      auto transport = std::make_shared<Azure::Core::Http::WinHttpTransport>(options);
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> retryPolicies;
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;

      Azure::Core::_internal::ClientOptions op;
      op.Retry.RetryDelay = std::chrono::milliseconds(10);
      op.Transport.Transport = transport;
      auto pipeline = std::make_unique<Azure::Core::Http::_internal::HttpPipeline>(
          op, "TransportTest", "X.X", std::move(retryPolicies), std::move(policies));
      return pipeline;
    }
#endif
    return nullptr;
  }

}}} // namespace Azure::Core::Test

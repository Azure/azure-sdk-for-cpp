#include "../../../../attestation/azure-security-attestation/src/private/package_version.hpp"
#include <azure/attestation.hpp>
#include <azure/attestation/attestation_client_models.hpp>
#include <azure/core.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/response.hpp>
#include <string>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys { namespace Test {

  using namespace Azure::Core::Http;
  using namespace Azure::Core::Http::_internal;
  using namespace Azure::Core::Json::_internal;
  using namespace Azure::Core::Http;
  using namespace Azure::Core::Http::Policies;
  using namespace Azure::Core::Http::Policies::_internal;
  using namespace Azure::Security::Attestation;
  using namespace Azure::Security::Attestation::Models;
  using namespace Azure::Security::Attestation::_detail;

  class MaaSandboxClient final {
  public:
    /**
     * @brief Destructor.
     *
     */
    virtual ~MaaSandboxClient() = default;

    /** @brief Construct a new Attestation Client object
     *
     * @param endpoint The URL address where the client will send the requests to.
     * @param credential The authentication method to use (required for TPM attestation).
     * @param options The options to customize the client behavior.
     */
    explicit MaaSandboxClient(
        std::string const& endpoint,
        Azure::Core::_internal::ClientOptions& options,
        std::shared_ptr<Azure::Core::Credentials::TokenCredential const> credential)
        : m_endpoint(endpoint)
    {
      std::vector<std::unique_ptr<HttpPolicy>> perRetrypolicies;
      if (credential)
      {
        m_credentials = credential;
        Azure::Core::Credentials::TokenRequestContext const tokenContext
            = {{"https://attest.azure.net/.default"}};

        perRetrypolicies.emplace_back(
            std::make_unique<BearerTokenAuthenticationPolicy>(credential, tokenContext));
      }
      std::vector<std::unique_ptr<HttpPolicy>> perCallpolicies;

      m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
          options,
          "Attestation",
          Azure::Security::Attestation::_detail::PackageVersion::ToString(),
          std::move(perRetrypolicies),
          std::move(perCallpolicies));
    }

    /** @brief Construct a new anonymous Attestation Client object
     *
     * @param endpoint The URL address where the client will send the requests to.
     * @param options The options to customize the client behavior.
     *
     * @note TPM attestation requires an authenticated attestation client.
     */
    explicit MaaSandboxClient(
        std::string const& endpoint,
        Azure::Core::_internal::ClientOptions options = Azure::Core::_internal::ClientOptions())
        : MaaSandboxClient(endpoint, options, nullptr)
    {
    }

    /** @brief Construct a new Attestation Client object from an existing attestation client.
     *
     * @param attestationClient An existing attestation client.
     */
    explicit MaaSandboxClient(MaaSandboxClient const& sandboxClient)
        : m_endpoint(sandboxClient.m_endpoint), m_apiVersion(sandboxClient.m_apiVersion),
          m_pipeline(sandboxClient.m_pipeline){};

    /**
     * @brief Returns the API version the client was configured with.
     *
     * @returns The API version used when communicating with the attestation service.
     */
    std::string const& ClientVersion() const { return m_apiVersion; }

    /**
     * Retrieves metadata about the attestation signing keys in use by the attestation
     * service.
     *
     * Retrieve the OpenID metadata for this attestation service instance..
     *
     * @return an \ref Models::AttestationOpenIdMetadata object containing metadata about the
     * specified service instance.
     */
    Azure::Response<std::vector<uint8_t>> GenerateQuote(
        AttestationDataType const& dataType,
        std::string const& data,
        Azure::Core::Context const& context = Azure::Core::Context::ApplicationContext) const
    {
      json requestJson;
      std::string encodedData = Azure::Core::_internal::Base64Url::Base64UrlEncode(
          std::vector<uint8_t>(data.begin(), data.end()));
      requestJson["attestedData"]
          = json::object({{"dataType", dataType.ToString()}, {"data", encodedData}});

      std::string serializedRequest = requestJson.dump();
      const auto encodedVector
          = std::vector<uint8_t>(serializedRequest.begin(), serializedRequest.end());
      Azure::Core::IO::MemoryBodyStream stream(encodedVector);
      auto request = CreateRequest(m_endpoint, HttpMethod::Post, {"Quotes/Generate"}, &stream);

      auto response = SendRequest(*m_pipeline, request, context);
      json jsonBody = json::parse(response->GetBody());
      auto returnValue(Azure::Core::_internal::Base64Url::Base64UrlDecode(
          jsonBody["quoteBase64UrlEncoded"].get<std::string>()));
      return Azure::Response<std::vector<uint8_t>>(returnValue, std::move(response));
    }

  private:
    /**
     * @brief Send a request to the service and process the response.
     *
     */
    static std::unique_ptr<Azure::Core::Http::RawResponse> SendRequest(
        Azure::Core::Http::_internal::HttpPipeline const& pipeline,
        Azure::Core::Http::Request& request,
        Azure::Core::Context const& context)
    {
      auto response = pipeline.Send(request, context);
      auto responseCode = response->GetStatusCode();

      switch (responseCode)
      {

        // 200, 201, 202, 204 are accepted responses
        case Azure::Core::Http::HttpStatusCode::Ok:
        case Azure::Core::Http::HttpStatusCode::Created:
        case Azure::Core::Http::HttpStatusCode::Accepted:
        case Azure::Core::Http::HttpStatusCode::NoContent:
          break;
        default:
          throw Azure::Core::RequestFailedException(response);
      }
      return response;
    }

    /**
     *
     * @brief Create a new request without an API version parameter.
     *
     * Used for the GetOpenIdMetadata and other APIs that do not take an API version
     * parameter.
     */
    static Azure::Core::Http::Request CreateRequest(
        Azure::Core::Url url,
        Azure::Core::Http::HttpMethod method,
        std::vector<std::string> const& path,
        Azure::Core::IO::BodyStream* content)
    {
      using namespace Azure::Core::Http;
      return CreateRequest(url, "", method, path, content);
    }

    static Azure::Core::Http::Request CreateRequest(
        Azure::Core::Url url,
        std::string const& apiVersion,
        Azure::Core::Http::HttpMethod method,
        std::vector<std::string> const& path,
        Azure::Core::IO::BodyStream* content)
    {
      using namespace Azure::Core::Http;
      Request request = content == nullptr ? Request(method, url) : Request(method, url, content);

      constexpr static const char ContentHeaderName[] = "content-type";
      constexpr static const char ApplicationJsonValue[] = "application/json";
      constexpr static const char ApiVersionQueryParamName[] = "api-version";

      request.SetHeader(ContentHeaderName, ApplicationJsonValue);
      if (!apiVersion.empty())
      {
        request.GetUrl().AppendQueryParameter(ApiVersionQueryParamName, apiVersion);
      }

      for (std::string const& P : path)
      {
        if (!P.empty())
        {
          request.GetUrl().AppendPath(P);
        }
      }
      return request;
    }

    Azure::Core::Url m_endpoint;
    std::string m_apiVersion;
    std::shared_ptr<Azure::Core::Credentials::TokenCredential const> m_credentials;
    std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> m_pipeline;
  };
}}}}} // namespace Azure::Security::KeyVault::Keys::Test
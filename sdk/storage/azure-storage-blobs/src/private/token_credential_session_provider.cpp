// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "token_credential_session_provider.hpp"

#include "package_version.hpp"

#include <azure/core/io/body_stream.hpp>
#include <azure/storage/common/internal/constants.hpp>
#include <azure/storage/common/internal/storage_bearer_token_auth.hpp>
#include <azure/storage/common/internal/storage_pipeline.hpp>
#include <azure/storage/common/internal/storage_url.hpp>
#include <azure/storage/common/internal/xml_wrapper.hpp>
#include <azure/storage/common/storage_credential.hpp>
#include <azure/storage/common/storage_exception.hpp>

#include <map>
#include <mutex>
#include <stdexcept>

namespace {
constexpr auto SessionRefreshLeadTime = std::chrono::seconds(30);
constexpr auto SessionRefreshSuppression = std::chrono::seconds(30);
constexpr auto SessionFailureCooldown = std::chrono::minutes(5);

} // namespace

namespace Azure { namespace Storage { namespace Blobs { namespace _detail {

  std::string TokenCredentialSessionProvider::GetContainerUrl(
      const Azure::Core::Http::Request& request) const
  {
    auto url = request.GetUrl();
    const auto& path = url.GetPath();
    const auto& servicePath = m_serviceUrl.GetPath();
    size_t containerStart = 0;
    if (!servicePath.empty() && path.compare(0, servicePath.size(), servicePath) == 0)
    {
      containerStart = servicePath.size();
    }
    const auto getFirstPathSegment = [](const std::string& value, size_t start) {
      while (start < value.size() && value[start] == '/')
      {
        ++start;
      }
      const auto end = value.find('/', start);
      return value.substr(start, end - start);
    };
    const auto container = getFirstPathSegment(path, containerStart);
    if (container.empty())
    {
      return {};
    }
    auto containerUrl = m_serviceUrl;
    containerUrl.AppendPath(container);
    return containerUrl.GetAbsoluteUrl();
  }

  std::shared_ptr<TokenCredentialSessionProvider::CacheEntry>
  TokenCredentialSessionProvider::GetCacheEntry(const std::string& containerUrl)
  {
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    auto& entry = m_cache[containerUrl];
    if (!entry)
    {
      entry = std::make_shared<CacheEntry>();
    }
    return entry;
  }

  SessionTokenInfo TokenCredentialSessionProvider::CreateSession(
      const std::string& containerUrl,
      const Azure::Core::Context& context)
  {
    _internal::XmlWriter writer;
    writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "CreateSessionRequest"});
    writer.Write(
        _internal::XmlNode{_internal::XmlNodeType::StartTag, "AuthenticationType", "HMAC"});
    writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
    writer.Write(_internal::XmlNode{_internal::XmlNodeType::End});
    auto xmlBody = writer.GetDocument();

    Azure::Core::IO::MemoryBodyStream body(
        reinterpret_cast<const uint8_t*>(xmlBody.data()), xmlBody.size());
    Azure::Core::Url url(containerUrl);
    url.AppendQueryParameter("restype", "container");
    url.AppendQueryParameter("comp", "session");
    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Post, url, &body);
    request.SetHeader(_internal::HttpHeaderContentType, "application/xml; charset=UTF-8");
    request.SetHeader(_internal::HttpHeaderContentLength, std::to_string(body.Length()));

    auto response = m_pipeline->Send(request, context);
    if (response->GetStatusCode() != Azure::Core::Http::HttpStatusCode::Created)
    {
      throw StorageException::CreateFromResponse(std::move(response));
    }

    SessionTokenInfo session;
    const auto& responseBody = response->GetBody();
    _internal::XmlReader reader(
        reinterpret_cast<const char*>(responseBody.data()), responseBody.size());
    std::string currentTag;
    bool expirationFound = false;
    while (true)
    {
      const auto node = reader.Read();
      if (node.Type == _internal::XmlNodeType::End)
      {
        break;
      }
      if (node.Type == _internal::XmlNodeType::StartTag)
      {
        currentTag = node.Name;
      }
      else if (node.Type == _internal::XmlNodeType::Text)
      {
        if (currentTag == "SessionToken")
        {
          session.Token = node.Value;
        }
        else if (currentTag == "SessionKey")
        {
          session.Key = node.Value;
        }
        else if (currentTag == "Expiration")
        {
          session.ExpiresOn = Azure::DateTime::Parse(
              node.Value,
              node.Value.find(',') != std::string::npos ? Azure::DateTime::DateFormat::Rfc1123
                                                        : Azure::DateTime::DateFormat::Rfc3339);
          expirationFound = true;
        }
      }
      else if (node.Type == _internal::XmlNodeType::EndTag)
      {
        currentTag.clear();
      }
    }
    if (session.Token.empty() || session.Key.empty() || !expirationFound)
    {
      throw std::runtime_error("Create Session response was missing required session fields.");
    }
    return session;
  }

  TokenCredentialSessionProvider::TokenCredentialSessionProvider(
      const std::string& serviceUrl,
      std::shared_ptr<const Azure::Core::Credentials::TokenCredential> credential,
      const BlobClientOptions& options)
  {
    if (!options.Session.AccountName.empty())
    {
      m_serviceUrl = Azure::Core::Url(serviceUrl);
    }
    else
    {
      auto urlParts = Azure::Storage::_internal::ParseStorageUrl(Azure::Core::Url(serviceUrl));
      if (!urlParts.HasValue())
      {
        throw std::invalid_argument(
            "SessionOptions.AccountName could not be determined from the provided service URL.");
      }
      m_serviceUrl = Azure::Core::Url(urlParts.Value().ServiceUrl);
    }
    _internal::BuildStoragePipelineOptions pipelineOptions;
    pipelineOptions.PackageName = _internal::BlobServicePackageName;
    pipelineOptions.PackageVersion = PackageVersion::ToString();
    pipelineOptions.ApiVersion = Blobs::_detail::ApiVersion;
    Azure::Core::Credentials::TokenRequestContext tokenContext;
    tokenContext.Scopes.emplace_back(
        options.Audience.HasValue()
            ? _internal::GetDefaultScopeForAudience(options.Audience.Value().ToString())
            : _internal::StorageScope);
    pipelineOptions.TokenAuthPolicy
        = std::make_unique<_internal::StorageBearerTokenAuthenticationPolicy>(
            std::move(credential), tokenContext, options.EnableTenantDiscovery);
    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        _internal::BuildHttpPipelinePolicies(options, std::move(pipelineOptions)));
  }

  bool TokenCredentialSessionProvider::IsRequestEligible(
      const Azure::Core::Http::Request& request) const
  {
    auto requestHost = request.GetUrl().GetHost();
    const auto headers = request.GetHeaders();
    const auto hostHeader = headers.find("Host");
    if (hostHeader != headers.end())
    {
      requestHost
          = Azure::Core::Url(request.GetUrl().GetScheme() + "://" + hostHeader->second).GetHost();
    }
    if (requestHost != m_serviceUrl.GetHost())
    {
      return false;
    }
    if (request.GetMethod() != Azure::Core::Http::HttpMethod::Get)
    {
      return false;
    }
    const auto query = request.GetUrl().GetQueryParameters();
    if (query.count("comp") != 0 || query.count("restype") != 0)
    {
      return false;
    }
    if (request.GetHeaders().count("x-ms-structured-body") != 0)
    {
      return false;
    }
    const auto containerUrl = GetContainerUrl(request);
    return !containerUrl.empty()
        && request.GetUrl().GetPath().size() > Azure::Core::Url(containerUrl).GetPath().size();
  }

  Azure::Nullable<SessionTokenInfo> TokenCredentialSessionProvider::GetSession(
      const Azure::Core::Http::Request& request,
      const Azure::Core::Context& context)
  {
    const auto containerUrl = GetContainerUrl(request);
    if (containerUrl.empty())
    {
      return {};
    }
    auto entry = GetCacheEntry(containerUrl);
    auto now = Azure::DateTime::clock::now();
    {
      std::lock_guard<std::mutex> lock(entry->Mutex);
      if (entry->Current.HasValue()
          && now < entry->Current.Value().ExpiresOn - SessionRefreshLeadTime)
      {
        return entry->Current;
      }
      if (entry->CooldownUntil > now)
      {
        if (entry->Current.HasValue() && now < entry->Current.Value().ExpiresOn)
        {
          return entry->Current;
        }
        return {};
      }
      if (entry->LastRefreshStarted + SessionRefreshSuppression > now)
      {
        if (entry->Current.HasValue() && now < entry->Current.Value().ExpiresOn)
        {
          return entry->Current;
        }
        return {};
      }
      entry->LastRefreshStarted = now;
    }

    try
    {
      auto session = CreateSession(containerUrl, context);
      std::lock_guard<std::mutex> lock(entry->Mutex);
      if (!entry->Current.HasValue() || entry->Current.Value().ExpiresOn < session.ExpiresOn)
      {
        entry->Current = session;
      }
      return session;
    }
    catch (const StorageException& e)
    {
      const auto statusCode = static_cast<int>(e.StatusCode);
      const bool softFailure = statusCode >= 500 || statusCode == 403
          || (statusCode == 400 && e.ErrorCode == "FeatureNotEnabled");
      now = Azure::DateTime::clock::now();
      std::lock_guard<std::mutex> lock(entry->Mutex);
      if (softFailure)
      {
        const auto cooldownUntil = now + SessionFailureCooldown;
        if (entry->CooldownUntil < cooldownUntil)
        {
          entry->CooldownUntil = cooldownUntil;
        }
      }
      if (entry->Current.HasValue() && now < entry->Current.Value().ExpiresOn)
      {
        return entry->Current;
      }
      if (!softFailure)
      {
        throw;
      }
      return {};
    }
    catch (const std::exception&)
    {
      now = Azure::DateTime::clock::now();
      std::lock_guard<std::mutex> lock(entry->Mutex);
      if (entry->Current.HasValue() && now < entry->Current.Value().ExpiresOn)
      {
        return entry->Current;
      }
      throw;
    }
  }

  void TokenCredentialSessionProvider::InvalidateSession(
      const Azure::Core::Http::Request& request,
      const SessionTokenInfo& session)
  {
    const auto containerUrl = GetContainerUrl(request);
    if (containerUrl.empty())
    {
      return;
    }
    auto entry = GetCacheEntry(containerUrl);
    std::lock_guard<std::mutex> lock(entry->Mutex);
    if (entry->Current.HasValue() && entry->Current.Value().Token == session.Token)
    {
      entry->Current.Reset();
    }
  }

}}}} // namespace Azure::Storage::Blobs::_detail

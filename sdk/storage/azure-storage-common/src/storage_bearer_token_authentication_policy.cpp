// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/common/internal/constants.hpp"
#include "azure/storage/common/internal/shared_key_policy.hpp"
#include "azure/storage/common/internal/storage_bearer_token_auth.hpp"
#include "azure/storage/common/internal/storage_url.hpp"
#include "azure/storage/common/internal/xml_wrapper.hpp"
#include "azure/storage/common/storage_exception.hpp"

#include <azure/core/datetime.hpp>
#include <azure/core/internal/credentials/authorization_challenge_parser.hpp>

#include <mutex>

namespace {
struct SessionToken
{
  std::string Id;
  std::string Token;
  std::string Key;
  Azure::DateTime ExpiresOn;
};

constexpr static const int g_sessionTokenRefreshIntervalInSeconds = 3;
constexpr static const int g_rereshLeadTimeInSeconds = 30;

struct SessionTokenContainerContext
{
  Azure::DateTime LastRefreshedOn;
  Azure::DateTime CoolDownUntil;

  Azure::Nullable<SessionToken> Token;

  std::mutex Mutex;

  void SetTokenUnlocked(SessionToken token)
  {
    if (!Token.HasValue() || Token.Value().ExpiresOn < token.ExpiresOn)
    {
      Token = std::move(token);
    }
  }

  void InvalidateTokenUnlocked() { Token.Reset(); }

  bool IsTokenValidUnlocked()
  {
    if (!Token.HasValue())
    {
      return false;
    }
    if (Token.Value().ExpiresOn > Azure::DateTime::clock::now())
    {
      return true;
    }
    else
    {
      InvalidateTokenUnlocked();
      return false;
    }
  }
};

class SessionTokenContextMap {
public:
  SessionTokenContainerContext& GetContextFor(const std::string& key)
  {
    std::lock_guard<std::mutex> lock(m_lock);
    return m_contexts[key];
  }

private:
  std::mutex m_lock;
  std::map<std::string, SessionTokenContainerContext> m_contexts;
};

SessionTokenContextMap& GetSessionTokenGlobalContext()
{
  static SessionTokenContextMap m;
  return m;
}

} // namespace

namespace Azure { namespace Storage { namespace _internal {

  std::unique_ptr<Azure::Core::Http::RawResponse>
  StorageBearerTokenAuthenticationPolicy::AuthorizeAndSendRequest(
      Azure::Core::Http::Request& request,
      Azure::Core::Http::Policies::NextHttpPolicy& nextPolicy,
      Azure::Core::Context const& context) const
  {
    const auto getSessionToken
        = [&nextPolicy, &context, &request](const std::string& containerUrl) -> SessionToken {
      std::string xmlBody;
      {
        _internal::XmlWriter writer;
        writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "CreateSessionRequest"});
        writer.Write(
            _internal::XmlNode{_internal::XmlNodeType::StartTag, "AuthenticationType", "HMAC"});
        writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        writer.Write(_internal::XmlNode{_internal::XmlNodeType::End});
        xmlBody = writer.GetDocument();
      }
      Core::IO::MemoryBodyStream requestBody(
          reinterpret_cast<const uint8_t*>(xmlBody.data()), xmlBody.length());
      auto sessionRequest = Core::Http::Request(
          Core::Http::HttpMethod::Post, Core::Url(containerUrl), &requestBody);
      sessionRequest.SetHeader(HttpHeaderContentType, "application/xml; charset=UTF-8");
      sessionRequest.SetHeader(HttpHeaderContentLength, std::to_string(requestBody.Length()));
      const auto authorizationHeader = request.GetHeader(HttpHeaderAuthorization);
      if (authorizationHeader.HasValue())
      {
        sessionRequest.SetHeader(HttpHeaderAuthorization, authorizationHeader.Value());
      }
      const auto versionHeader = request.GetHeader(HttpHeaderXMsVersion);
      if (versionHeader.HasValue())
      {
        sessionRequest.SetHeader(HttpHeaderXMsVersion, versionHeader.Value());
      }
      sessionRequest.GetUrl().AppendQueryParameter("restype", "container");
      sessionRequest.GetUrl().AppendQueryParameter("comp", "session");
      auto pRawResponse = nextPolicy.Send(sessionRequest, context);
      auto httpStatusCode = pRawResponse->GetStatusCode();
      if (httpStatusCode != Core::Http::HttpStatusCode::Created)
      {
        throw StorageException::CreateFromResponse(std::move(pRawResponse));
      }
      const auto& responseBody = pRawResponse->GetBody();
      SessionToken token;
      _internal::XmlReader reader(
          reinterpret_cast<const char*>(responseBody.data()), responseBody.size());
      std::string currentTag;
      while (true)
      {
        auto node = reader.Read();
        if (node.Type == _internal::XmlNodeType::End)
        {
          break;
        }
        else if (node.Type == _internal::XmlNodeType::StartTag)
        {
          currentTag = node.Name;
        }
        else if (node.Type == _internal::XmlNodeType::Text)
        {
          if (currentTag == "Id")
          {
            token.Id = node.Value;
          }
          else if (currentTag == "SessionToken")
          {
            token.Token = node.Value;
          }
          else if (currentTag == "SessionKey")
          {
            token.Key = node.Value;
          }
          else if (currentTag == "Expiration")
          {
            token.ExpiresOn
                = Azure::DateTime::Parse(node.Value, Azure::DateTime::DateFormat::Rfc1123);
          }
        }
        else if (node.Type == _internal::XmlNodeType::EndTag)
        {
          currentTag.clear();
        }
      }
      return token;
    };

    auto setupBearerToken = [&]() {
      const std::string tenantId = m_safeTenantId.Get();
      if (!tenantId.empty() || !m_enableTenantDiscovery)
      {
        Azure::Core::Credentials::TokenRequestContext tokenRequestContext;
        tokenRequestContext.Scopes = m_scopes;
        tokenRequestContext.TenantId = tenantId;
        AuthenticateAndAuthorizeRequest(request, tokenRequestContext, context);
      }
    };

    if (m_sessionOptions.Enabled)
    {
      const auto urlParts = ParseStorageUrl(request.GetUrl());
      const auto accountName = [this, &urlParts]() {
        if (!m_sessionOptions.AccountName.empty())
        {
          return m_sessionOptions.AccountName;
        }
        else if (urlParts.HasValue())
        {
          return urlParts.Value().AccountName;
        }
        return std::string();
      }();
      const auto queryParameters = request.GetUrl().GetQueryParameters();
      const bool isGetObject = request.GetMethod() == Azure::Core::Http::HttpMethod::Get
          && queryParameters.count("restype") == 0 && queryParameters.count("comp") == 0;

      if (urlParts.HasValue() && urlParts.Value().Service == "blob"
          && urlParts.Value().ContainerName.HasValue() && urlParts.Value().ContainerUrl.HasValue()
          && isGetObject && !accountName.empty())
      {
        const auto lookupKey = urlParts.Value().Service + "/" + accountName + "/"
            + urlParts.Value().ContainerName.Value();
        auto& sessionContext = GetSessionTokenGlobalContext().GetContextFor(lookupKey);
        Azure::Nullable<SessionToken> sessionTokenToUse;
        bool shouldRefreshToken = false;
        {
          std::lock_guard<std::mutex> lock(sessionContext.Mutex);
          if (sessionContext.IsTokenValidUnlocked())
          {
            sessionTokenToUse = sessionContext.Token;
            if (Azure::DateTime::clock::now() + std::chrono::seconds(g_rereshLeadTimeInSeconds)
                > sessionContext.Token.Value().ExpiresOn)
            {
              shouldRefreshToken = true;
            }
          }
          else
          {
            shouldRefreshToken = true;
          }

          if (sessionContext.LastRefreshedOn
                  + std::chrono::seconds(g_sessionTokenRefreshIntervalInSeconds)
              > Azure::DateTime::clock::now())
          {
            shouldRefreshToken = false;
          }
          else if (sessionContext.CoolDownUntil > Azure::DateTime::clock::now())
          {
            shouldRefreshToken = false;
          }
          if (shouldRefreshToken)
          {
            setupBearerToken();
            if (!request.GetHeader(HttpHeaderAuthorization).HasValue())
            {
              shouldRefreshToken = false;
            }
          }
          if (shouldRefreshToken)
          {
            sessionContext.LastRefreshedOn = Azure::DateTime::clock::now();
          }
        }
        if (shouldRefreshToken)
        {
          try
          {
            SessionToken newToken = getSessionToken(urlParts.Value().ContainerUrl.Value());
            {
              std::lock_guard<std::mutex> lock(sessionContext.Mutex);
              sessionContext.SetTokenUnlocked(newToken);
            }
            sessionTokenToUse = std::move(newToken);
          }
          catch (StorageException& e)
          {
            bool shouldResetToken = false;
            bool shouldFail = false;
            int coolDownTimeInSeconds = 0;

            const int statusCode = static_cast<int>(e.StatusCode);
            if (statusCode >= 500 && statusCode <= 599)
            {
              coolDownTimeInSeconds = 60;
            }
            else if (
                e.ErrorCode == "UnsupportedHttpVerb"
                || e.ErrorCode == "AuthorizationPermissionMismatch"
                || e.ErrorCode == "AuthorizationFailure" || e.ErrorCode == "FeatureNotEnabled")
            {
              shouldResetToken = true;
              coolDownTimeInSeconds = 24 * 3600;
            }
            else if (
                e.ErrorCode == "InvalidAuthenticationInfo"
                || e.ErrorCode == "InvalidQueryParameterValue"
                || e.ErrorCode == "InvalidHeaderValue" || e.ErrorCode == "InvalidXmlDocument"
                || e.ErrorCode == "InvalidXmlNodeValue" || e.ErrorCode == "MissingRequiredHeader"
                || e.ErrorCode == "MissingRequiredQueryParameter")
            {
              shouldFail = true;
              shouldResetToken = true;
              coolDownTimeInSeconds = 24 * 3600;
            }
            else
            {
              shouldFail = true;
              shouldResetToken = true;
              coolDownTimeInSeconds = 24 * 3600;
            }

            {
              std::lock_guard<std::mutex> lock(sessionContext.Mutex);
              if (shouldResetToken)
              {
                sessionContext.InvalidateTokenUnlocked();
              }
              sessionContext.CoolDownUntil
                  = Azure::DateTime::clock::now() + std::chrono::seconds(coolDownTimeInSeconds);
            }
            if (shouldFail)
            {
              throw;
            }
          }
        } // shouldRefreshToken

        if (sessionTokenToUse.HasValue())
        {
          request.RemoveHeader(HttpHeaderAuthorization);
          const auto signature
              = SharedKeyPolicy::GetSignature(request, accountName, sessionTokenToUse.Value().Key);
          request.SetHeader(
              HttpHeaderAuthorization,
              "Session " + sessionTokenToUse.Value().Token + ":" + signature);

          auto pRawResponse = nextPolicy.Send(request, context);
          auto httpStatusCode = static_cast<int>(pRawResponse->GetStatusCode());
          if (httpStatusCode >= 400)
          {
            bool shouldResetToken = false;
            bool shouldFail = true;
            int coolDownTimeInSeconds = 0;

            const auto& headers = pRawResponse->GetHeaders();
            auto ite = headers.find("WWW-Authenticate");
            if (ite != headers.end())
            {
              const auto& headerValue = ite->second;
              auto pos1 = headerValue.find("Session error=");
              if (pos1 != std::string::npos)
              {
                auto pos2 = headerValue.find_first_of(" ,", pos1 + 14);
                const auto sessionError = headerValue.substr(pos1 + 14, pos2 - pos1 - 14);
                if (sessionError == "session_expired" || sessionError == "session_token_invalid")
                {
                  shouldResetToken = true;
                  shouldFail = false;
                }
              }
            }
            ite = headers.find("x-ms-error-code");
            if (ite != headers.end())
            {
              const auto& headerValue = ite->second;
              if (headerValue == "SessionOperationsTemporarilyUnavailable")
              {
                shouldFail = false;
                shouldResetToken = true;
                coolDownTimeInSeconds = 60;
              }
            }

            {
              std::lock_guard<std::mutex> lock(sessionContext.Mutex);
              if (shouldResetToken)
              {
                sessionContext.InvalidateTokenUnlocked();
              }
              sessionContext.CoolDownUntil
                  = Azure::DateTime::clock::now() + std::chrono::seconds(coolDownTimeInSeconds);
            }
            if (shouldFail)
            {
              return pRawResponse;
            }
            // otherwise, fallback
          }
          else
          {
            return pRawResponse;
          }
        }
      }
    }

    // fallback to bearer token
    setupBearerToken();
    return nextPolicy.Send(request, context);
  }

  bool StorageBearerTokenAuthenticationPolicy::AuthorizeRequestOnChallenge(
      std::string const& challenge,
      Azure::Core::Http ::Request& request,
      Azure::Core::Context const& context) const
  {
    if (!m_enableTenantDiscovery)
    {
      return false;
    }
    const std::string authorizationUri
        = Azure::Core::Credentials::_internal::AuthorizationChallengeParser::GetChallengeParameter(
            challenge, "Bearer", "authorization_uri");

    if (authorizationUri.empty())
    {
      return false;
    }

    // tenantId should be the guid as seen in this example:
    // https://login.microsoftonline.com/72f988bf-86f1-41af-91ab-2d7cd011db47/oauth2/authorize
    std::string path = Azure::Core::Url(authorizationUri).GetPath();
    std::string tenantId = path.substr(0, path.find('/'));
    m_safeTenantId.Set(tenantId);

    Azure::Core::Credentials::TokenRequestContext tokenRequestContext;
    tokenRequestContext.Scopes = m_scopes;
    tokenRequestContext.TenantId = tenantId;
    AuthenticateAndAuthorizeRequest(request, tokenRequestContext, context);
    return true;
  }

}}} // namespace Azure::Storage::_internal

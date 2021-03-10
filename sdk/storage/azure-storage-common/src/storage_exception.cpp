// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/common/storage_exception.hpp"

#include <type_traits>

#include <azure/core/http/policy.hpp>
#include <azure/core/internal/json.hpp>

#include "azure/storage/common/constants.hpp"
#include "azure/storage/common/xml_wrapper.hpp"

namespace Azure { namespace Storage {
  StorageException StorageException::CreateFromResponse(
      std::unique_ptr<Azure::Core::Http::RawResponse> response)
  {
    std::vector<uint8_t> bodyBuffer = std::move(response->GetBody());

    auto httpStatusCode = response->GetStatusCode();
    std::string reasonPhrase = response->GetReasonPhrase();
    std::string requestId;
    if (response->GetHeaders().find(_detail::HttpHeaderRequestId) != response->GetHeaders().end())
    {
      requestId = response->GetHeaders().at(_detail::HttpHeaderRequestId);
    }

    std::string clientRequestId;
    if (response->GetHeaders().find(_detail::HttpHeaderClientRequestId)
        != response->GetHeaders().end())
    {
      clientRequestId = response->GetHeaders().at(_detail::HttpHeaderClientRequestId);
    }

    std::string errorCode;
    std::string message;
    std::map<std::string, std::string> additionalInformation;

    if (response->GetHeaders().find(_detail::HttpHeaderContentType) != response->GetHeaders().end())
    {
      if (response->GetHeaders().at(_detail::HttpHeaderContentType).find("xml")
          != std::string::npos)
      {
        auto xmlReader = _detail::XmlReader(
            reinterpret_cast<const char*>(bodyBuffer.data()), bodyBuffer.size());

        enum class XmlTagName
        {
          XmlTagError,
          XmlTagCode,
          XmlTagMessage,
          XmlTagUnknown,
        };
        std::vector<XmlTagName> path;
        std::string startTagName;

        while (true)
        {
          auto node = xmlReader.Read();
          if (node.Type == _detail::XmlNodeType::End)
          {
            break;
          }
          else if (node.Type == _detail::XmlNodeType::EndTag)
          {
            startTagName.clear();
            if (path.size() > 0)
            {
              path.pop_back();
            }
            else
            {
              break;
            }
          }
          else if (node.Type == _detail::XmlNodeType::StartTag)
          {
            startTagName = node.Name;
            if (std::strcmp(node.Name, "Error") == 0)
            {
              path.emplace_back(XmlTagName::XmlTagError);
            }
            else if (std::strcmp(node.Name, "Code") == 0)
            {
              path.emplace_back(XmlTagName::XmlTagCode);
            }
            else if (std::strcmp(node.Name, "Message") == 0)
            {
              path.emplace_back(XmlTagName::XmlTagMessage);
            }
            else
            {
              path.emplace_back(XmlTagName::XmlTagUnknown);
            }
          }
          else if (node.Type == _detail::XmlNodeType::Text)
          {
            if (path.size() == 2 && path[0] == XmlTagName::XmlTagError
                && path[1] == XmlTagName::XmlTagCode)
            {
              errorCode = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::XmlTagError
                && path[1] == XmlTagName::XmlTagMessage)
            {
              message = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::XmlTagError
                && path[1] == XmlTagName::XmlTagUnknown)
            {
              if (!startTagName.empty())
              {
                additionalInformation.emplace(std::move(startTagName), node.Value);
              }
            }
          }
        }
      }
      else if (
          response->GetHeaders().at(_detail::HttpHeaderContentType).find("html")
          != std::string::npos)
      {
        // TODO: add a refined message parsed from result.
        message = std::string(bodyBuffer.begin(), bodyBuffer.end());
      }
      else if (
          response->GetHeaders().at(_detail::HttpHeaderContentType).find("json")
          != std::string::npos)
      {
        auto jsonParser = Azure::Core::_internal::Json::json::parse(bodyBuffer);
        errorCode = jsonParser["error"]["code"].get<std::string>();
        message = jsonParser["error"]["message"].get<std::string>();
      }
      else
      {
        // TODO: add a refined message parsed from result.
        message = std::string(bodyBuffer.begin(), bodyBuffer.end());
      }
    }

    StorageException result = StorageException(
        std::to_string(static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
            httpStatusCode))
        + " " + reasonPhrase + "\n" + message + "\nRequest ID: " + requestId);
    result.StatusCode = httpStatusCode;
    result.ReasonPhrase = std::move(reasonPhrase);
    result.RequestId = std::move(requestId);
    result.ClientRequestId = std::move(clientRequestId);
    result.ErrorCode = std::move(errorCode);
    result.Message = std::move(message);
    result.RawResponse = std::move(response);
    result.AdditionalInformation = std::move(additionalInformation);
    return result;
  }
}} // namespace Azure::Storage

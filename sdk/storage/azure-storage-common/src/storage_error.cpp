// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/common/storage_error.hpp"

#include "azure/storage/common/constants.hpp"
#include "azure/storage/common/storage_common.hpp"
#include "azure/storage/common/xml_wrapper.hpp"
#include "azure/core/http/policy.hpp"
#include "azure/storage/common/json.hpp"

#include <type_traits>

namespace Azure { namespace Storage {
  StorageError StorageError::CreateFromResponse(
      std::unique_ptr<Azure::Core::Http::RawResponse> response)
  {
    std::vector<uint8_t> bodyBuffer = std::move(response->GetBody());

    auto httpStatusCode = response->GetStatusCode();
    std::string reasonPhrase = response->GetReasonPhrase();
    std::string requestId;
    if (response->GetHeaders().find("x-ms-request-id") != response->GetHeaders().end())
    {
      requestId = response->GetHeaders().at("x-ms-request-id");
    }

    std::string clientRequestId;
    if (response->GetHeaders().find("x-ms-client-request-id") != response->GetHeaders().end())
    {
      clientRequestId = response->GetHeaders().at("x-ms-client-request-id");
    }

    std::string errorCode;
    std::string message;

    if (response->GetHeaders().find(Details::c_HttpHeaderContentType)
        != response->GetHeaders().end())
    {
      if (response->GetHeaders().at(Details::c_HttpHeaderContentType).find("xml")
          != std::string::npos)
      {
        auto xmlReader
            = XmlReader(reinterpret_cast<const char*>(bodyBuffer.data()), bodyBuffer.size());

        enum class XmlTagName
        {
          c_Error,
          c_Code,
          c_Message,
        };
        std::vector<XmlTagName> path;

        while (true)
        {
          auto node = xmlReader.Read();
          if (node.Type == XmlNodeType::End)
          {
            break;
          }
          else if (node.Type == XmlNodeType::EndTag)
          {
            if (path.size() > 0)
            {
              path.pop_back();
            }
            else
            {
              break;
            }
          }
          else if (node.Type == XmlNodeType::StartTag)
          {
            if (std::strcmp(node.Name, "Error") == 0)
            {
              path.emplace_back(XmlTagName::c_Error);
            }
            else if (std::strcmp(node.Name, "Code") == 0)
            {
              path.emplace_back(XmlTagName::c_Code);
            }
            else if (std::strcmp(node.Name, "Message") == 0)
            {
              path.emplace_back(XmlTagName::c_Message);
            }
          }
          else if (node.Type == XmlNodeType::Text)
          {
            if (path.size() == 2 && path[0] == XmlTagName::c_Error && path[1] == XmlTagName::c_Code)
            {
              errorCode = node.Value;
            }
            else if (
                path.size() == 2 && path[0] == XmlTagName::c_Error
                && path[1] == XmlTagName::c_Message)
            {
              message = node.Value;
            }
          }
        }
      }
      else if (
          response->GetHeaders().at(Details::c_HttpHeaderContentType).find("html")
          != std::string::npos)
      {
        // TODO: add a refined message parsed from result.
        message = std::string(bodyBuffer.begin(), bodyBuffer.end());
      }
      else if (
          response->GetHeaders().at(Details::c_HttpHeaderContentType).find("json")
          != std::string::npos)
      {
        auto jsonParser = nlohmann::json::parse(bodyBuffer);
        errorCode = jsonParser["error"]["code"].get<std::string>();
        message = jsonParser["error"]["message"].get<std::string>();
      }
      else
      {
        // TODO: add a refined message parsed from result.
        message = std::string(bodyBuffer.begin(), bodyBuffer.end());
      }
    }

    StorageError result = StorageError(
        std::to_string(static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
            httpStatusCode))
        + " " + reasonPhrase + "\n" + message + "\nRequest ID: " + requestId);
    result.StatusCode = httpStatusCode;
    result.ReasonPhrase = std::move(reasonPhrase);
    result.RequestId = std::move(requestId);
    result.ErrorCode = std::move(errorCode);
    result.Message = std::move(message);
    result.RawResponse = std::move(response);
    return result;
  }
}} // namespace Azure::Storage

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "common/storage_error.hpp"

#include "common/xml_wrapper.hpp"
#include "http/http.hpp"

namespace Azure { namespace Storage {
  StorageError StorageError::CreateFromResponse(/* const */ Azure::Core::Http::Response& response)
  {
    auto bodyBuffer
        = Azure::Core::Http::Response::ConstructBodyBufferFromStream(response.GetBodyStream());

    auto xmlReader
        = XmlReader(reinterpret_cast<const char*>(bodyBuffer->data()), bodyBuffer->size());

    enum class XmlTagName
    {
      c_Error,
      c_Code,
      c_Message,
      c_Details,
    };
    std::vector<XmlTagName> path;
    std::string code;
    std::string message;
    std::map<std::string, std::string> details;

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
        else
        {
          path.emplace_back(XmlTagName::c_Details);
        }
      }
      else if (node.Type == XmlNodeType::Text)
      {
        if (path.size() == 2 && path[0] == XmlTagName::c_Error && path[1] == XmlTagName::c_Code)
        {
          code = node.Value;
        }
        else if (
            path.size() == 2 && path[0] == XmlTagName::c_Error && path[1] == XmlTagName::c_Message)
        {
          message = node.Value;
        }
        else if (
            path.size() == 2 && path[0] == XmlTagName::c_Error && path[1] == XmlTagName::c_Details)
        {
          details[node.Name] = node.Value;
        }
      }
    }
    StorageError result = StorageError(message, code);

    if (response.GetHeaders().find("x-ms-request-id") != response.GetHeaders().end())
    {
      result.RequestId = response.GetHeaders().at("x-ms-request-id");
    }
    result.Details = std::move(details);

    return result;
  }
}} // namespace Azure::Storage

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

// TODO: remove this
#pragma warning(disable : 4239)

#include "http/http.hpp"
#include "common/xml_wrapper.hpp"

#include <map>
#include <string>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <ctime>

namespace Azure
{
namespace Storage
{
namespace Blobs
{
class BlobRestClient
{
  const static std::string k_HEADER_METADATA_PREFIX;
  const static std::string k_HEADER_DATE;
  const static std::string k_HEADER_X_MS_VERSION;
  const static std::string k_HEADER_AUTHORIZATION;
  const static std::string k_HEADER_CLIENT_REQUEST_ID;
  const static std::string k_HEADER_ETAG;
  const static std::string k_HEADER_LAST_MODIFIED;
  const static std::string k_HEADER_X_MS_CLIENT_REQUEST_ID;
  const static std::string k_HEADER_X_MS_REQUEST_ID;
  const static std::string k_HEADER_CONTENT_MD5;
  const static std::string k_HEADER_X_MS_CONTENT_CRC64;
  const static std::string k_RESTYPE;
  const static std::string k_CONTAINER;
  const static std::string k_QUERY_COMP;
  const static std::string k_QUERY_BLOCK_LIST;
  const static std::string k_QUERY_BLOCK;
  const static std::string k_BLOB;
  const static std::string k_BLOCK_ID;

public:
  struct RequestOptions
  {
    std::string Version = "2019-02-02";
    std::string ClientRequestID = std::string();
    std::string Date = std::string();
  };

  struct BodiedRequestOptions : public RequestOptions
  {
    azure::core::http::BodyBuffer* BodyBuffer = nullptr;
    azure::core::http::BodyStream* BodyStream = nullptr;
  };

  struct ResponseInfo
  {
    std::string RequestId = std::string();
    std::string Date = std::string();
    std::string Version = std::string();
    std::string ClientRequestID = std::string();
  };

  class Container
  {
    const static std::string k_HEADER_MS_BLOB_PUBLIC_ACCESS;

  public:
    enum class PublicAccessType
    {
      Container = 0,
      Blob,
      Anonymous
    };

    struct CreateOptions : public RequestOptions
    {
      PublicAccessType AccessType = PublicAccessType::Anonymous;
      std::map<std::string, std::string> Metadata = std::map<std::string, std::string>();
    };

    struct BlobContainerInfo : public ResponseInfo
    {
      std::string ContainerUrl = std::string();
      std::string Etag = std::string();
      std::string LastModified = std::string();
    };

    static azure::core::http::Request CreateConstructRequest(
        /*const*/ std::string& url,
        /*const*/ CreateOptions& options)
    {
      auto request = azure::core::http::Request(azure::core::http::HttpMethod::Put, url);
      if (options.AccessType == PublicAccessType::Blob)
      {
        request.addHeader(k_HEADER_MS_BLOB_PUBLIC_ACCESS, k_BLOB);
      }
      else if (options.AccessType == PublicAccessType::Container)
      {
        request.addHeader(k_HEADER_MS_BLOB_PUBLIC_ACCESS, k_CONTAINER);
      }

      ApplyBasicHeaders(options, request);

      AddMetadata(options.Metadata, request);

      request.addQueryParameter(k_RESTYPE, k_CONTAINER);

      return request;
    };

    // TODO: should return azure::core::http::Response<BlobContainerInfo> instead
    static BlobContainerInfo CreateParseResponse(/*const*/ azure::core::http::Response& response)
    {
      BlobContainerInfo info;
      if (/*TODO: when response errored*/ response.getStatusCode() >= 300U)
      {
      }
      else
      {
        // TODO: ContainerUrl initialization.
        info.Etag = GetHeaderValue(response.getHeaders(), k_HEADER_ETAG);
        info.LastModified = GetHeaderValue(response.getHeaders(), k_HEADER_LAST_MODIFIED);
        ParseBasicResponseHeaders(response.getHeaders(), info);
      }

      return info;
    }

    // TODO: should return azure::core::http::Response<BlobContainerInfo> instead
    static BlobContainerInfo Create(
        // TODO: Context and Pipeline should go here
        /*const*/ std::string& url,
        /*const*/ CreateOptions& options)
    {
      return CreateParseResponse(
          azure::core::http::Client::send(CreateConstructRequest(url, options)));
    };
  };

  class BlockBlob
  {
    const static std::string k_XML_TAG_BLOCK_LIST;
    const static std::string k_XML_TAG_COMMITTED;
    const static std::string k_XML_TAG_UNCOMMITTED;
    const static std::string k_XML_TAG_LATEST;
  public:
    enum class BlockType
    {
      Committed = 0,
      Uncommitted,
      Latest
    };

    struct BlockInfo : public ResponseInfo
    {
      bool ServerEncrypted = true;
      std::string ContentMd5 = std::string();
      std::string ContentCrc64 = std::string();
      std::string EncryptionKeySha256 = std::string();
    };

    struct StageBlockOptions : public BodiedRequestOptions
    {
      std::string BlockId;
      std::string ContentMd5 = std::string();
      std::string ContentCrc64 = std::string();
      std::string LeaseId = std::string();
      std::string EncryptionKey = std::string();
      std::string EncryptionKeySha256 = std::string();
      std::string EncryptionScope = std::string();
    };

    static azure::core::http::Request StageBlockConstructRequest(
        /*const*/ std::string& url,
        /*const*/ StageBlockOptions& options)
    {
      azure::core::http::Request request(azure::core::http::HttpMethod::Put, url);
      request.addQueryParameter(k_BLOCK_ID, options.BlockId);
      request.addQueryParameter(k_QUERY_COMP, k_QUERY_BLOCK);
      request.setBodyBuffer(options.BodyBuffer);
      request.setBodyStream(options.BodyStream);
      ApplyBasicHeaders(options, request);
      return request;
    }

    // TODO: should return azure::core::http::Response<BlockInfo> instead
    static BlockInfo StageBlockParseResponse(/*const*/ azure::core::http::Response& response)
    {
      BlockInfo info;
      info.ContentMd5 = GetHeaderValue(response.getHeaders(), k_HEADER_CONTENT_MD5);
      info.ContentCrc64 = GetHeaderValue(response.getHeaders(), k_HEADER_X_MS_CONTENT_CRC64);
      ParseBasicResponseHeaders(response.getHeaders(), info);
      return info;
    }

    // TODO: should return azure::core::http::Response<BlockInfo> instead
    static BlockInfo StageBlock(
        // TODO: Context and Pipeline should go here
        /*const*/ std::string& url,
        /*const*/ StageBlockOptions& options)
    {
      return StageBlockParseResponse(
          azure::core::http::Client::send(StageBlockConstructRequest(url, options)));
    }

    struct StageBlockListOptions : public RequestOptions
    {
      std::vector<std::pair<BlockType, std::string>> BlockList
          = std::vector<std::pair<BlockType, std::string>>();
      std::string ContentMd5 = std::string();
      std::string ContentCrc64 = std::string();
      std::string LeaseId = std::string();
      std::string EncryptionKey = std::string();
      std::string EncryptionKeySha256 = std::string();
      std::string EncryptionScope = std::string();
    };

    static azure::core::http::Request StageBlockListConstructRequest(
        /*const*/ std::string& url,
        /*const*/ StageBlockListOptions& options)
    {
      azure::core::http::Request request(azure::core::http::HttpMethod::Put, url);
      request.addQueryParameter(k_QUERY_COMP, k_QUERY_BLOCK_LIST);
      ApplyBasicHeaders(options, request);
      //TODO: avoid the data copy here.
      block_list_writer writer;
      std::string body = writer.write(options.BlockList);
      //TODO: Investigate why not copying the ending \0 would result in request body sending more data than expected.
      uint8_t* buffer = new uint8_t[body.size() + 1];
      memcpy_s(buffer, body.size() + 1, body.c_str(), body.size() + 1);
      azure::core::http::BodyBuffer* bodyBuffer = new azure::core::http::BodyBuffer(buffer, body.size());
      request.setBodyBuffer(bodyBuffer);
      return request;
    }



    // TODO: should return azure::core::http::Response<BlockInfo> instead
    static BlockInfo StageBlockListParseResponse(/*const*/ azure::core::http::Response& response)
    {
      BlockInfo info;
      info.ContentMd5 = GetHeaderValue(response.getHeaders(), k_HEADER_CONTENT_MD5);
      info.ContentCrc64 = GetHeaderValue(response.getHeaders(), k_HEADER_X_MS_CONTENT_CRC64);
      ParseBasicResponseHeaders(response.getHeaders(), info);
      return info;
    }

    // TODO: should return azure::core::http::Response<BlockInfo> instead
    static BlockInfo StageBlockList(
        // TODO: Context and Pipeline should go here
        /*const*/ std::string& url,
        /*const*/ StageBlockListOptions& options)
    {
        //TODO Manage the memory that is allocated when constructing the request.
      return StageBlockListParseResponse(
          azure::core::http::Client::send(StageBlockListConstructRequest(url, options)));
    }

  private:
    class block_list_writer : public Azure::Storage::Common::XML::xml_writer
    {
    public:
      block_list_writer() {}

      std::string write(const std::vector<std::pair<BlockType, std::string>>& blockList)
      {
        std::ostringstream outstream;
        initialize(outstream);

        write_start_element(k_XML_TAG_BLOCK_LIST);

        for (auto block = blockList.cbegin(); block != blockList.cend(); ++block)
        {
          std::string tag;
          switch (block->first)
          {
            case BlockType::Committed:
              tag = k_XML_TAG_COMMITTED;
              break;

            case BlockType::Uncommitted:
              tag = k_XML_TAG_UNCOMMITTED;
              break;

            case BlockType::Latest:
              tag = k_XML_TAG_LATEST;
              break;
          }

          write_element(tag, block->second);
        }

        finalize();
        return outstream.str();
      }
    };
  };

private:
  static void AddMetadata(
      /*const*/ std::map<std::string, std::string>& metadata,
      azure::core::http::Request& request)
  {
    for (auto pair : metadata)
    {
      request.addHeader(k_HEADER_METADATA_PREFIX + pair.first, pair.second);
    }
  }

  static void ApplyBasicHeaders(
      /*const*/ RequestOptions& options,
      azure::core::http::Request& request)
  {
    if (options.Date.empty())
    {
      options.Date = GetDateString();
    }
    request.addHeader(k_HEADER_X_MS_VERSION, options.Version);
    request.addHeader(k_HEADER_CLIENT_REQUEST_ID, options.ClientRequestID);
    request.addHeader(k_HEADER_DATE, options.Date);
  }

  static void ParseBasicResponseHeaders(
      const std::map<std::string, std::string>& headers,
      ResponseInfo& info)
  {
    info.RequestId = GetHeaderValue(headers, k_HEADER_X_MS_REQUEST_ID);
    info.ClientRequestID = GetHeaderValue(headers, k_HEADER_X_MS_CLIENT_REQUEST_ID);
    info.Version = GetHeaderValue(headers, k_HEADER_X_MS_VERSION);
    info.Date = GetHeaderValue(headers, k_HEADER_DATE);
  }

  static std::string GetHeaderValue(
      const std::map<std::string, std::string>& headers,
      const std::string& key)
  {
    if (headers.find(key) != headers.end())
    {
      return headers.at(key);
    }
    return std::string();
  }

  static std::string GetDateString()
  {
    char buf[30];
    std::time_t t = std::time(nullptr);
    std::tm* pm;
    pm = std::gmtime(&t);
    size_t s = std::strftime(
        buf,
        30,
        "%a, %d %b %Y %H:%M:%S GMT",
        pm);
    return std::string(buf, s);
  }
};

} // namespace Blobs
} // namespace Storage
} // namespace Azure

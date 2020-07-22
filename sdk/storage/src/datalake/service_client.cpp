// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "datalake/service_client.hpp"

#include "blobs/internal/protocol/blob_rest_client.hpp"
#include "common/common_headers_request_policy.hpp"
#include "common/constants.hpp"
#include "common/shared_key_policy.hpp"
#include "common/storage_common.hpp"
#include "common/storage_credential.hpp"
#include "credentials/policy/policies.hpp"
#include "datalake/datalake_utilities.hpp"
#include "datalake/file_system_client.hpp"
#include "http/curl/curl.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake {
  namespace {
    Blobs::BlobServiceClientOptions GetBlobServiceClientOptions(const ServiceClientOptions& options)
    {
      Blobs::BlobServiceClientOptions blobOptions;
      for (const auto& p : options.PerOperationPolicies)
      {
        blobOptions.PerOperationPolicies.emplace_back(p->Clone());
      }
      for (const auto& p : options.PerRetryPolicies)
      {
        blobOptions.PerRetryPolicies.emplace_back(p->Clone());
      }
      return blobOptions;
    }

    std::vector<FileSystem> FileSystemsFromContainerItems(
        const std::vector<Blobs::BlobContainerItem>& items)
    {
      std::vector<FileSystem> fileSystems;
      for (const auto& item : items)
      {
        FileSystem fileSystem;
        fileSystem.ETag = item.ETag;
        fileSystem.Name = item.Name;
        fileSystem.LastModified = item.LastModified;
        fileSystems.emplace_back(std::move(fileSystem));
      }
      return fileSystems;
    }
  } // namespace

  ServiceClient ServiceClient::CreateFromConnectionString(
      const std::string& connectionString,
      const ServiceClientOptions& options)
  {
    auto parsedConnectionString = Azure::Storage::Details::ParseConnectionString(connectionString);
    auto serviceUri = std::move(parsedConnectionString.DataLakeServiceUri);

    if (parsedConnectionString.KeyCredential)
    {
      return ServiceClient(serviceUri.ToString(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return ServiceClient(serviceUri.ToString(), options);
    }
  }

  ServiceClient::ServiceClient(
      const std::string& serviceUri,
      std::shared_ptr<SharedKeyCredential> credential,
      const ServiceClientOptions& options)
      : m_dfsUri(Details::GetDfsUriFromUri(serviceUri)), m_blobServiceClient(
                                                             Details::GetBlobUriFromUri(serviceUri),
                                                             credential,
                                                             GetBlobServiceClientOptions(options))
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::RetryPolicy>(Azure::Core::Http::RetryOptions()));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<CommonHeadersRequestPolicy>());
    policies.emplace_back(std::make_unique<SharedKeyPolicy>(credential));
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  ServiceClient::ServiceClient(
      const std::string& serviceUri,
      std::shared_ptr<Core::Credentials::TokenCredential> credential,
      const ServiceClientOptions& options)
      : m_dfsUri(Details::GetDfsUriFromUri(serviceUri)), m_blobServiceClient(
                                                             Details::GetBlobUriFromUri(serviceUri),
                                                             credential,
                                                             GetBlobServiceClientOptions(options))
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::RetryPolicy>(Azure::Core::Http::RetryOptions()));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<CommonHeadersRequestPolicy>());
    policies.emplace_back(
        std::make_unique<Core::Credentials::Policy::BearerTokenAuthenticationPolicy>(
            credential, Azure::Storage::Details::c_StorageScope));
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  ServiceClient::ServiceClient(const std::string& serviceUri, const ServiceClientOptions& options)
      : m_dfsUri(Details::GetDfsUriFromUri(serviceUri)), m_blobServiceClient(
                                                             Details::GetBlobUriFromUri(serviceUri),
                                                             GetBlobServiceClientOptions(options))
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::RetryPolicy>(Azure::Core::Http::RetryOptions()));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<CommonHeadersRequestPolicy>());
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  FileSystemClient ServiceClient::GetFileSystemClient(const std::string& fileSystemName) const
  {
    auto builder = m_dfsUri;
    builder.AppendPath(fileSystemName, true);
    return FileSystemClient(
        builder, m_blobServiceClient.GetBlobContainerClient(fileSystemName), m_pipeline);
  }

  Azure::Core::Response<ListFileSystemsResult> ServiceClient::ListFileSystems(
      const ListFileSystemsOptions& options) const
  {
    Blobs::ListBlobContainersOptions blobOptions;
    blobOptions.Context = options.Context;
    blobOptions.Prefix = options.Prefix;
    blobOptions.Marker = options.Continuation;
    blobOptions.MaxResults = options.MaxResults;
    auto result = m_blobServiceClient.ListBlobContainersSegment(blobOptions);
    auto response = ListFileSystemsResult();
    response.Continuation = result->NextMarker.empty() ? response.Continuation : result->NextMarker;
    response.Filesystems = FileSystemsFromContainerItems(result->Items);
    return Azure::Core::Response<ListFileSystemsResult>(
        std::move(response), result.ExtractRawResponse());
  }

}}}} // namespace Azure::Storage::Files::DataLake

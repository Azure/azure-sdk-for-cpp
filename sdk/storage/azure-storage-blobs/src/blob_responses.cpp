// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs/blob_responses.hpp"

#include <azure/storage/common/storage_switch_to_secondary_policy.hpp>

#include "azure/storage/blobs/blob_client.hpp"
#include "azure/storage/blobs/blob_container_client.hpp"
#include "azure/storage/blobs/blob_service_client.hpp"
#include "azure/storage/blobs/page_blob_client.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  std::unique_ptr<Azure::Core::Http::RawResponse> StartBlobCopyOperation::PollInternal(
      Azure::Core::Context&)
  {

    auto response = m_blobClient->GetProperties();
    if (!response.Value.CopyStatus.HasValue())
    {
      m_status = Azure::Core::OperationStatus::Failed;
    }
    else if (response.Value.CopyStatus.Value() == Models::CopyStatus::Pending)
    {
      m_status = Azure::Core::OperationStatus::Running;
    }
    else if (response.Value.CopyStatus.Value() == Models::CopyStatus::Success)
    {
      m_status = Azure::Core::OperationStatus::Succeeded;
    }
    else
    {
      m_status = Azure::Core::OperationStatus::Failed;
    }
    m_pollResult = response.Value;
    return std::move(response.RawResponse);
  }

  Azure::Response<Models::BlobProperties> StartBlobCopyOperation::PollUntilDoneInternal(
      std::chrono::milliseconds period,
      Azure::Core::Context& context)
  {
    while (true)
    {
      auto rawResponse = Poll(context);

      if (m_status == Azure::Core::OperationStatus::Succeeded)
      {
        return Azure::Response<Models::BlobProperties>(
            m_pollResult, std::make_unique<Azure::Core::Http::RawResponse>(rawResponse));
      }
      else if (m_status == Azure::Core::OperationStatus::Failed)
      {
        throw Azure::Core::RequestFailedException("Operation failed");
      }
      else if (m_status == Azure::Core::OperationStatus::Cancelled)
      {
        throw Azure::Core::RequestFailedException("Operation was cancelled");
      }

      std::this_thread::sleep_for(period);
    };
  }

  void ListBlobContainersPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    _detail::BlobRestClient::Service::ListBlobContainersSinglePageOptions protocolLayerOptions;
    protocolLayerOptions.Prefix = m_operationOptions.Prefix;
    protocolLayerOptions.ContinuationToken = NextPageToken;
    protocolLayerOptions.MaxResults = m_operationOptions.PageSizeHint;
    protocolLayerOptions.Include = m_operationOptions.Include;
    auto response = _detail::BlobRestClient::Service::ListBlobContainersSinglePage(
        *m_blobServiceClient->m_pipeline,
        m_blobServiceClient->m_serviceUrl,
        protocolLayerOptions,
        _internal::WithReplicaStatus(context));

    ServiceEndpoint = std::move(response.Value.ServiceEndpoint);
    Prefix = std::move(response.Value.Prefix);
    Items = std::move(response.Value.Items);
    NextPageToken = response.Value.ContinuationToken.ValueOr(std::string());
    RawResponse = std::move(response.RawResponse);
  }

  void FindBlobsByTagsPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    _detail::BlobRestClient::Service::FindBlobsByTagsSinglePageOptions protocolLayerOptions;
    protocolLayerOptions.Where = m_tagFilterSqlExpression;
    protocolLayerOptions.ContinuationToken = NextPageToken;
    protocolLayerOptions.MaxResults = m_operationOptions.PageSizeHint;
    auto response = _detail::BlobRestClient::Service::FindBlobsByTagsSinglePage(
        *m_blobServiceClient->m_pipeline,
        m_blobServiceClient->m_serviceUrl,
        protocolLayerOptions,
        _internal::WithReplicaStatus(context));

    ServiceEndpoint = std::move(response.Value.ServiceEndpoint);
    Items = std::move(response.Value.Items);
    NextPageToken = response.Value.ContinuationToken.ValueOr(std::string());
    RawResponse = std::move(response.RawResponse);
  }

  void ListBlobsPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    _detail::BlobRestClient::BlobContainer::ListBlobsSinglePageOptions protocolLayerOptions;
    protocolLayerOptions.Prefix = m_operationOptions.Prefix;
    protocolLayerOptions.ContinuationToken = NextPageToken;
    protocolLayerOptions.MaxResults = m_operationOptions.PageSizeHint;
    protocolLayerOptions.Include = m_operationOptions.Include;
    auto response = _detail::BlobRestClient::BlobContainer::ListBlobsSinglePage(
        *m_blobContainerClient->m_pipeline,
        m_blobContainerClient->m_blobContainerUrl,
        protocolLayerOptions,
        _internal::WithReplicaStatus(context));
    for (auto& i : response.Value.Items)
    {
      if (i.Details.AccessTier.HasValue() && !i.Details.IsAccessTierInferred.HasValue())
      {
        i.Details.IsAccessTierInferred = false;
      }
      if (i.VersionId.HasValue() && !i.IsCurrentVersion.HasValue())
      {
        i.IsCurrentVersion = false;
      }
      if (i.BlobType == Models::BlobType::AppendBlob && !i.Details.IsSealed)
      {
        i.Details.IsSealed = false;
      }
    }

    ServiceEndpoint = std::move(response.Value.ServiceEndpoint);
    BlobContainerName = std::move(response.Value.BlobContainerName);
    Prefix = std::move(response.Value.Prefix);
    Items = std::move(response.Value.Items);
    NextPageToken = response.Value.ContinuationToken.ValueOr(std::string());
    RawResponse = std::move(response.RawResponse);
  }

  void ListBlobsByHierarchyPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    _detail::BlobRestClient::BlobContainer::ListBlobsByHierarchySinglePageOptions
        protocolLayerOptions;
    protocolLayerOptions.Prefix = m_operationOptions.Prefix;
    protocolLayerOptions.Delimiter = Delimiter;
    protocolLayerOptions.ContinuationToken = NextPageToken;
    protocolLayerOptions.MaxResults = m_operationOptions.PageSizeHint;
    protocolLayerOptions.Include = m_operationOptions.Include;
    auto response = _detail::BlobRestClient::BlobContainer::ListBlobsByHierarchySinglePage(
        *m_blobContainerClient->m_pipeline,
        m_blobContainerClient->m_blobContainerUrl,
        protocolLayerOptions,
        _internal::WithReplicaStatus(context));
    for (auto& i : response.Value.Items)
    {
      if (i.Details.AccessTier.HasValue() && !i.Details.IsAccessTierInferred.HasValue())
      {
        i.Details.IsAccessTierInferred = false;
      }
      if (i.VersionId.HasValue() && !i.IsCurrentVersion.HasValue())
      {
        i.IsCurrentVersion = false;
      }
      if (i.BlobType == Models::BlobType::AppendBlob && !i.Details.IsSealed)
      {
        i.Details.IsSealed = false;
      }
    }

    ServiceEndpoint = std::move(response.Value.ServiceEndpoint);
    BlobContainerName = std::move(response.Value.BlobContainerName);
    Prefix = std::move(response.Value.Prefix);
    Delimiter = std::move(response.Value.Delimiter);
    Items = std::move(response.Value.Items);
    BlobPrefixes = std::move(response.Value.BlobPrefixes);
    NextPageToken = response.Value.ContinuationToken.ValueOr(std::string());
    RawResponse = std::move(response.RawResponse);
  }

  void GetPageRangesPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    _detail::BlobRestClient::PageBlob::GetPageBlobPageRangesOptions protocolLayerOptions;
    protocolLayerOptions.Range = m_operationOptions.Range;
    protocolLayerOptions.LeaseId = m_operationOptions.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = m_operationOptions.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = m_operationOptions.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = m_operationOptions.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = m_operationOptions.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfTags = m_operationOptions.AccessConditions.TagConditions;
    auto response = _detail::BlobRestClient::PageBlob::GetPageRanges(
        *m_pageBlobClient->m_pipeline,
        m_pageBlobClient->m_blobUrl,
        protocolLayerOptions,
        _internal::WithReplicaStatus(context));

    ETag = std::move(response.Value.ETag);
    LastModified = std::move(response.Value.LastModified);
    BlobSize = response.Value.BlobSize;
    PageRanges = std::move(response.Value.PageRanges);
    NextPageToken.clear();
    RawResponse = std::move(response.RawResponse);
  }

  void GetPageRangesDiffPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    _detail::BlobRestClient::PageBlob::GetPageBlobPageRangesOptions protocolLayerOptions;
    protocolLayerOptions.PreviousSnapshot = m_previousSnapshotUri;
    protocolLayerOptions.Range = m_operationOptions.Range;
    protocolLayerOptions.LeaseId = m_operationOptions.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = m_operationOptions.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = m_operationOptions.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = m_operationOptions.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = m_operationOptions.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfTags = m_operationOptions.AccessConditions.TagConditions;
    auto response = _detail::BlobRestClient::PageBlob::GetPageRanges(
        *m_pageBlobClient->m_pipeline,
        m_pageBlobClient->m_blobUrl,
        protocolLayerOptions,
        _internal::WithReplicaStatus(context));

    ETag = std::move(response.Value.ETag);
    LastModified = std::move(response.Value.LastModified);
    BlobSize = response.Value.BlobSize;
    PageRanges = std::move(response.Value.PageRanges);
    ClearRanges = std::move(response.Value.ClearRanges);
    NextPageToken.clear();
    RawResponse = std::move(response.RawResponse);
  }

}}} // namespace Azure::Storage::Blobs

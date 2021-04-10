// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs/blob_responses.hpp"

#include "azure/storage/blobs/blob_client.hpp"
#include "azure/storage/blobs/blob_container_client.hpp"

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

  void ListBlobsPageResult::OnNextPage(const Azure::Core::Context& context)
  {
    auto contextRef = std::cref(context);
    if (&context == &Azure::Core::Context::GetApplicationContext())
    {
      // Called via iterator increment, use the same context as the last call.
      contextRef = std::cref(m_context);
    }

    _detail::BlobRestClient::BlobContainer::ListBlobsSinglePageOptions protocolLayerOptions;
    protocolLayerOptions.Prefix = m_operationOptions.Prefix;
    protocolLayerOptions.ContinuationToken = NextPageToken;
    protocolLayerOptions.MaxResults = m_operationOptions.PageSizeHint;
    protocolLayerOptions.Include = m_operationOptions.Include;
    auto response = _detail::BlobRestClient::BlobContainer::ListBlobsSinglePage(
        *m_blobContainerClient->m_pipeline,
        m_blobContainerClient->m_blobContainerUrl,
        protocolLayerOptions,
        contextRef.get());
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

}}} // namespace Azure::Storage::Blobs

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/shares/share_responses.hpp"

#include <thread>

#include "azure/storage/files/shares/share_directory_client.hpp"
#include "azure/storage/files/shares/share_file_client.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  std::unique_ptr<Azure::Core::Http::RawResponse> StartFileCopyOperation::PollInternal(
      Azure::Core::Context&)
  {

    auto response = m_fileClient->GetProperties();
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

  Azure::Response<Models::FileProperties> StartFileCopyOperation::PollUntilDoneInternal(
      std::chrono::milliseconds period,
      Azure::Core::Context& context)
  {
    while (true)
    {
      auto rawResponse = Poll(context);

      if (m_status == Azure::Core::OperationStatus::Succeeded)
      {
        return Azure::Response<Models::FileProperties>(
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

  void ListFilesAndDirectoriesPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    auto protocolLayerOptions
        = _detail::ShareRestClient::Directory::ListFilesAndDirectoriesSinglePageOptions();
    protocolLayerOptions.Prefix = m_operationOptions.Prefix;
    protocolLayerOptions.ContinuationToken = NextPageToken;
    protocolLayerOptions.MaxResults = m_operationOptions.PageSizeHint;
    auto response = _detail::ShareRestClient::Directory::ListFilesAndDirectoriesSinglePage(
        m_shareDirectoryClient->m_shareDirectoryUrl,
        *m_shareDirectoryClient->m_pipeline,
        context,
        protocolLayerOptions);

    Models::ListFilesAndDirectoriesSinglePageResult ret;
    ServiceEndpoint = std::move(response.Value.ServiceEndpoint);
    ShareName = std::move(response.Value.ShareName);
    ShareSnapshot = std::move(response.Value.ShareSnapshot);
    DirectoryPath = std::move(response.Value.DirectoryPath);
    Prefix = std::move(response.Value.Prefix);
    DirectoryItems = std::move(response.Value.SinglePage.DirectoryItems);
    FileItems = std::move(response.Value.SinglePage.FileItems);
    NextPageToken = response.Value.ContinuationToken.ValueOr(std::string());
    RawResponse = std::move(response.RawResponse);
  }

  void ForceCloseAllFileHandlesPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    auto protocolLayerOptions = _detail::ShareRestClient::File::ForceCloseHandlesOptions();
    protocolLayerOptions.HandleId = FileAllHandles;
    protocolLayerOptions.ContinuationToken = NextPageToken;
    auto response = _detail::ShareRestClient::File::ForceCloseHandles(
        m_shareFileClient->m_shareFileUrl,
        *m_shareFileClient->m_pipeline,
        context,
        protocolLayerOptions);

    NumberOfHandlesClosed = response.Value.NumberOfHandlesClosed;
    NumberOfHandlesFailedToClose = response.Value.NumberOfHandlesFailedToClose;
    NextPageToken = response.Value.ContinuationToken.ValueOr(std::string());
    RawResponse = std::move(response.RawResponse);
  }

  void ListSharesPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    auto protocolLayerOptions = _detail::ShareRestClient::Service::ListSharesSinglePageOptions();
    protocolLayerOptions.IncludeFlags = m_operationOptions.ListSharesIncludeFlags;
    protocolLayerOptions.ContinuationToken = NextPageToken;
    protocolLayerOptions.MaxResults = m_operationOptions.PageSizeHint;
    protocolLayerOptions.Prefix = m_operationOptions.Prefix;
    auto response = _detail::ShareRestClient::Service::ListSharesSinglePage(
        m_shareServiceClient->m_serviceUrl,
        *m_shareServiceClient->m_pipeline,
        context,
        protocolLayerOptions);

    ServiceEndpoint = std::move(response.Value.ServiceEndpoint);
    Prefix = std::move(response.Value.Prefix);
    Items = std::move(response.Value.Items);
    NextPageToken = response.Value.ContinuationToken.ValueOr(std::string());
    RawResponse = std::move(response.RawResponse);
  }

  void ListDirectoryHandlesPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    auto protocolLayerOptions = _detail::ShareRestClient::Directory::ListHandlesOptions();
    protocolLayerOptions.ContinuationToken = NextPageToken;
    protocolLayerOptions.MaxResults = m_operationOptions.PageSizeHint;
    protocolLayerOptions.Recursive = m_operationOptions.Recursive;
    auto response = _detail::ShareRestClient::Directory::ListHandles(
        m_shareDirectoryClient->m_shareDirectoryUrl,
        *m_shareDirectoryClient->m_pipeline,
        context,
        protocolLayerOptions);

    Handles = std::move(response.Value.HandleList);
    NextPageToken = response.Value.ContinuationToken.ValueOr(std::string());
    RawResponse = std::move(response.RawResponse);
  }

  void ForceCloseAllDirectoryHandlesPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    auto protocolLayerOptions = _detail::ShareRestClient::Directory::ForceCloseHandlesOptions();
    protocolLayerOptions.HandleId = FileAllHandles;
    protocolLayerOptions.ContinuationToken = NextPageToken;
    protocolLayerOptions.Recursive = m_operationOptions.Recursive;
    auto response = _detail::ShareRestClient::Directory::ForceCloseHandles(
        m_shareDirectoryClient->m_shareDirectoryUrl,
        *m_shareDirectoryClient->m_pipeline,
        context,
        protocolLayerOptions);

    NumberOfHandlesClosed = response.Value.NumberOfHandlesClosed;
    NumberOfHandlesFailedToClose = response.Value.NumberOfHandlesFailedToClose;
    NextPageToken = response.Value.ContinuationToken.ValueOr(std::string());
    RawResponse = std::move(response.RawResponse);
  }
}}}} // namespace Azure::Storage::Files::Shares

// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/storage/files/shares/share_responses.hpp"

#include "azure/storage/files/shares/share_directory_client.hpp"
#include "azure/storage/files/shares/share_file_client.hpp"

#include <thread>

namespace Azure { namespace Storage { namespace Files { namespace Shares {
  namespace Models {

    ShareFileHandleAccessRights::ShareFileHandleAccessRights(const std::string& value)
    {
      if (!value.empty())
      {
        m_value.insert(value);
      }
    }
    ShareFileHandleAccessRights ShareFileHandleAccessRights::operator|(
        const ShareFileHandleAccessRights& other) const
    {
      ShareFileHandleAccessRights ret;
      std::set_union(
          m_value.begin(),
          m_value.end(),
          other.m_value.begin(),
          other.m_value.end(),
          std::inserter(ret.m_value, ret.m_value.begin()));
      return ret;
    }
    ShareFileHandleAccessRights ShareFileHandleAccessRights::operator&(
        const ShareFileHandleAccessRights& other) const
    {
      ShareFileHandleAccessRights ret;
      std::set_intersection(
          m_value.begin(),
          m_value.end(),
          other.m_value.begin(),
          other.m_value.end(),
          std::inserter(ret.m_value, ret.m_value.begin()));
      return ret;
    }
    ShareFileHandleAccessRights ShareFileHandleAccessRights::operator^(
        const ShareFileHandleAccessRights& other) const
    {
      ShareFileHandleAccessRights ret;
      std::set_symmetric_difference(
          m_value.begin(),
          m_value.end(),
          other.m_value.begin(),
          other.m_value.end(),
          std::inserter(ret.m_value, ret.m_value.begin()));
      return ret;
    }
    const ShareFileHandleAccessRights ShareFileHandleAccessRights::Read("Read");
    const ShareFileHandleAccessRights ShareFileHandleAccessRights::Write("Write");
    const ShareFileHandleAccessRights ShareFileHandleAccessRights::Delete("Delete");
  } // namespace Models

  std::unique_ptr<Azure::Core::Http::RawResponse> StartFileCopyOperation::PollInternal(
      const Azure::Core::Context&)
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
        throw Azure::Core::RequestFailedException("Operation failed.");
      }
      else if (m_status == Azure::Core::OperationStatus::Cancelled)
      {
        throw Azure::Core::RequestFailedException("Operation was cancelled.");
      }

      std::this_thread::sleep_for(period);
    }
  }

  void ListSharesPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    m_operationOptions.ContinuationToken = NextPageToken;
    *this = m_shareServiceClient->ListShares(m_operationOptions, context);
  }

  void ListFilesAndDirectoriesPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    m_operationOptions.ContinuationToken = NextPageToken;
    *this = m_shareDirectoryClient->ListFilesAndDirectories(m_operationOptions, context);
  }

  void ListFileHandlesPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    m_operationOptions.ContinuationToken = NextPageToken;
    *this = m_shareFileClient->ListHandles(m_operationOptions, context);
  }

  void ForceCloseAllFileHandlesPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    m_operationOptions.ContinuationToken = NextPageToken;
    *this = m_shareFileClient->ForceCloseAllHandles(m_operationOptions, context);
  }

  void ListDirectoryHandlesPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    m_operationOptions.ContinuationToken = NextPageToken;
    *this = m_shareDirectoryClient->ListHandles(m_operationOptions, context);
  }

  void ForceCloseAllDirectoryHandlesPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    m_operationOptions.ContinuationToken = NextPageToken;
    *this = m_shareDirectoryClient->ForceCloseAllHandles(m_operationOptions, context);
  }

}}}} // namespace Azure::Storage::Files::Shares

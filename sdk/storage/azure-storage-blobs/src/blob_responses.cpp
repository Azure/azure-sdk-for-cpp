// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs/blob_responses.hpp"

#include "azure/storage/blobs/blob_client.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  std::unique_ptr<Azure::Core::Http::RawResponse> StartCopyBlobOperation::PollInternal(
      Azure::Core::Context& context)
  {
    (void)context;

    auto response = m_blobClient->GetProperties();
    if (!response->CopyStatus.HasValue())
    {
      m_status = Azure::Core::OperationStatus::Failed;
    }
    else if (response->CopyStatus.GetValue() == Models::CopyStatus::Pending)
    {
      m_status = Azure::Core::OperationStatus::Running;
    }
    else if (response->CopyStatus.GetValue() == Models::CopyStatus::Success)
    {
      m_status = Azure::Core::OperationStatus::Succeeded;
    }
    else
    {
      m_status = Azure::Core::OperationStatus::Failed;
    }
    m_pollResult = *response;
    return response.ExtractRawResponse();
  }

  Azure::Response<Models::BlobProperties> StartCopyBlobOperation::PollUntilDoneInternal(
      std::chrono::milliseconds period,
      Azure::Core::Context& context)
  {
    while (true)
    {
      auto rawResponse = PollInternal(context);

      if (m_status == Azure::Core::OperationStatus::Succeeded)
      {
        return Azure::Response<Models::BlobProperties>(m_pollResult, std::move(rawResponse));
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

}}} // namespace Azure::Storage::Blobs

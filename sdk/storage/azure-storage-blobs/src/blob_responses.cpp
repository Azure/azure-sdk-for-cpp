// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs/blob_responses.hpp"

#include "azure/storage/blobs/blob_client.hpp"

namespace Azure { namespace Storage { namespace Blobs { namespace Models {

  std::unique_ptr<Azure::Core::Http::RawResponse> StartCopyBlobResult::PollInternal(
      Azure::Core::Context& context)
  {
    (void)context;

    auto response = m_blobClient->GetProperties();
    if (!response->CopyStatus.HasValue())
    {
      m_status = Azure::Core::OperationStatus::Failed;
    }
    else if (response->CopyStatus.GetValue() == CopyStatus::Pending)
    {
      m_status = Azure::Core::OperationStatus::Running;
    }
    else if (response->CopyStatus.GetValue() == CopyStatus::Success)
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

  Azure::Core::Response<GetBlobPropertiesResult> StartCopyBlobResult::PollUntilDoneInternal(
      Azure::Core::Context& context,
      std::chrono::milliseconds period)
  {
    while (true)
    {
      auto rawResponse = PollInternal(context);

      if (m_status == Azure::Core::OperationStatus::Succeeded)
      {
        return Azure::Core::Response<GetBlobPropertiesResult>(m_pollResult, std::move(rawResponse));
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

}}}} // namespace Azure::Storage::Blobs::Models

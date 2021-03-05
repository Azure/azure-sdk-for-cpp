// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/shares/share_responses.hpp"

#include <thread>

#include "azure/storage/files/shares/share_file_client.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  std::unique_ptr<Azure::Core::Http::RawResponse> StartCopyShareFileOperation::PollInternal(
      Azure::Core::Context& context)
  {
    (void)context;

    auto response = m_fileClient->GetProperties();
    if (!response->CopyStatus.HasValue())
    {
      m_status = Azure::Core::OperationStatus::Failed;
    }
    else if (response->CopyStatus.GetValue() == Models::CopyStatusType::Pending)
    {
      m_status = Azure::Core::OperationStatus::Running;
    }
    else if (response->CopyStatus.GetValue() == Models::CopyStatusType::Success)
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

  Azure::Core::Response<Models::GetShareFilePropertiesResult> StartCopyShareFileOperation::
      PollUntilDoneInternal(std::chrono::milliseconds period, Azure::Core::Context& context)
  {
    while (true)
    {
      auto rawResponse = PollInternal(context);

      if (m_status == Azure::Core::OperationStatus::Succeeded)
      {
        return Azure::Core::Response<Models::GetShareFilePropertiesResult>(
            m_pollResult, std::move(rawResponse));
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

}}}} // namespace Azure::Storage::Files::Shares

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/shares/share_responses.hpp"

#include <thread>

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
    else if (response.Value.CopyStatus.GetValue() == Models::CopyStatus::Pending)
    {
      m_status = Azure::Core::OperationStatus::Running;
    }
    else if (response.Value.CopyStatus.GetValue() == Models::CopyStatus::Success)
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

}}}} // namespace Azure::Storage::Files::Shares

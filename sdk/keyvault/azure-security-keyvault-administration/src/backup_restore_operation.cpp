#include "azure/keyvault/administration/backup_restore_client.hpp"
#include "azure/keyvault/administration/backup_restore_operation.hpp"

using namespace Azure::Security::KeyVault::Administration;

std::unique_ptr<Azure::Core::Http::RawResponse> BackupRestoreOperation::PollInternal(
    Azure::Core::Context const& context) 
{
  std::unique_ptr<Azure::Core::Http::RawResponse> rawResponse;
  try
  {
    Azure::Response<BackupRestoreOperationStatus> response = m_isBackupOperation
        ? m_keyClient->FullBackupStatus(m_continuationToken, context)
        : m_keyClient->RestoreStatus(m_continuationToken, context);

    m_value = response.Value;
    m_continuationToken = response.Value.JobId;
    rawResponse = std::move(response.RawResponse);
    if (response.Value.Status == "InProgress")
    {
      m_status = Azure::Core::OperationStatus::Running;
    }
    else if (response.Value.Status == "Succeeded")
    {
      m_status = Azure::Core::OperationStatus::Succeeded;
    }
    else if (response.Value.Status == "Failed")
    {
      m_status = Azure::Core::OperationStatus::Failed;
    }
    else
    {
      throw Azure::Core::RequestFailedException(response.RawResponse);
    }
  }
  catch (Azure::Core::RequestFailedException& error)
  {
    rawResponse = std::move(error.RawResponse);
  }

  return rawResponse;
}

Azure::Response<BackupRestoreOperationStatus> BackupRestoreOperation::PollUntilDoneInternal(
    std::chrono::milliseconds period,
    Azure::Core::Context& context) 
{
  while (true)
  {
    // Poll will update the raw response.
    Poll(context);
    if (IsDone())
    {
      break;
    }
    std::this_thread::sleep_for(period);
  }

  return Azure::Response<BackupRestoreOperationStatus>(
      m_value, std::make_unique<Azure::Core::Http::RawResponse>(*m_rawResponse));
}
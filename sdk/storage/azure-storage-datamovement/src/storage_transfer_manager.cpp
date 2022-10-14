#include "azure/storage/datamovement/storage_transfer_manager.hpp"

namespace Azure { namespace Storage {

  StorageTransferManager::StorageTransferManager(const StorageTransferManagerOptions& options)
      : m_options(options), m_scheduler(_internal::SchedulerOptions{})
  {
  }

  StorageTransferManager::~StorageTransferManager() {}

}} // namespace Azure::Storage

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/datamovement/storage_transfer_manager.hpp"

namespace Azure { namespace Storage {

  StorageTransferManager::StorageTransferManager(const StorageTransferManagerOptions& options)
      : m_scheduler(_internal::SchedulerOptions{options.NumThreads, options.MaxMemorySize}),
        m_jobEngine(options.TransferStateDirectoryPath + "/plan", &m_scheduler), m_options(options)
  {
  }

  StorageTransferManager::~StorageTransferManager()
  {
    // TODO: implement this
  }

  void StorageTransferManager::PauseJob(const std::string& jobId) { m_jobEngine.RemoveJob(jobId); }

  void StorageTransferManager::PauseAllJobs()
  {
    // TODO: implement this
  }

  JobProperties StorageTransferManager::ResumeJob(
      const std::string& jobId,
      const ResumeJobOptions& options)
  {
    _internal::HydrationParameters hydrateOptions;
    hydrateOptions.SourceCredential = options.SourceCredential;
    hydrateOptions.DestinationCredential = options.DestinationCredential;
    return m_jobEngine.ResumeJob(jobId, hydrateOptions);
  }

}} // namespace Azure::Storage

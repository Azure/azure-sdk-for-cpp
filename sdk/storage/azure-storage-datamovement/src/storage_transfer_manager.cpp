// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/datamovement/storage_transfer_manager.hpp"

#include "azure/storage/datamovement/utilities.hpp"

namespace Azure { namespace Storage {

  StorageTransferManager::StorageTransferManager(const StorageTransferManagerOptions& options)
      : m_transferEngine(
          _internal::TransferEngineOptions{options.NumThreads, options.MaxMemorySize}),
        m_jobEngine(
            _internal::JoinPath(options.TransferStateDirectoryPath, "plan"),
            &m_transferEngine),
        m_options(options)
  {
  }

  StorageTransferManager::~StorageTransferManager() { m_transferEngine.Stop(); }

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
    hydrateOptions.ProgressHandler = options.ProgressHandler;
    hydrateOptions.ErrorHandler = options.ErrorHandler;
    return m_jobEngine.ResumeJob(jobId, hydrateOptions);
  }

}} // namespace Azure::Storage

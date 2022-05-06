// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#include "azure/storage/datamovement/datamovement_options.hpp"
#include "azure/storage/datamovement/job_properties.hpp"
#include "azure/storage/datamovement/scheduler.hpp"

namespace Azure { namespace Storage {

  class StorageTransferManager {
  public:
    explicit StorageTransferManager(const StorageTransferManagerOptions& options);
    virtual ~StorageTransferManager() = 0;

    void PauseJob(const std::string& jobId);
    void PauseAllJobs();
    void ResumeJob(const std::string& jobId);
    void ResumeAllJobs();
    void CancelJob(const std::string& jobId);
    void CancelAllJobs();

  protected:
    StorageTransferManagerOptions m_options;
    _internal::Scheduler m_scheduler;
  };

}} // namespace Azure::Storage

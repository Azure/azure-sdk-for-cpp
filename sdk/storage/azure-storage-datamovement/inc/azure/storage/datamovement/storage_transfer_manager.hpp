// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#include "azure/storage/datamovement/datamovement_options.hpp"
#include "azure/storage/datamovement/job_engine.hpp"
#include "azure/storage/datamovement/job_properties.hpp"
#include "azure/storage/datamovement/scheduler.hpp"

namespace Azure { namespace Storage {

  class StorageTransferManager {
  public:
    explicit StorageTransferManager(const StorageTransferManagerOptions& options);
    virtual ~StorageTransferManager() = 0;

    void PauseJob(const std::string& jobId);
    void PauseAllJobs();
    JobProperties ResumeJob(
        const std::string& jobId,
        const ResumeJobOptions& options = ResumeJobOptions());

  protected:
    _internal::Scheduler m_scheduler;
    _detail::JobEngine m_jobEngine;

  private:
    StorageTransferManagerOptions m_options;
  };

}} // namespace Azure::Storage

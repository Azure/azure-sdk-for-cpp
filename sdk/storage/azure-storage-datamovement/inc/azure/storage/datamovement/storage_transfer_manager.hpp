// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

#include "azure/storage/datamovement/datamovement_options.hpp"
#include "azure/storage/datamovement/job_properties.hpp"
#include "azure/storage/datamovement/scheduler.hpp"

namespace Azure { namespace Storage {

  class StorageTransferManager {
  public:
    explicit StorageTransferManager(const StorageTransferManagerOptions& options);
    virtual ~StorageTransferManager() = 0;

    void CancelJob(const std::string& jobId);
    void CancelAllJobs();
    void PauseJob(const std::string& jobId);
    void PauseAllJobs();
    // Only unfinished jobs can be resumed. Failed or cancelled jobs cannot be resumed.
    JobProperties ResumeJob(
        const std::string& jobId,
        const ResumeJobOptions& options = ResumeJobOptions());

  protected:
    std::pair<JobProperties, _internal::Task> CreateJob(
        TransferType type,
        std::string sourceUrl,
        std::string destinationUrl);

  protected:
    _internal::Scheduler m_scheduler;

  private:
    std::shared_ptr<_internal::TaskSharedStatus> GetJobStatus(const std::string& jobId);

  private:
    StorageTransferManagerOptions m_options;

    // TODO: need some way to clean up finished jobs
    std::map<std::string, _internal::JobDetails> m_jobDetails;
    std::mutex m_jobDetailsMutex;
  };

}} // namespace Azure::Storage

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <atomic>
#include <condition_variable>
#include <fstream>
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <azure/core/nullable.hpp>
#include <azure/storage/blobs.hpp>

#include "azure/storage/datamovement/blob_folder.hpp"
#include "azure/storage/datamovement/datamovement_options.hpp"
#include "azure/storage/datamovement/filesystem.hpp"
#include "azure/storage/datamovement/job_properties.hpp"
#include "azure/storage/datamovement/scheduler.hpp"
#include "azure/storage/datamovement/task.hpp"
#include "azure/storage/datamovement/task_shared_status.hpp"

namespace Azure { namespace Storage {
  namespace _detail {
    class JobPlan;
    class JobEngine;
  } // namespace _detail
  namespace _internal {

    class TransferEnd {
    public:
      enum class EndType
      {
        LocalFile,
        LocalDirectory,
        AzureBlob,
        AzureBlobFolder,
      };

      std::string ToString() const;
      static TransferEnd FromString(const std::string& str, const TransferCredential& credential);
      static TransferEnd CreateFromLocalFile(const std::string& path);
      static TransferEnd CreateFromLocalDirectory(const std::string& path);
      static TransferEnd CreateFromAzureBlob(Blobs::BlobClient blobClient);
      static TransferEnd CreateFromAzureBlobFolder(Blobs::BlobFolder blobFolder);

    private:
      EndType m_type = static_cast<EndType>(0);
      std::string m_url;
      Nullable<Blobs::BlobClient> m_blobClient;
      Nullable<Blobs::BlobFolder> m_blobFolder;

      friend class _detail::JobPlan;
      friend class _detail::JobEngine;
    };

    struct JobModel
    {
      TransferEnd Source;
      TransferEnd Destination;
    };

    struct HydrationParameters
    {
      TransferCredential SourceCredential;
      TransferCredential DestinationCredential;
      std::function<void(const TransferProgress&)> ProgressHandler;
      std::function<void(TransferError&)> ErrorHandler;
    };
  } // namespace _internal

  namespace _detail {
    struct TaskModel
    {
      int32_t NumSubTasks = 0;
      std::string Source;
      std::string Destination;
      int64_t ObjectSize = -1;
      int64_t ChunkSize = -1;
      std::map<std::string, std::string> ExtendedAttributes;

      std::string ToString() const;
      static TaskModel FromString(const std::string& str);
    };

    struct PartGenerator
    {
      std::string Source;
      std::string Destination;
      std::string ContinuationToken;

      std::string ToString() const;
      static PartGenerator FromString(const std::string& str);
    };

    class JobPart {
    public:
      static std::pair<JobPart, std::vector<TaskModel>> LoadTasks(
          uint32_t id,
          std::string jobPlanDir);

      static void CreateJobPart(
          uint32_t id,
          const std::string& jobPlanDir,
          const std::vector<TaskModel>& tasks);

    private:
      uint32_t m_id = 0;
      std::string m_jobPlanDir;
      int32_t m_numDoneBits = 0;
      std::unique_ptr<_internal::MemoryMap> m_mappedFile;
      bool* m_doneBitmap = nullptr;

      friend class JobPlan;
    };

    class JobPlan {
    public:
      static void CreateJobPlan(const _internal::JobModel& model, const std::string& jobPlanDir);
      static JobPlan LoadJobPlan(
          _internal::HydrationParameters hydrateOptions,
          const std::string& jobPlanDir);

      void AppendPartGenerators(const std::vector<PartGenerator>& gens);
      void GenerateParts();
      std::vector<_internal::Task> HydrateTasks(const std::vector<TaskModel>& taskModels);

    private:
      JobPlan() = default;
      void GeneratePart(const PartGenerator& gen);
      void PartDone(uint32_t id);

      _internal::JobModel m_model;
      _internal::HydrationParameters m_hydrateOptions;
      std::string m_jobPlanDir;
      size_t m_generatorFileInOffset = 0;
      size_t m_generatorFileOutOffset = 0;
      bool m_hasMoreParts = false;
      std::fstream m_partGens;
      std::map<uint32_t, std::unique_ptr<JobPart>> m_jobParts;
      uint32_t m_maxPartId = 0;

      _internal::Task m_rootTask;

      friend class JobEngine;
    };

    class JobEngine {
    public:
      explicit JobEngine(const std::string& plansDir, _internal::Scheduler* scheduler);
      ~JobEngine();

      JobProperties CraeteJob(
          _internal::JobModel model,
          _internal::HydrationParameters hydrateOptions);
      JobProperties ResumeJob(
          const std::string& jobId,
          _internal::HydrationParameters hydrateOptions);
      void RemoveJob(const std::string& jobId);
      std::vector<_internal::Task> GetMoreTasks();

    private:
      struct EngineOperation
      {
        enum class OperationType
        {
          CreateJob,
          ResumeJob,
          RemoveJob,
          JobPartDone,
        };
        OperationType Type = static_cast<OperationType>(0);
        std::string JobId;
        uint32_t PartId = 0;
        _internal::JobModel Model;
        _internal::HydrationParameters HydrationParameters;
        std::promise<JobProperties> Promise;
      };
      void ProcessMessage(EngineOperation& op);

    private:
      std::string m_plansDir;
      _internal::Scheduler* m_scheduler;

      std::list<JobPlan> m_jobs;
      std::map<std::string, decltype(m_jobs)::iterator> m_jobsIndex;
      // TODO: optimization: last scan pos for GetMoreTasks()

      std::vector<EngineOperation> m_messages;
      std::mutex m_messageMutex;
      std::condition_variable m_messageCond;
      std::thread m_messageProcessor;
      std::atomic<bool> m_messageProcessorStop{false};
    };

  } // namespace _detail
}} // namespace Azure::Storage

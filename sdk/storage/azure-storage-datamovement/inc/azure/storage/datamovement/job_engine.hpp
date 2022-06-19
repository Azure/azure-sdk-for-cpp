// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <atomic>
#include <condition_variable>
#include <fstream>
#include <functional>
#include <future>
#include <list>
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
#include "azure/storage/datamovement/task.hpp"
#include "azure/storage/datamovement/task_shared_status.hpp"
#include "azure/storage/datamovement/transfer_engine.hpp"
#include "azure/storage/datamovement/utilities.hpp"

namespace Azure { namespace Storage {
  namespace _detail {
    struct JobPlan;
    class JobEngine;
  } // namespace _detail
  namespace _internal {

    class TransferEnd final {
    public:
      enum class EndType
      {
        Uninitialized,
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
      EndType m_type = EndType::Uninitialized;
      std::string m_url;
      Nullable<Blobs::BlobClient> m_blobClient;
      Nullable<Blobs::BlobFolder> m_blobFolder;

      friend struct _detail::JobPlan;
      friend class _detail::JobEngine;
    };

    struct JobModel final
    {
      TransferEnd Source;
      TransferEnd Destination;
    };

    struct HydrationParameters final
    {
      TransferCredential SourceCredential;
      TransferCredential DestinationCredential;
      std::function<void(TransferProgress&)> ProgressHandler;
      std::function<void(TransferError&)> ErrorHandler;
    };
  } // namespace _internal

  namespace _detail {
    struct TaskModel final
    {
      int32_t NumSubtasks = 0;
      std::string Source;
      std::string Destination;
      int64_t ObjectSize = -1;
      int64_t ChunkSize = -1;
      std::map<std::string, std::string> ExtendedAttributes;

      std::string ToString() const;
      static TaskModel FromString(const std::string& str);
    };

    struct PartGenerator final
    {
      std::string Source;
      std::string Destination;
      std::string ContinuationToken;

      std::string ToString() const;
      static PartGenerator FromString(const std::string& str);
    };

    struct JobPart final
    {
      JobPart() = default;
      JobPart(JobPart&&) = default;
      ~JobPart();

      static std::pair<JobPart, std::vector<TaskModel>> LoadTasks(JobPlan* plan, uint32_t id);

      static void CreateJobPart(
          uint32_t id,
          const std::string& jobPlanDir,
          const std::vector<TaskModel>& tasks);

      _internal::MovablePtr<JobPlan> m_jobPlan;
      uint32_t m_id = 0;
      size_t m_numDoneBits = 0;
      std::unique_ptr<std::atomic<size_t>> m_numUndoneBits
          = std::make_unique<std::atomic<size_t>>(0);
      std::unique_ptr<_internal::MemoryMap> m_mappedFile;
      bool* m_doneBitmap = nullptr;
    };

    struct JobPlan final
    {
      JobPlan() = default;
      JobPlan(JobPlan&&) = default;
      ~JobPlan();

      static void CreateJobPlan(const _internal::JobModel& model, const std::string& jobPlanDir);
      static JobPlan LoadJobPlan(
          _internal::HydrationParameters hydrateParameters,
          const std::string& jobPlanDir);

      void AppendPartGenerators(const std::vector<PartGenerator>& gens);
      void GenerateParts();
      std::vector<_internal::Task> HydrateTasks(
          std::shared_ptr<JobPart>& jobPart,
          const std::vector<TaskModel>& taskModels);

      void GeneratePartImpl(const PartGenerator& gen);
      void RemoveDonePart(uint32_t id);
      void TaskFinishCallback(
          const JournalContext& context,
          int64_t fileTransferred,
          int64_t fileSkipped,
          int64_t fileFailed,
          int64_t bytesTransferred);

      // job related
      JobEngine* m_engine = nullptr;
      std::string m_jobId;
      _internal::JobModel m_model;
      _internal::HydrationParameters m_hydrateParameters;
      _internal::Task m_rootTask;
      std::unique_ptr<std::atomic<uint64_t>> m_progressLastInvokedTime;

      // plan file related
      std::string m_jobPlanDir;
      size_t m_generatorFileInOffset = 0;
      size_t m_generatorFileOutOffset = 0;
      std::fstream m_partGens;

      // job info file
      std::unique_ptr<_internal::MemoryMap> m_jobInfoMappedFile;
      int64_t* m_numFilesTransferred = nullptr;
      int64_t* m_numFilesSkipped = nullptr;
      int64_t* m_numFilesFailed = nullptr;
      int64_t* m_totalBytesTransferred = nullptr;

      // parts
      std::map<uint32_t, std::shared_ptr<JobPart>> m_jobParts;
      std::unique_ptr<std::atomic<size_t>> m_numAliveParts
          = std::make_unique<std::atomic<size_t>>(0);
      bool m_hasMoreParts = false;
      uint32_t m_maxPartId = 0;
    };

    class JobEngine final {
    public:
      explicit JobEngine(const std::string& plansDir, _internal::TransferEngine* transferEngine);
      ~JobEngine();

      JobProperties CreateJob(
          _internal::JobModel model,
          _internal::HydrationParameters hydrateOptions);
      JobProperties ResumeJob(
          const std::string& jobId,
          _internal::HydrationParameters hydrateOptions);
      void RemoveJob(const std::string& jobId);
      void PartDone(const std::string& jobId, uint32_t partId);
      std::vector<_internal::Task> GetMoreTasks();

    private:
      struct EngineOperation final
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
      _internal::TransferEngine* m_transferEngine;

      std::list<JobPlan> m_jobs;
      std::map<std::string, decltype(m_jobs)::iterator> m_jobsIndex;
      std::pair<decltype(m_jobs)::iterator, uint32_t> m_loadPos = std::make_pair(m_jobs.end(), 0);

      std::vector<EngineOperation> m_messages;
      std::mutex m_messageMutex;
      std::condition_variable m_messageCond;
      std::thread m_messageProcessor;
      std::atomic<bool> m_messageProcessorStop{false};
    };

  } // namespace _detail
}} // namespace Azure::Storage

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

    /**
     * @brief On-disk data representation of a job. It consists of absolute paths of source and
     * destination. Credentials are not stored here, so it needs to be used in conjunction with
     * HydrationParameters after being loaded into memory.
     */
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
    /**
     * On-disk data respsentation of a task. Only relative paths of source and destination are
     * stored so it needs to be used in conjunction with JobModel.
     */
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

    /**
     * On-disk data representation of a task generator. Only relative paths of source and
     * destination are stored so it needs to be used in conjunction with JobModel.
     */
    struct PartGeneratorModel final
    {
      std::string Source;
      std::string Destination;
      std::string ContinuationToken;

      std::string ToString() const;
      static PartGeneratorModel FromString(const std::string& str);
    };

    /**
     * @brief JobPart struct manages status of a job part file.
     *
     * It's responsible for loading tasks from part file and mapping the bitmap region into memory
     * so that transfer engine can update the status of tasks.
     */
    struct JobPart final
    {
      JobPart() = default;
      JobPart(JobPart&&) = default;
      ~JobPart();

      static std::pair<JobPart, std::vector<TaskModel>> LoadTasks(JobPlan* plan, uint32_t id);

      _internal::MovablePtr<JobPlan> m_jobPlan;
      uint32_t m_id = 0;
      size_t m_numDoneBits = 0;
      std::unique_ptr<std::atomic<size_t>> m_numUndoneBits
          = std::make_unique<std::atomic<size_t>>(0);
      std::unique_ptr<_internal::MemoryMap> m_mappedFile;
      bool* m_doneBitmap = nullptr;
    };

    /**
     * @brief JobPlan struct manages plan files for one job.
     *
     * Plan files include:
     *   1. job_info, which records information of the job.
     *   2. part files, each of which consists of certain amount of tasks and a bitmap marking
     * status of these tasks.
     *   3. part_gen, which consists of several part generatos. Each generator can generates some
     * tasks and maybe some more generators.
     *
     * All plan-level and part-level operations are handled by this struct.
     */
    struct JobPlan final
    {
      JobPlan() = default;
      JobPlan(JobPlan&&) = default;
      ~JobPlan();

      static void CreateJobPlan(const _internal::JobModel& model, const std::string& jobPlanDir);
      static JobPlan LoadJobPlan(
          _internal::HydrationParameters hydrateParameters,
          const std::string& jobPlanDir);

      void AppendPartGenerators(const std::vector<PartGeneratorModel>& gens);
      void GenerateParts();
      std::vector<_internal::Task> HydrateTasks(
          std::shared_ptr<JobPart>& jobPart,
          const std::vector<TaskModel>& taskModels);

      void GeneratePartImpl(const PartGeneratorModel& gen);
      static void CreateJobPart(
          uint32_t id,
          const std::string& jobPlanDir,
          const std::vector<TaskModel>& tasks);
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

    /**
     * @brief JobEngine keeps track of all running jobs. It's responsible for creating new jobs,
     * pausing running jobs, resuming paused jobs and removing finished jobs. It also adds new tasks
     * to transfer engine when transfer engine is running low on tasks.
     */
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

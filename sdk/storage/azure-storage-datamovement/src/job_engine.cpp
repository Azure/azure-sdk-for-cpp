// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/datamovement/job_engine.hpp"

#include <chrono>

#include <azure/core/azure_assert.hpp>
#include <azure/core/uuid.hpp>

#include "azure/storage/datamovement/utilities.hpp"

namespace Azure { namespace Storage {

  namespace _internal {

    TransferEnd TransferEnd::CreateFromLocalFile(const std::string& path)
    {
      TransferEnd ret;
      ret.m_type = EndType::LocalFile;
      ret.m_url = _internal::PathToUrl(path);
      return ret;
    }

    TransferEnd TransferEnd::CreateFromLocalDirectory(const std::string& path)
    {
      TransferEnd ret;
      ret.m_type = EndType::LocalDirectory;
      ret.m_url = _internal::PathToUrl(path);
      return ret;
    }

    TransferEnd TransferEnd::CreateFromAzureBlob(Blobs::BlobClient blobClient)
    {
      TransferEnd ret;
      ret.m_type = EndType::AzureBlob;
      ret.m_blobClient = std::move(blobClient);
      ret.m_url = _internal::RemoveSasToken(ret.m_blobClient.Value().GetUrl());
      return ret;
    }

    TransferEnd TransferEnd::CreateFromAzureBlobFolder(Blobs::BlobFolder blobFolder)
    {
      TransferEnd ret;
      ret.m_type = EndType::AzureBlobFolder;
      ret.m_blobFolder = std::move(blobFolder);
      ret.m_url = _internal::RemoveSasToken(ret.m_blobFolder.Value().GetUrl());
      return ret;
    }

  } // namespace _internal
  namespace _detail {

    JobPart::~JobPart()
    {
      if (m_jobPlan)
      {
        m_jobPlan->m_numAliveParts->fetch_sub(1, std::memory_order_relaxed);
      }
    }

    void JobPlan::TaskFinishCallback(
        const JournalContext& context,
        int64_t fileTransferred,
        int64_t fileSkipped,
        int64_t fileFailed,
        int64_t bytesTransferred)
    {
      auto jobPart = context.JobPart.lock();
      if (!jobPart)
      {
        return;
      }
      jobPart->m_doneBitmap[context.BitmapOffset] = 1;
      jobPart->m_numUndoneBits->fetch_sub(1, std::memory_order_relaxed);
      if (fileTransferred != 0)
      {
        _internal::AtomicFetchAdd(m_numFilesTransferred, fileTransferred);
      }
      if (fileFailed != 0)
      {
        _internal::AtomicFetchAdd(m_numFilesFailed, fileFailed);
      }
      if (fileSkipped != 0)
      {
        _internal::AtomicFetchAdd(m_numFilesSkipped, fileSkipped);
      }
      if (bytesTransferred != 0)
      {
        _internal::AtomicFetchAdd(m_totalBytesTransferred, bytesTransferred);
      }
      if (m_hydrateParameters.ProgressHandler)
      {
        const uint64_t ProgressInvokedMinumumIntervalMs = 100;
        auto curr = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now().time_since_epoch())
                        .count();
        auto last = m_progressLastInvokedTime->load(std::memory_order_relaxed);
        if (curr - last >= ProgressInvokedMinumumIntervalMs)
        {
          bool successfullyExchanged
              = m_progressLastInvokedTime->compare_exchange_strong(last, curr);
          if (successfullyExchanged)
          {
            TransferProgress progress;
            progress.NumFilesTransferred = _internal::AtomicLoad(m_numFilesTransferred);
            progress.NumFilesFailed = _internal::AtomicLoad(m_numFilesFailed);
            progress.NumFilesSkipped = _internal::AtomicLoad(m_numFilesSkipped);
            progress.TotalBytesTransferred = _internal::AtomicLoad(m_totalBytesTransferred);
            m_hydrateParameters.ProgressHandler(progress);
          }
        }
      }
      if (jobPart->m_numUndoneBits->load(std::memory_order_relaxed) == 0)
      {
        uint32_t partId = jobPart->m_id;
        jobPart.reset();
        m_engine->PartDone(m_jobId, partId);
      }
    }

    JobPlan::~JobPlan()
    {
      if (m_rootTask)
      {
        auto currStatus = m_rootTask->SharedStatus->Status.load(std::memory_order_relaxed);
        if (currStatus == JobStatus::InProgress)
        {
          m_rootTask->SharedStatus->Status.compare_exchange_strong(
              currStatus, JobStatus::Paused, std::memory_order_relaxed, std::memory_order_relaxed);
        }
      }
      m_jobParts.clear();

      if (m_numAliveParts)
      {
        while (m_numAliveParts->load(std::memory_order_relaxed) != 0)
        {
          std::this_thread::yield();
        }
      }
    }

    JobEngine::JobEngine(const std::string& plansDir, _internal::TransferEngine* transferEngine)
        : m_plansDir(plansDir), m_transferEngine(transferEngine)
    {
      _internal::CreateDirectory(m_plansDir);

      m_messageProcessor = std::thread([this]() {
        std::unique_lock<std::mutex> guard(m_messageMutex, std::defer_lock);
        while (true)
        {
          guard.lock();
          m_messageCond.wait_for(guard, std::chrono::milliseconds(100));

          std::vector<EngineOperation> messages;
          messages.swap(m_messages);
          guard.unlock();

          for (auto& m : messages)
          {
            ProcessMessage(m);
          }

          if (m_messageProcessorStop.load(std::memory_order_relaxed))
          {
            break;
          }

          const size_t RefillQueueThreshold = 5000;
          if (m_transferEngine->m_numTasks.load(std::memory_order_relaxed) < RefillQueueThreshold
              && !m_transferEngine->m_stopped.load(std::memory_order_relaxed))
          {
            auto tasks = GetMoreTasks();
            if (!tasks.empty())
            {
              m_transferEngine->AddTasks(std::move(tasks));
            }
          }
        }
      });
    }

    JobEngine::~JobEngine()
    {
      m_messageProcessorStop.store(true, std::memory_order_relaxed);
      if (m_messageProcessor.joinable())
      {
        m_messageProcessor.join();
      }
    }

    JobProperties JobEngine::CreateJob(
        _internal::JobModel model,
        _internal::HydrationParameters hydrateParameters)
    {
      // TODO: exception handling during job creation/resumption
      auto uuid = Core::Uuid::CreateUuid().ToString();
      EngineOperation op;
      op.JobId = uuid;
      op.Type = decltype(op.Type)::CreateJob;
      op.Model = model;

      EngineOperation op2;
      op2.JobId = uuid;
      op2.Type = decltype(op.Type)::ResumeJob;
      op2.Model = std::move(model);
      op2.HydrationParameters = std::move(hydrateParameters);
      auto f = op2.Promise.get_future();

      {
        std::lock_guard<std::mutex> guard(m_messageMutex);
        m_messages.push_back(std::move(op));
        m_messages.push_back(std::move(op2));
      }
      m_messageCond.notify_one();

      return f.get();
    }

    JobProperties JobEngine::ResumeJob(
        const std::string& jobId,
        _internal::HydrationParameters hydrateOptions)
    {
      EngineOperation op;
      op.Type = decltype(op.Type)::ResumeJob;
      op.JobId = jobId;
      op.HydrationParameters = std::move(hydrateOptions);
      auto f = op.Promise.get_future();
      {
        std::lock_guard<std::mutex> guard(m_messageMutex);
        m_messages.push_back(std::move(op));
      }
      m_messageCond.notify_one();
      return f.get();
    }

    void JobEngine::RemoveJob(const std::string& jobId)
    {
      EngineOperation op;
      op.Type = decltype(op.Type)::RemoveJob;
      op.JobId = jobId;
      auto f = op.Promise.get_future();
      {
        std::lock_guard<std::mutex> guard(m_messageMutex);
        m_messages.push_back(std::move(op));
      }
      m_messageCond.notify_one();
      auto useless = f.get();
      (void)useless;
    }

    void JobEngine::PartDone(const std::string& jobId, uint32_t partId)
    {
      EngineOperation op;
      op.Type = decltype(op.Type)::JobPartDone;
      op.JobId = jobId;
      op.PartId = partId;
      {
        std::lock_guard<std::mutex> guard(m_messageMutex);
        m_messages.push_back(std::move(op));
      }
      m_messageCond.notify_one();
    }

    std::vector<_internal::Task> JobEngine::GetMoreTasks()
    {
      auto jobIte = m_loadPos.first;
      if (jobIte == m_jobs.end())
      {
        return {};
      }
      auto partIte = jobIte->m_jobParts.lower_bound(m_loadPos.second);
      while (true)
      {
        if (partIte != jobIte->m_jobParts.end())
        {
          m_loadPos.second = partIte->first + 1;
          if (partIte->second != nullptr)
          {
            ++partIte;
            continue;
          }
          else // if (partIte->second == nullptr)
          {
            auto p = JobPart::LoadTasks(&*jobIte, partIte->first);
            partIte->second = std::make_shared<JobPart>(std::move(p.first));
            std::vector<TaskModel> taskModels = std::move(p.second);
            if (!taskModels.empty())
            {
              std::vector<_internal::Task> tasks
                  = jobIte->HydrateTasks(partIte->second, taskModels);
              return tasks;
            }
            if (partIte->second->m_numUndoneBits->load(std::memory_order_relaxed) == 0)
            {
              PartDone(jobIte->m_jobId, partIte->second->m_id);
            }
          }
        }
        else // if (partIte == jobIte->m_jobParts.end())
        {
          if (jobIte->m_hasMoreParts)
          {
            jobIte->GenerateParts();
            partIte = jobIte->m_jobParts.lower_bound(m_loadPos.second);
          }
          else
          {
            ++jobIte;
            m_loadPos.first = jobIte;
            m_loadPos.second = 0;
            if (jobIte == m_jobs.end())
            {
              return {};
            }
            partIte = jobIte->m_jobParts.begin();
          }
        }
      }
      AZURE_UNREACHABLE_CODE();
    }

    void JobEngine::ProcessMessage(EngineOperation& op)
    {
      struct DummyTask final : public Storage::_internal::TaskBase
      {
        using TaskBase::TaskBase;
        void Execute() noexcept override { AZURE_UNREACHABLE_CODE(); }
      };

      if (op.Type == decltype(op.Type)::CreateJob)
      {
        JobPlan::CreateJobPlan(std::move(op.Model), _internal::JoinPath(m_plansDir, op.JobId));
      }
      else if (op.Type == decltype(op.Type)::ResumeJob)
      {
        if (m_jobsIndex.count(op.JobId) != 0)
        {
          op.Promise.set_exception(
              std::make_exception_ptr(std::runtime_error("Job already exists.")));
        }
        else
        {
          JobProperties properties;
          properties.Id = op.JobId;

          auto existingJobPlan = JobPlan::LoadJobPlan(
              std::move(op.HydrationParameters), _internal::JoinPath(m_plansDir, op.JobId));
          existingJobPlan.m_engine = this;
          existingJobPlan.m_jobId = op.JobId;
          if (op.Model.Source.m_type != decltype(op.Model.Source.m_type)::Uninitialized
              && op.Model.Destination.m_type != decltype(op.Model.Source.m_type)::Uninitialized)
          {
            existingJobPlan.m_model = std::move(op.Model);
          }
          auto sharedStatus = std::make_shared<_internal::TaskSharedStatus>();
          sharedStatus->ErrorHandler = existingJobPlan.m_hydrateParameters.ErrorHandler;
          sharedStatus->TransferEngine = m_transferEngine;
          sharedStatus->JobId = op.JobId;
          if (*existingJobPlan.m_numFilesFailed != 0)
          {
            sharedStatus->HasFailure = true;
          }
          if (*existingJobPlan.m_numFilesTransferred != 0
              || *existingJobPlan.m_numFilesSkipped != 0)
          {
            sharedStatus->HasSuccess = true;
          }
          existingJobPlan.m_rootTask = std::make_unique<DummyTask>(_internal::TaskType::Other);
          existingJobPlan.m_rootTask->SharedStatus = sharedStatus;
          if (existingJobPlan.m_model.Source.m_type == _internal::TransferEnd::EndType::LocalFile)
          {
            properties.Type = TransferType::SingleUpload;
          }
          else if (
              existingJobPlan.m_model.Source.m_type
              == _internal::TransferEnd::EndType::LocalDirectory)
          {
            properties.Type = TransferType::DirectoryUpload;
          }
          else if (
              existingJobPlan.m_model.Source.m_type == _internal::TransferEnd::EndType::AzureBlob)
          {
            properties.Type = TransferType::SingleDownload;
          }
          else if (
              existingJobPlan.m_model.Source.m_type
              == _internal::TransferEnd::EndType::AzureBlobFolder)
          {
            properties.Type = TransferType::DirectoryDownload;
          }
          else
          {
            AZURE_NOT_IMPLEMENTED();
          }
          properties.SourceUrl = _internal::RemoveSasToken(existingJobPlan.m_model.Source.m_url);
          properties.DestinationUrl
              = _internal::RemoveSasToken(existingJobPlan.m_model.Destination.m_url);
          properties.WaitHandle = sharedStatus->WaitHandle;

          m_jobs.push_back(std::move(existingJobPlan));
          auto jobIndex = --m_jobs.end();
          m_jobsIndex[op.JobId] = jobIndex;
          if (m_loadPos.first == m_jobs.end())
          {
            m_loadPos = std::make_pair(jobIndex, 0);
          }
          JobPlan* jobPlanPtr = &*jobIndex;
          sharedStatus->WriteJournal = [jobPlanPtr](
                                           const _detail::JournalContext& context,
                                           int64_t numFileTransferred,
                                           int64_t numFileSkipped,
                                           int64_t numFileFailed,
                                           int64_t bytesTransferred) {
            auto jobPartPtr = context.JobPart.lock();
            if (jobPartPtr)
            {
              jobPlanPtr->TaskFinishCallback(
                  context, numFileTransferred, numFileSkipped, numFileFailed, bytesTransferred);
            }
          };
          op.Promise.set_value(std::move(properties));
        }
      }
      else if (op.Type == decltype(op.Type)::RemoveJob)
      {
        auto ite = m_jobsIndex.find(op.JobId);
        if (ite == m_jobsIndex.end())
        {
          op.Promise.set_exception(std::make_exception_ptr(std::runtime_error("Cannot find job.")));
        }
        else
        {
          auto jobIndex = ite->second;
          if (m_loadPos.first == jobIndex)
          {
            m_loadPos = std::make_pair(++decltype(jobIndex)(jobIndex), 0);
          }
          m_jobs.erase(jobIndex);
          m_jobsIndex.erase(op.JobId);
          op.Promise.set_value({});
        }
      }
      else if (op.Type == decltype(op.Type)::JobPartDone)
      {
        auto& jobPlan = *m_jobsIndex[op.JobId];
        jobPlan.RemoveDonePart(op.PartId);
        if (jobPlan.m_jobParts.empty() && !jobPlan.m_hasMoreParts)
        {
          jobPlan.m_rootTask.reset();
          const std::string jobPlanDir = jobPlan.m_jobPlanDir;
          auto jobIndex = m_jobsIndex[op.JobId];
          if (m_loadPos.first == jobIndex)
          {
            m_loadPos = std::make_pair(++decltype(jobIndex)(jobIndex), 0);
          }
          m_jobs.erase(jobIndex);
          m_jobsIndex.erase(op.JobId);
          _internal::Rename(jobPlanDir, jobPlanDir + ".delete");
        }
      }
    }

  } // namespace _detail
}} // namespace Azure::Storage

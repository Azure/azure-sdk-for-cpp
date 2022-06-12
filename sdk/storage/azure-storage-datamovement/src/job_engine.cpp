// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/datamovement/job_engine.hpp"

#include <azure/core/azure_assert.hpp>
#include <azure/core/uuid.hpp>

#include "azure/storage/datamovement/utilities.hpp"

namespace Azure { namespace Storage {
  namespace {
    struct DummyTask final : public Storage::_internal::TaskBase
    {
      using TaskBase::TaskBase;
      void Execute() noexcept override { AZURE_UNREACHABLE_CODE(); }
    };

  } // namespace

  namespace _internal {

    TransferEnd TransferEnd::CreateFromLocalFile(const std::string& path)
    {
      TransferEnd ret;
      ret.m_type = EndType::LocalFile;
      ret.m_url = _internal::GetPathUrl(path);
      return ret;
    }

    TransferEnd TransferEnd::CreateFromLocalDirectory(const std::string& path)
    {
      TransferEnd ret;
      ret.m_type = EndType::LocalDirectory;
      ret.m_url = _internal::GetPathUrl(path);
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

          // TODO: check scheduler, add tasks
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

    JobProperties JobEngine::CraeteJob(
        _internal::JobModel model,
        _internal::HydrationParameters hydrateOptions)
    {
      auto uuid = Core::Uuid::CreateUuid().ToString();
      EngineOperation op;
      op.JobId = uuid;
      op.Type = decltype(op.Type)::CreateJob;
      op.Model = std::move(model);

      EngineOperation op2;
      op2.JobId = uuid;
      op2.Type = decltype(op.Type)::ResumeJob;
      op2.HydrationParameters = std::move(hydrateOptions);
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

    std::vector<_internal::Task> JobEngine::GetMoreTasks()
    {
      std::vector<TaskModel> taskModels;
      for (auto& job : m_jobs)
      {
        while (true)
        {
          for (auto& p1 : job.m_jobParts)
          {
            uint32_t partId = p1.first;
            auto& jobPart = p1.second;
            if (!jobPart)
            {
              auto p2 = JobPart::LoadTasks(partId, job.m_jobPlanDir);
              jobPart = std::make_unique<JobPart>(std::move(p2.first));
              taskModels = std::move(p2.second);
              break;
            }
          }
          if (taskModels.empty() && job.m_hasMoreParts)
          {
            job.GenerateParts();
          }
          else
          {
            break;
          }
        }
        if (!taskModels.empty())
        {
          std::vector<_internal::Task> tasks = job.HydrateTasks(taskModels);
          return tasks;
        }
      }
      return {};
    }

    void JobEngine::ProcessMessage(EngineOperation& op)
    {
      if (op.Type == decltype(op.Type)::CreateJob)
      {
        JobPlan::CreateJobPlan(std::move(op.Model), m_plansDir + "/" + op.JobId);
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

          auto existingJobPlan = JobPlan::LoadJobPlan(std::move(op.HydrationParameters), op.JobId);
          auto sharedStatus = std::make_shared<_internal::TaskSharedStatus>();
          sharedStatus->ProgressHandler = existingJobPlan.m_hydrateOptions.ProgressHandler;
          sharedStatus->ErrorHandler = existingJobPlan.m_hydrateOptions.ErrorHandler;
          sharedStatus->TransferEngine = m_transferEngine;
          sharedStatus->JobId = op.JobId;
          // TODO: restore counters
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
          properties.SourceUrl = existingJobPlan.m_model.Source.m_url;
          properties.DestinationUrl = existingJobPlan.m_model.Destination.m_url;
          properties.WaitHandle = sharedStatus->WaitHandle;

          m_jobs.push_back(std::move(existingJobPlan));
          auto jobIndex = m_jobs.end();
          --jobIndex;
          m_jobsIndex[op.JobId] = jobIndex;
          op.Promise.set_value(std::move(properties));
        }
      }
      else if (op.Type == decltype(op.Type)::RemoveJob)
      {
        m_jobs.erase(m_jobsIndex[op.JobId]);
        m_jobsIndex.erase(op.JobId);
        op.Promise.set_value({});
      }
      else if (op.Type == decltype(op.Type)::JobPartDone)
      {
        auto& jobPlan = *m_jobsIndex[op.JobId];
        jobPlan.PartDone(op.PartId);
        if (jobPlan.m_jobParts.empty() && !jobPlan.m_hasMoreParts)
        {
          m_jobs.erase(m_jobsIndex[op.JobId]);
          m_jobsIndex.erase(op.JobId);
          _internal::Rename(jobPlan.m_jobPlanDir, jobPlan.m_jobPlanDir + ".delete");
        }
      }
    }

  } // namespace _detail
}} // namespace Azure::Storage

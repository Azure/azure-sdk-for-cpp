// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/datamovement/job_engine.hpp"

#include <algorithm>
#include <fstream>

#include <azure/core/azure_assert.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/strings.hpp>

#include "azure/storage/datamovement/utilities.hpp"

namespace Azure { namespace Storage {
  namespace {
    constexpr static int PlanFileVersion = 1;
    constexpr size_t JobInfoFileHeaderSize = 32;
    constexpr size_t JobPartFileHeaderSize = 8;

    template <class T> T ReadFixedInt(std::fstream& in);

    template <> int8_t ReadFixedInt<int8_t>(std::fstream& in)
    {
      char v;
      in.read(&v, sizeof(v));
      return static_cast<int8_t>(v);
    }

    template <> int32_t ReadFixedInt<int32_t>(std::fstream& in)
    {
      uint8_t v[4];
      in.read(reinterpret_cast<char*>(v), sizeof(v));
      int32_t value = v[0] | (v[1] << 8) | (v[2] << 16) | (v[3] << 24);
      return value;
    }

    template <class T> void WriteFixedInt(std::fstream& out, T value)
    {
      uint8_t v[sizeof(value)];
      for (size_t i = 0; i < sizeof(value); ++i)
      {
        v[i] = (value >> (8 * i)) & 0xff;
      }
      out.write(reinterpret_cast<char*>(v), sizeof(v));
    }

    int64_t ReadVarInt(std::fstream& in)
    {
      uint64_t r = 0;
      int nb = 0;
      while (true)
      {
        uint8_t c = static_cast<uint8_t>(in.get());
        r = r | ((static_cast<uint64_t>(c) & 0x7f) << (nb * 7));
        if (c & 0x80)
        {
          ++nb;
          continue;
        }
        break;
      }
      return static_cast<int64_t>(r >> 1) ^ -static_cast<int64_t>(r & 0x01);
    }

    void WriteVarInt(std::fstream& out, int64_t value)
    {
      uint64_t v = (value << 1) ^ (value >> 63);
      while (true)
      {
        uint8_t c = v & 0x7f;
        v >>= 7;
        if (v != 0)
        {
          c |= 0x80;
        }
        out.put(c);
        if (v == 0)
        {
          break;
        }
      }
    }

    std::string ReadString(std::fstream& in)
    {
      int64_t length = ReadVarInt(in);
      if (length < 0)
      {
        throw std::runtime_error("Failed to parse plan file.");
      }
      std::string ret;
      ret.resize(length);
      if (length != 0)
      {
        in.read(&ret[0], length);
      }
      return ret;
    }

    void WriteString(std::fstream& out, const std::string& value)
    {
      WriteVarInt(out, static_cast<int64_t>(value.size()));
      if (!value.empty())
      {
        out.write(value.data(), value.size());
      }
    }

    void SkipString(std::fstream& in)
    {
      int64_t length = ReadVarInt(in);
      if (length < 0)
      {
        throw std::runtime_error("Failed to parse plan file.");
      }
      in.seekg(in.tellg() + length);
    }

    void WriteZeros(std::fstream& out, size_t numZeros)
    {
      const static std::string zeroMemory(4096, '\x00');
      while (numZeros)
      {
        size_t n = std::min(numZeros, zeroMemory.size());
        out.write(zeroMemory.data(), n);
        numZeros -= n;
      }
    }

    static const char* HexDigits = "0123456789abcdef";
    std::string PartIdToString(uint32_t partId)
    {

      std::string ret;
      while (partId)
      {
        ret += HexDigits[partId % 16];
        partId /= 16;
      }
      while (ret.length() < sizeof(partId) * 2)
      {
        ret.push_back('0');
      }
      std::reverse(ret.begin(), ret.end());
      return ret;
    }

    uint32_t PartIdFromString(const std::string& partId)
    {
      return std::stoul("0x" + partId, nullptr, 16);
    }
  } // namespace
  namespace _internal {
    std::string TransferEnd::ToString() const
    {
      Core::Json::_internal::json object;
      object["type"] = m_type;
      switch (m_type)
      {
        case EndType::LocalFile:
        case EndType::LocalDirectory:
          object["url"] = m_url;
          break;
        case EndType::AzureBlob:
          object["url"] = _internal::RemoveSasToken(m_blobClient.Value().GetUrl());
          break;
        case EndType::AzureBlobFolder:
          object["url"] = _internal::RemoveSasToken(m_blobFolder.Value().GetUrl());
          object["folder_path"] = m_blobFolder.Value().m_folderPath;
          break;
        default:
          AZURE_UNREACHABLE_CODE();
      }
      return object.dump();
    }

    TransferEnd TransferEnd::FromString(
        const std::string& str,
        const TransferCredential& credential)
    {
      TransferEnd ret;
      auto object = Core::Json::_internal::json::parse(str);
      ret.m_type = static_cast<EndType>(object["type"].get<std::underlying_type_t<EndType>>());
      switch (ret.m_type)
      {
        case EndType::LocalFile:
        case EndType::LocalDirectory:
          ret.m_url = object["url"].get<std::string>();
          break;
        case EndType::AzureBlob: {
          auto blobUrl = object["url"].get<std::string>();
          ret.m_url = blobUrl;
          if (!credential.SasCredential.empty())
          {
            ret.m_blobClient
                = Blobs::BlobClient(_internal::ApplySasToken(blobUrl, credential.SasCredential));
          }
          else if (credential.SharedKeyCredential)
          {
            ret.m_blobClient = Blobs::BlobClient(blobUrl, credential.SharedKeyCredential);
          }
          else if (credential.TokenCredential)
          {
            ret.m_blobClient = Blobs::BlobClient(blobUrl, credential.TokenCredential);
          }
          else
          {
            ret.m_blobClient = Blobs::BlobClient(blobUrl);
          }
          break;
        }
        case EndType::AzureBlobFolder: {
          auto folderUrl = object["url"].get<std::string>();
          auto folderPath = object["folder_path"].get<std::string>();
          auto blobContainerUrl = folderUrl.substr(0, folderUrl.length() - folderPath.length());
          Nullable<Blobs::BlobContainerClient> blobContainerClient;
          ret.m_url = folderUrl;
          if (!credential.SasCredential.empty())
          {
            blobContainerClient = Blobs::BlobContainerClient(
                _internal::ApplySasToken(blobContainerUrl, credential.SasCredential));
          }
          else if (credential.SharedKeyCredential)
          {
            blobContainerClient
                = Blobs::BlobContainerClient(blobContainerUrl, credential.SharedKeyCredential);
          }
          else if (credential.TokenCredential)
          {
            blobContainerClient
                = Blobs::BlobContainerClient(blobContainerUrl, credential.TokenCredential);
          }
          else
          {
            blobContainerClient = Blobs::BlobContainerClient(blobContainerUrl);
          }
          ret.m_blobFolder = Blobs::BlobFolder(std::move(blobContainerClient.Value()), folderPath);
          break;
        }
        default:
          AZURE_UNREACHABLE_CODE();
      }
      return ret;
    }
  } // namespace _internal

  namespace _detail {
    std::string TaskModel::ToString() const
    {
      Core::Json::_internal::json object;
      object["num_subtasks"] = NumSubTasks;
      object["source"] = Source;
      object["destination"] = Destination;
      object["object_size"] = ObjectSize;
      object["chunk_size"] = ChunkSize;
      if (!ExtendedAttributes.empty())
      {
        object["extended"] = ExtendedAttributes;
      }
      return object.dump();
    }

    TaskModel TaskModel::FromString(const std::string& str)
    {
      auto object = Core::Json::_internal::json::parse(str);
      TaskModel ret;
      ret.NumSubTasks = object["num_subtasks"].get<int32_t>();
      ret.Source = object["source"].get<std::string>();
      ret.Destination = object["destination"].get<std::string>();
      ret.ObjectSize = object["object_size"].get<int64_t>();
      ret.ChunkSize = object["chunk_size"].get<int64_t>();
      if (object.count("extended") != 0)
      {
        for (const auto& pair : object["extended"].items())
        {
          ret.ExtendedAttributes[pair.key()] = pair.value().get<std::string>();
        }
      }
      return ret;
    }

    std::string PartGenerator::ToString() const
    {
      Core::Json::_internal::json object;
      object["source"] = Source;
      object["destination"] = Destination;
      if (!ContinuationToken.empty())
      {
        object["continuation_token"] = ContinuationToken;
      }
      return object.dump();
    }

    PartGenerator PartGenerator::FromString(const std::string& str)
    {
      auto object = Core::Json::_internal::json::parse(str);
      PartGenerator ret;
      ret.Source = object["source"].get<std::string>();
      ret.Destination = object["destination"].get<std::string>();
      if (object.count("continuation_token") != 0)
      {
        ret.ContinuationToken = object["continuation_token"].get<std::string>();
      }
      return ret;
    }

    std::pair<JobPart, std::vector<TaskModel>> JobPart::LoadTasks(
        JobPlan* plan,
        uint32_t id,
        std::string jobPlanDir)
    {
      const std::string partFilename = _internal::JoinPath(jobPlanDir, PartIdToString(id));
      std::fstream fin(partFilename, std::fstream::in | std::fstream::binary);
      fin.exceptions(std::fstream::failbit | std::fstream::badbit);
      int32_t planFileVersion = ReadFixedInt<int32_t>(fin);
      AZURE_ASSERT(planFileVersion == PlanFileVersion);
      JobPart jobPart;
      jobPart.m_jobPlan = plan;
      jobPart.m_jobPlan->m_numAliveParts->fetch_add(1, std::memory_order_relaxed);
      jobPart.m_id = id;
      jobPart.m_jobPlanDir = std::move(jobPlanDir);
      jobPart.m_numDoneBits = ReadFixedInt<int32_t>(fin);
      AZURE_ASSERT(static_cast<size_t>(fin.tellg()) == JobPartFileHeaderSize);
      std::vector<char> doneBits(jobPart.m_numDoneBits);
      fin.read(doneBits.data(), jobPart.m_numDoneBits);

      std::vector<TaskModel> tasks;
      size_t currDoneBit = 0;
      size_t numUndoneBits = 0;
      while (currDoneBit < jobPart.m_numDoneBits)
      {
        auto numSubTasks = static_cast<int32_t>(ReadVarInt(fin));
        AZURE_ASSERT(numSubTasks > 0);
        if (numSubTasks == 1)
        {
          if (!doneBits[currDoneBit])
          {
            auto task = TaskModel::FromString(ReadString(fin));
            tasks.push_back(std::move(task));
            ++numUndoneBits;
          }
        }
        else
        {
          bool hasUndoneSubtask = false;
          std::string subtasksDoneBitMap(numSubTasks, '\x00');
          for (size_t i = 0; i < static_cast<size_t>(numSubTasks); ++i)
          {
            subtasksDoneBitMap[i] = char(doneBits[currDoneBit + i] + '0');
            if (subtasksDoneBitMap[i] == '0')
            {
              hasUndoneSubtask = true;
              ++numUndoneBits;
            }
          }
          if (hasUndoneSubtask)
          {
            auto task = TaskModel::FromString(ReadString(fin));
            task.ExtendedAttributes["_subtasks"] = subtasksDoneBitMap;
            tasks.push_back(std::move(task));
          }
        }
        currDoneBit += numSubTasks;
      }
      fin.close();
      jobPart.m_mappedFile = std::make_unique<_internal::MemoryMap>(partFilename);
      jobPart.m_doneBitmap = static_cast<bool*>(
          jobPart.m_mappedFile->Map(JobPartFileHeaderSize, jobPart.m_numDoneBits));
      jobPart.m_numUndoneBits->store(numUndoneBits, std::memory_order_relaxed);
      return std::make_pair(std::move(jobPart), std::move(tasks));
    }

    void JobPart::CreateJobPart(
        uint32_t id,
        const std::string& jobPlanDir,
        const std::vector<TaskModel>& tasks)
    {
      const std::string partFilename = _internal::JoinPath(jobPlanDir, PartIdToString(id));
      std::fstream fOut(
          partFilename + ".tmp", std::fstream::out | std::fstream::trunc | std::fstream::binary);
      fOut.exceptions(std::fstream::failbit | std::fstream::badbit);
      WriteFixedInt(fOut, PlanFileVersion);
      int32_t numDoneBits
          = std::accumulate(tasks.begin(), tasks.end(), 0, [](int32_t s, const TaskModel& t) {
              return s + t.NumSubTasks;
            });
      WriteFixedInt(fOut, numDoneBits);
      WriteZeros(fOut, numDoneBits);
      for (const auto& t : tasks)
      {
        WriteVarInt(fOut, t.NumSubTasks);
        WriteString(fOut, t.ToString());
      }
      fOut.close();
      _internal::Rename(partFilename + ".tmp", partFilename);
    }

    void JobPlan::CreateJobPlan(const _internal::JobModel& model, const std::string& jobPlanDir)
    {
      AZURE_ASSERT(!_internal::PathExists(jobPlanDir));
      _internal::CreateDirectory(jobPlanDir);
      {
        const std::string partGensFilename = _internal::JoinPath(jobPlanDir, "part_gens");
        std::fstream fout(
            partGensFilename + ".tmp",
            std::fstream::out | std::fstream::trunc | std::fstream::binary);
        fout.exceptions(std::fstream::failbit | std::fstream::badbit);
        PartGenerator rootGenerator;
        std::string serializedGenerator = rootGenerator.ToString();
        WriteFixedInt(fout, int8_t(0));
        WriteString(fout, serializedGenerator);
        fout.close();
        _internal::Rename(partGensFilename + ".tmp", partGensFilename);
      }
      {
        const std::string jobInfoFilename = _internal::JoinPath(jobPlanDir, "job_info");
        std::fstream fout(
            jobInfoFilename + ".tmp",
            std::fstream::out | std::fstream::trunc | std::fstream::binary);
        fout.exceptions(std::fstream::failbit | std::fstream::badbit);
        WriteZeros(fout, JobInfoFileHeaderSize);
        Core::Json::_internal::json object;
        object["source"] = Core::Json::_internal::json::parse(model.Source.ToString());
        object["destination"] = Core::Json::_internal::json::parse(model.Destination.ToString());
        std::string serializedJobInfo = object.dump();
        WriteString(fout, serializedJobInfo);
        fout.close();
        _internal::Rename(jobInfoFilename + ".tmp", jobInfoFilename);
      }
    }

    JobPlan JobPlan::LoadJobPlan(
        _internal::HydrationParameters hydrateOptions,
        const std::string& jobPlanDir)
    {
      JobPlan jobPlan;
      jobPlan.m_hydrateParameters = std::move(hydrateOptions);
      jobPlan.m_jobPlanDir = jobPlanDir;

      AZURE_ASSERT(_internal::PathExists(jobPlanDir));
      std::fstream fin(
          _internal::JoinPath(jobPlanDir, "job_info"), std::fstream::in | std::fstream::binary);
      fin.exceptions(std::fstream::failbit | std::fstream::badbit);
      fin.seekg(JobInfoFileHeaderSize);
      auto serializedJobInfo = ReadString(fin);
      auto object = Core::Json::_internal::json::parse(serializedJobInfo);
      jobPlan.m_model.Source = _internal::TransferEnd::FromString(
          object["source"].dump(), jobPlan.m_hydrateParameters.SourceCredential);
      jobPlan.m_model.Destination = _internal::TransferEnd::FromString(
          object["destination"].dump(),
          jobPlan.m_hydrateParameters.DestinationCredential);
      fin.close();

      jobPlan.m_jobInfoMappedFile
          = std::make_unique<_internal::MemoryMap>(_internal::JoinPath(jobPlanDir, "job_info"));
      void* jobInfoFileHeader = jobPlan.m_jobInfoMappedFile->Map(0, JobInfoFileHeaderSize);
      jobPlan.m_numFilesTransferred = static_cast<int64_t*>(jobInfoFileHeader);
      jobPlan.m_numFilesSkipped = static_cast<int64_t*>(jobInfoFileHeader) + 1;
      jobPlan.m_numFilesFailed = static_cast<int64_t*>(jobInfoFileHeader) + 2;
      jobPlan.m_totalBytesTransferred = static_cast<int64_t*>(jobInfoFileHeader) + 3;
      if (jobPlan.m_hydrateParameters.ProgressHandler)
      {
        jobPlan.m_progressLastInvokedTime = std::make_unique<std::atomic<uint64_t>>(0);
      }

      _internal::DirectoryIterator dir(jobPlan.m_jobPlanDir);
      while (true)
      {
        auto entry = dir.Next();
        if (entry.Name.empty())
        {
          break;
        }
        if (entry.IsDirectory)
        {
          continue;
        }
        if (entry.Name == "part_gens")
        {
          auto fio = std::fstream(
              _internal::JoinPath(jobPlan.m_jobPlanDir, "part_gens"),
              std::fstream::in | std::fstream::out | std::fstream::binary);
          fio.exceptions(std::fstream::failbit | std::fstream::badbit);
          jobPlan.m_generatorFileOutOffset = entry.Size;
          while (static_cast<size_t>(fio.tellg()) < jobPlan.m_generatorFileOutOffset)
          {
            bool doneBit = ReadFixedInt<int8_t>(fio);
            if (doneBit)
            {
              SkipString(fio);
            }
            else
            {
              size_t offset = fio.tellg();
              --offset;
              fio.seekg(offset);
              jobPlan.m_partGens = std::move(fio);
              jobPlan.m_hasMoreParts = true;
              jobPlan.m_generatorFileInOffset = offset;
              break;
            }
          }
        }
        static size_t PartIdFileLength = PartIdToString(0).length();
        if (entry.Name.length() == PartIdFileLength
            && Core::_internal::StringExtensions::ToLower(entry.Name).find_first_not_of(HexDigits)
                == std::string::npos)
        {
          uint32_t partId = PartIdFromString(entry.Name);
          jobPlan.m_jobParts.emplace(partId, nullptr);
          jobPlan.m_maxPartId = std::max(partId, jobPlan.m_maxPartId);
        }
        if (entry.Name.length() == PartIdFileLength + std::strlen(".delete")
            && Core::_internal::StringExtensions::ToLower(entry.Name.substr(0, PartIdFileLength))
                    .find_first_not_of(HexDigits)
                == std::string::npos)
        {
          uint32_t partId = PartIdFromString(entry.Name.substr(0, PartIdFileLength));
          jobPlan.m_maxPartId = std::max(partId, jobPlan.m_maxPartId);
        }
      }

      return jobPlan;
    }

    void JobPlan::AppendPartGenerators(const std::vector<PartGenerator>& gens)
    {
      if (gens.empty())
      {
        return;
      }
      m_partGens.seekp(m_generatorFileOutOffset);
      for (const auto& gen : gens)
      {
        WriteFixedInt(m_partGens, int8_t(0));
        WriteString(m_partGens, gen.ToString());
      }
      m_generatorFileOutOffset = m_partGens.tellp();
      m_partGens.flush();
    }

    void JobPlan::GenerateParts()
    {
      AZURE_ASSERT(m_hasMoreParts);

      while (true)
      {
        if (m_generatorFileInOffset == m_generatorFileOutOffset)
        {
          m_hasMoreParts = false;
          m_partGens.close();
          const std::string partGensFilename = _internal::JoinPath(m_jobPlanDir, "part_gens");
          _internal::Rename(partGensFilename, partGensFilename + ".delete");
          break;
        }
        m_partGens.seekg(m_generatorFileInOffset);
        size_t doneBitOffset = m_partGens.tellg();
        bool doneBit = ReadFixedInt<int8_t>(m_partGens);
        if (doneBit)
        {
          SkipString(m_partGens);
          m_generatorFileInOffset = m_partGens.tellg();
        }
        else
        {
          auto partGen = PartGenerator::FromString(ReadString(m_partGens));
          m_generatorFileInOffset = m_partGens.tellg();
          GeneratePart(partGen);
          m_partGens.seekp(doneBitOffset);
          WriteFixedInt(m_partGens, int8_t(1));
          m_partGens.flush();
        }
      }
    }

    void JobPlan::RemoveDonePart(uint32_t id)
    {
      auto ite = m_jobParts.find(id);
      AZURE_ASSERT(ite != m_jobParts.end());
      auto& jobPart = *ite->second;
      for (size_t i = 0; i < jobPart.m_numDoneBits; ++i)
      {
        AZURE_ASSERT(jobPart.m_doneBitmap[i]);
      }
      m_jobParts.erase(ite);
      const std::string jobPlanDirectoryName
          = _internal::JoinPath(m_jobPlanDir, PartIdToString(id));
      _internal::Rename(jobPlanDirectoryName, jobPlanDirectoryName + ".delete");
    }
  } // namespace _detail
}} // namespace Azure::Storage

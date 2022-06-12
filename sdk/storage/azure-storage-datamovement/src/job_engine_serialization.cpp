// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/datamovement/job_engine.hpp"

#include <fstream>

#include <azure/core/azure_assert.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/strings.hpp>

#include "azure/storage/datamovement/utilities.hpp"

namespace Azure { namespace Storage {
  namespace {
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

    void WriteFixedInt(std::fstream& out, int8_t value)
    {
      out.write(reinterpret_cast<char*>(&value), sizeof(value));
    }

    void WriteFixedInt(std::fstream& out, int32_t value)
    {
      uint8_t v[4];
      v[0] = value & 0xff;
      v[1] = (value >> 8) & 0xff;
      v[2] = (value >> 16) & 0xff;
      v[3] = (value >> 24) & 0xff;
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

    constexpr static int PlanFileVersion = 1;
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
      return stoul("0x" + partId, nullptr, 16);
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
        }
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
      object["extended"] = ExtendedAttributes;
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
        uint32_t id,
        std::string jobPlanDir)
    {
      const std::string partFilename = jobPlanDir + "/" + PartIdToString(id);
      std::fstream fin(partFilename, std::fstream::in | std::fstream::binary);
      fin.exceptions(std::fstream::failbit | std::fstream::badbit);
      int32_t planFileVersion = ReadFixedInt<int32_t>(fin);
      AZURE_ASSERT(planFileVersion == PlanFileVersion);
      JobPart jobPart;
      jobPart.m_id = id;
      jobPart.m_jobPlanDir = std::move(jobPlanDir);
      jobPart.m_numDoneBits = ReadFixedInt<int32_t>(fin);
      const size_t doneBitmapOffset = fin.tellg();
      std::vector<char> doneBits(jobPart.m_numDoneBits);
      fin.read(doneBits.data(), jobPart.m_numDoneBits);

      std::vector<TaskModel> tasks;
      size_t currDoneBit = 0;
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
          }
        }
        else
        {
          auto subtasksDoneBitMap = std::string(
              doneBits.begin() + currDoneBit, doneBits.begin() + currDoneBit + numSubTasks);
          if (std::any_of(subtasksDoneBitMap.begin(), subtasksDoneBitMap.end(), [](char b) {
                return b == 0;
              }))
          {
            auto task = TaskModel::FromString(ReadString(fin));
            task.ExtendedAttributes["_subtasks"] = subtasksDoneBitMap;
          }
        }
        currDoneBit += numSubTasks;
      }
      fin.close();
      jobPart.m_mappedFile = std::make_unique<_internal::MemoryMap>(partFilename);
      jobPart.m_doneBitmap
          = static_cast<bool*>(jobPart.m_mappedFile->Map(doneBitmapOffset, jobPart.m_numDoneBits));

      return std::make_pair(std::move(jobPart), std::move(tasks));
    }

    void JobPart::CreateJobPart(
        uint32_t id,
        const std::string& jobPlanDir,
        const std::vector<TaskModel>& tasks)
    {
      const std::string partFilename = jobPlanDir + "/" + PartIdToString(id);
      std::fstream fout(
          partFilename + ".tmp", std::fstream::out | std::fstream::trunc | std::fstream::binary);
      fout.exceptions(std::fstream::failbit | std::fstream::badbit);
      WriteFixedInt(fout, PlanFileVersion);
      int32_t numDoneBits
          = std::accumulate(tasks.begin(), tasks.end(), 0, [](int32_t s, const TaskModel& t) {
              return s + t.NumSubTasks;
            });
      WriteFixedInt(fout, numDoneBits);
      std::string zeroArray(4096, '\x00');
      while (numDoneBits != 0)
      {
        int32_t writeCount = std::min(static_cast<int32_t>(zeroArray.length()), numDoneBits);
        fout.write(zeroArray.data(), writeCount);
        numDoneBits -= writeCount;
      }
      for (const auto& t : tasks)
      {
        WriteVarInt(fout, t.NumSubTasks);
        WriteString(fout, t.ToString());
      }
      fout.close();
      _internal::Rename(partFilename + ".tmp", partFilename);
    }

    void JobPlan::CreateJobPlan(const _internal::JobModel& model, const std::string& jobPlanDir)
    {
      AZURE_ASSERT(!_internal::PathExists(jobPlanDir));
      _internal::CreateDirectory(jobPlanDir);
      {
        std::fstream fout(
            jobPlanDir + "/part_gens.tmp",
            std::fstream::out | std::fstream::trunc | std::fstream::binary);
        fout.exceptions(std::fstream::failbit | std::fstream::badbit);
        PartGenerator rootGenerator;
        std::string serializedGenerator = rootGenerator.ToString();
        WriteFixedInt(fout, int8_t(0));
        WriteString(fout, serializedGenerator);
        fout.close();
        _internal::Rename(jobPlanDir + "/part_gens.tmp", jobPlanDir + "/part_gens");
      }
      {
        std::fstream fout(
            jobPlanDir + "/job_info.tmp",
            std::fstream::out | std::fstream::trunc | std::fstream::binary);
        fout.exceptions(std::fstream::failbit | std::fstream::badbit);
        // TODO: progress and header length
        Core::Json::_internal::json object;
        object["source"] = Core::Json::_internal::json::parse(model.Source.ToString());
        object["destination"] = Core::Json::_internal::json::parse(model.Destination.ToString());
        std::string serializedJobInfo = object.dump();
        WriteString(fout, serializedJobInfo);
        fout.close();
        _internal::Rename(jobPlanDir + "/job_info.tmp", jobPlanDir + "/job_info");
      }
    }

    JobPlan JobPlan::LoadJobPlan(
        _internal::HydrationParameters hydrateOptions,
        const std::string& jobPlanDir)
    {
      JobPlan jobPlan;
      jobPlan.m_hydrateOptions = std::move(hydrateOptions);
      jobPlan.m_jobPlanDir = jobPlanDir;

      AZURE_ASSERT(_internal::PathExists(jobPlanDir));
      std::fstream fin(jobPlanDir + "/job_info", std::fstream::in | std::fstream::binary);
      fin.exceptions(std::fstream::failbit | std::fstream::badbit);
      auto serializedJobInfo = ReadString(fin);
      auto object = Core::Json::_internal::json::parse(serializedJobInfo);
      jobPlan.m_model.Source = _internal::TransferEnd::FromString(
          object["source"].get<std::string>(), jobPlan.m_hydrateOptions.SourceCredential);
      jobPlan.m_model.Destination = _internal::TransferEnd::FromString(
          object["destination"].get<std::string>(), jobPlan.m_hydrateOptions.DestinationCredential);

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
              jobPlan.m_jobPlanDir + "/part_gens",
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
          _internal::Rename(m_jobPlanDir + "/part_gens", m_jobPlanDir + "/part_gens.delete");
          m_hasMoreParts = false;
          m_partGens.close();
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

    void JobPlan::PartDone(uint32_t id)
    {
      auto ite = m_jobParts.find(id);
      AZURE_ASSERT(ite != m_jobParts.end());
      auto& jobPart = *ite->second;
      for (size_t i = 0; i < jobPart.m_numDoneBits; ++i)
      {
        AZURE_ASSERT(jobPart.m_doneBitmap[i]);
      }
      m_jobParts.erase(ite);
      _internal::Rename(
          m_jobPlanDir + "/" + PartIdToString(id),
          m_jobPlanDir + "/" + PartIdToString(id) + ".delete");
    }
  } // namespace _detail
}} // namespace Azure::Storage

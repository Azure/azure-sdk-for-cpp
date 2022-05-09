// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "avro_parser.hpp"

#include <algorithm>
#include <cstring>

#include <azure/core/azure_assert.hpp>
#include <azure/core/internal/json/json.hpp>

namespace Azure { namespace Storage { namespace Blobs { namespace _detail {

  namespace {
    int64_t parseInt(AvroStreamReader::ReaderPos& data)
    {
      uint64_t r = 0;
      int nb = 0;
      while (true)
      {
        uint8_t c = (*data.BufferPtr)[data.Offset++];
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

    std::string parseString(AvroStreamReader::ReaderPos& data)
    {
      const int64_t stringSize = parseInt(data);
      const uint8_t* start = &(*data.BufferPtr)[data.Offset];
      const uint8_t* end = start + stringSize;
      std::string ret(start, end);
      data.Offset += stringSize;
      return ret;
    }

    std::vector<uint8_t> parseBytes(AvroStreamReader::ReaderPos& data)
    {
      const int64_t bytesSize = parseInt(data);
      const uint8_t* start = &(*data.BufferPtr)[data.Offset];
      const uint8_t* end = start + bytesSize;
      std::vector<uint8_t> ret(start, end);
      data.Offset += bytesSize;
      return ret;
    }

    AvroSchema ParseSchemaFromJsonString(const std::string& jsonSchema)
    {
      const static std::map<std::string, AvroSchema> BuiltinNameSchemaMap = {
          {"string", AvroSchema::StringSchema},
          {"bytes", AvroSchema::BytesSchema},
          {"int", AvroSchema::IntSchema},
          {"long", AvroSchema::LongSchema},
          {"float", AvroSchema::FloatSchema},
          {"double", AvroSchema::DoubleSchema},
          {"boolean", AvroSchema::BoolSchema},
          {"null", AvroSchema::NullSchema},
          {"string", AvroSchema::StringSchema},
      };
      std::map<std::string, AvroSchema> nameSchemaMap = BuiltinNameSchemaMap;

      std::function<AvroSchema(const Core::Json::_internal::json& obj)> parseSchemaFromJsonObject;
      parseSchemaFromJsonObject = [&](const Core::Json::_internal::json& obj) -> AvroSchema {
        if (obj.is_string())
        {
          auto typeName = obj.get<std::string>();
          return nameSchemaMap.find(typeName)->second;
        }
        else if (obj.is_array())
        {
          std::vector<AvroSchema> unionSchemas;
          for (const auto& s : obj)
          {
            unionSchemas.push_back(parseSchemaFromJsonObject(s));
          }
          return AvroSchema::UnionSchema(std::move(unionSchemas));
        }
        else if (obj.is_object())
        {
          if (obj.count("namespace") != 0)
          {
            throw std::runtime_error("Namespace isn't supported yet in Avro schema.");
          }
          if (obj.count("aliases") != 0)
          {
            throw std::runtime_error("Alias isn't supported yet in Avro schema.");
          }
          auto typeName = obj["type"].get<std::string>();
          auto i = nameSchemaMap.find(typeName);
          if (i != nameSchemaMap.end())
          {
            return i->second;
          }
          if (typeName == "record")
          {
            std::vector<std::pair<std::string, AvroSchema>> fieldsSchema;
            for (const auto& field : obj["fields"])
            {
              fieldsSchema.push_back(std::make_pair(
                  field["name"].get<std::string>(), parseSchemaFromJsonObject(field["type"])));
            }

            auto recordSchema = AvroSchema::RecordSchema(std::move(fieldsSchema));
            const std::string recordName = obj["name"].get<std::string>();
            nameSchemaMap.insert(std::make_pair(recordName, recordSchema));
            return recordSchema;
          }
          else if (typeName == "enum")
          {
            throw std::runtime_error("Enum type isn't supported yet in Avro schema.");
          }
          else if (typeName == "array")
          {
            return AvroSchema::ArraySchema(parseSchemaFromJsonObject(obj["items"]));
          }
          else if (typeName == "map")
          {
            return AvroSchema::MapSchema(parseSchemaFromJsonObject(obj["items"]));
          }
          else if (typeName == "fixed")
          {
            auto fixedSchema = AvroSchema::FixedSchema(obj["size"].get<int64_t>());
            const std::string fixedName = obj["name"].get<std::string>();
            nameSchemaMap.insert(std::make_pair(fixedName, fixedSchema));
            return fixedSchema;
          }
          else
          {
            throw std::runtime_error("Unrecognized type " + typeName + " in Avro schema.");
          }
        }
        AZURE_UNREACHABLE_CODE();
      };

      auto jsonRoot = Core::Json::_internal::json::parse(jsonSchema.begin(), jsonSchema.end());
      return parseSchemaFromJsonObject(jsonRoot);
    }
  } // namespace

  int64_t AvroStreamReader::ParseInt(const Core::Context& context)
  {
    uint64_t r = 0;
    int nb = 0;
    while (true)
    {
      Preload(1, context);
      uint8_t c = m_streambuffer[m_pos.Offset++];

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

  void AvroStreamReader::Advance(size_t n, const Core::Context& context)
  {
    Preload(n, context);
    m_pos.Offset += n;
  }

  size_t AvroStreamReader::Preload(size_t n, const Core::Context& context)
  {
    size_t oldAvailable = AvailableBytes();
    while (true)
    {
      size_t newAvailable = TryPreload(n, context);
      if (newAvailable >= n)
      {
        return newAvailable;
      }
      if (oldAvailable == newAvailable)
      {
        throw std::runtime_error("Unexpected EOF of Avro stream.");
      }
      oldAvailable = newAvailable;
    }
    AZURE_UNREACHABLE_CODE();
  }

  size_t AvroStreamReader::TryPreload(size_t n, const Core::Context& context)
  {
    size_t availableBytes = AvailableBytes();
    if (availableBytes >= n)
    {
      return availableBytes;
    }
    const size_t MinRead = 4096;
    size_t tryReadSize = std::max(n, MinRead);
    size_t currSize = m_streambuffer.size();
    m_streambuffer.resize(m_streambuffer.size() + tryReadSize);
    size_t actualReadSize = m_stream->Read(m_streambuffer.data() + currSize, tryReadSize, context);
    m_streambuffer.resize(currSize + actualReadSize);
    return AvailableBytes();
  }

  void AvroStreamReader::Discard()
  {
    constexpr size_t MinimumReleaseMemory = 128 * 1024;
    if (m_pos.Offset < MinimumReleaseMemory)
    {
      return;
    }
    const size_t availableBytes = AvailableBytes();
    std::memmove(&m_streambuffer[0], &m_streambuffer[m_pos.Offset], availableBytes);
    m_streambuffer.resize(availableBytes);
    m_pos.Offset = 0;
  }

  const AvroSchema AvroSchema::StringSchema(AvroDatumType::String);
  const AvroSchema AvroSchema::BytesSchema(AvroDatumType::Bytes);
  const AvroSchema AvroSchema::IntSchema(AvroDatumType::Int);
  const AvroSchema AvroSchema::LongSchema(AvroDatumType::Long);
  const AvroSchema AvroSchema::FloatSchema(AvroDatumType::Float);
  const AvroSchema AvroSchema::DoubleSchema(AvroDatumType::Double);
  const AvroSchema AvroSchema::BoolSchema(AvroDatumType::Bool);
  const AvroSchema AvroSchema::NullSchema(AvroDatumType::Null);

  AvroSchema AvroSchema::RecordSchema(
      const std::vector<std::pair<std::string, AvroSchema>>& fieldsSchema)
  {
    AvroSchema recordSchema(AvroDatumType::Record);
    recordSchema.m_status = std::make_shared<SharedStatus>();
    for (auto& i : fieldsSchema)
    {
      recordSchema.m_status->m_keys.push_back(i.first);
      recordSchema.m_status->m_schemas.push_back(i.second);
    }
    return recordSchema;
  }

  AvroSchema AvroSchema::ArraySchema(AvroSchema elementSchema)
  {
    AvroSchema arraySchema(AvroDatumType::Array);
    arraySchema.m_status = std::make_shared<SharedStatus>();
    arraySchema.m_status->m_schemas.push_back(std::move(elementSchema));
    return arraySchema;
  }

  AvroSchema AvroSchema::MapSchema(AvroSchema elementSchema)
  {
    AvroSchema mapSchema(AvroDatumType::Map);
    mapSchema.m_status = std::make_shared<SharedStatus>();
    mapSchema.m_status->m_schemas.push_back(std::move(elementSchema));
    return mapSchema;
  }

  AvroSchema AvroSchema::UnionSchema(std::vector<AvroSchema> schemas)
  {
    AvroSchema unionSchema(AvroDatumType::Union);
    unionSchema.m_status = std::make_shared<SharedStatus>();
    unionSchema.m_status->m_schemas = std::move(schemas);
    return unionSchema;
  }

  AvroSchema AvroSchema::FixedSchema(int64_t size)
  {
    AvroSchema fixedSchema(AvroDatumType::Fixed);
    fixedSchema.m_status = std::make_shared<SharedStatus>();
    fixedSchema.m_status->m_size = size;
    return fixedSchema;
  }

  void AvroDatum::Fill(AvroStreamReader& reader, const Core::Context& context)
  {
    m_data = reader.m_pos;
    if (Type() == AvroDatumType::String || Type() == AvroDatumType::Bytes)
    {
      int64_t stringSize = reader.ParseInt(context);
      reader.Advance(stringSize, context);
    }
    else if (
        Type() == AvroDatumType::Int || Type() == AvroDatumType::Long
        || Type() == AvroDatumType::Enum)
    {
      reader.ParseInt(context);
    }
    else if (Type() == AvroDatumType::Float)
    {
      reader.Advance(4, context);
    }
    else if (Type() == AvroDatumType::Double)
    {
      reader.Advance(8, context);
    }
    else if (Type() == AvroDatumType::Bool)
    {
      reader.Advance(1, context);
    }
    else if (Type() == AvroDatumType::Null)
    {
      reader.Advance(0, context);
    }
    else if (Type() == AvroDatumType::Record)
    {
      for (const auto& s : m_schema.FieldSchemas())
      {
        AvroDatum(s).Fill(reader, context);
      }
    }
    else if (Type() == AvroDatumType::Array)
    {
      while (true)
      {
        int64_t numElementsInBlock = reader.ParseInt(context);
        if (numElementsInBlock == 0)
        {
          break;
        }
        else if (numElementsInBlock < 0)
        {
          int64_t blockSize = reader.ParseInt(context);
          reader.Advance(blockSize, context);
        }
        else
        {
          for (auto i = 0; i < numElementsInBlock; ++i)
          {
            AvroDatum(m_schema.ItemSchema()).Fill(reader, context);
          }
        }
      }
    }
    else if (Type() == AvroDatumType::Map)
    {
      while (true)
      {
        int64_t numElementsInBlock = reader.ParseInt(context);
        if (numElementsInBlock == 0)
        {
          break;
        }
        else if (numElementsInBlock < 0)
        {
          int64_t blockSize = reader.ParseInt(context);
          reader.Advance(blockSize, context);
        }
        else
        {
          for (int64_t i = 0; i < numElementsInBlock; ++i)
          {
            AvroDatum(AvroSchema::StringSchema).Fill(reader, context);
            AvroDatum(m_schema.ItemSchema()).Fill(reader, context);
          }
        }
      }
    }
    else if (Type() == AvroDatumType::Union)
    {
      int64_t i = reader.ParseInt(context);
      AvroDatum(m_schema.FieldSchemas()[i]).Fill(reader, context);
    }
    else if (Type() == AvroDatumType::Fixed)
    {
      reader.Advance(m_schema.Size(), context);
    }
    else
    {
      AZURE_UNREACHABLE_CODE();
    }
  }

  void AvroDatum::Fill(AvroStreamReader::ReaderPos& data)
  {
    m_data = data;
    if (Type() == AvroDatumType::String || Type() == AvroDatumType::Bytes)
    {
      int64_t stringSize = parseInt(data);
      data.Offset += stringSize;
    }
    else if (
        Type() == AvroDatumType::Int || Type() == AvroDatumType::Long
        || Type() == AvroDatumType::Enum)
    {
      parseInt(data);
    }
    else if (Type() == AvroDatumType::Float)
    {
      data.Offset += 4;
    }
    else if (Type() == AvroDatumType::Double)
    {
      data.Offset += 8;
    }
    else if (Type() == AvroDatumType::Bool)
    {
      data.Offset += 1;
    }
    else if (Type() == AvroDatumType::Null)
    {
      data.Offset += 0;
    }
    else if (Type() == AvroDatumType::Record)
    {
      for (const auto& s : m_schema.FieldSchemas())
      {
        AvroDatum(s).Fill(data);
      }
    }
    else if (Type() == AvroDatumType::Array)
    {
      while (true)
      {
        int64_t numElementsInBlock = parseInt(data);
        if (numElementsInBlock == 0)
        {
          break;
        }
        else if (numElementsInBlock < 0)
        {
          int64_t blockSize = parseInt(data);
          data.Offset += blockSize;
        }
        else
        {
          for (auto i = 0; i < numElementsInBlock; ++i)
          {
            AvroDatum(m_schema.ItemSchema()).Fill(data);
          }
        }
      }
    }
    else if (Type() == AvroDatumType::Map)
    {
      while (true)
      {
        int64_t numElementsInBlock = parseInt(data);
        if (numElementsInBlock == 0)
        {
          break;
        }
        else if (numElementsInBlock < 0)
        {
          int64_t blockSize = parseInt(data);
          data.Offset += blockSize;
        }
        else
        {
          for (int64_t i = 0; i < numElementsInBlock; ++i)
          {
            AvroDatum(AvroSchema::StringSchema).Fill(data);
            AvroDatum(m_schema.ItemSchema()).Fill(data);
          }
        }
      }
    }
    else if (Type() == AvroDatumType::Union)
    {
      int64_t i = parseInt(data);
      AvroDatum(m_schema.FieldSchemas()[i]).Fill(data);
    }
    else if (Type() == AvroDatumType::Fixed)
    {
      data.Offset += m_schema.Size();
    }
    else
    {
      AZURE_UNREACHABLE_CODE();
    }
  }

  template <> std::string AvroDatum::Value() const
  {
    auto stringView = Value<StringView>();
    return std::string(stringView.Data, stringView.Data + stringView.Length);
  }

  template <> std::vector<uint8_t> AvroDatum::Value() const
  {
    auto stringView = Value<StringView>();
    return std::vector<uint8_t>(stringView.Data, stringView.Data + stringView.Length);
  }

  template <> AvroDatum::StringView AvroDatum::Value() const
  {
    auto data = m_data;
    if (Type() == AvroDatumType::String || Type() == AvroDatumType::Bytes)
    {
      const int64_t length = parseInt(data);
      const uint8_t* start = &(*data.BufferPtr)[data.Offset];
      StringView ret{start, static_cast<size_t>(length)};
      data.Offset += length;
      return ret;
    }
    if (Type() == AvroDatumType::Fixed)
    {
      const size_t fixedSize = m_schema.Size();
      const uint8_t* start = &(*data.BufferPtr)[data.Offset];
      StringView ret{start, fixedSize};
      data.Offset += fixedSize;
      return ret;
    }
    AZURE_UNREACHABLE_CODE();
  }

  template <> int64_t AvroDatum::Value() const
  {
    auto data = m_data;
    return parseInt(data);
  }

  template <> AvroRecord AvroDatum::Value() const
  {
    auto data = m_data;

    AvroRecord r;
    r.m_keys = &m_schema.FieldNames();
    for (const auto& schema : m_schema.FieldSchemas())
    {
      auto datum = AvroDatum(schema);
      datum.Fill(data);
      r.m_values.push_back(std::move(datum));
    }

    return r;
  }

  template <> AvroMap AvroDatum::Value() const
  {
    auto data = m_data;

    AvroMap m;
    while (true)
    {
      int64_t numElementsInBlock = parseInt(data);
      if (numElementsInBlock == 0)
      {
        break;
      }
      if (numElementsInBlock < 0)
      {
        numElementsInBlock = -numElementsInBlock;
        parseInt(data);
      }
      for (int64_t i = 0; i < numElementsInBlock; ++i)
      {
        auto keyDatum = AvroDatum(AvroSchema::StringSchema);
        keyDatum.Fill(data);
        auto valueDatum = AvroDatum(m_schema.ItemSchema());
        valueDatum.Fill(data);
        m[keyDatum.Value<std::string>()] = valueDatum;
      }
    }
    return m;
  }

  template <> AvroDatum AvroDatum::Value() const
  {
    auto data = m_data;
    if (Type() == AvroDatumType::Union)
    {
      int64_t i = parseInt(data);
      auto datum = AvroDatum(m_schema.FieldSchemas()[i]);
      datum.Fill(data);
      return datum;
    }
    AZURE_UNREACHABLE_CODE();
  }

  AvroObjectContainerReader::AvroObjectContainerReader(Core::IO::BodyStream& stream)
      : m_reader(std::make_unique<AvroStreamReader>(stream))
  {
  }

  AvroDatum AvroObjectContainerReader::NextImpl(
      const AvroSchema* schema,
      const Core::Context& context)
  {
    AZURE_ASSERT_FALSE(m_eof);
    constexpr size_t SyncMarkerSize = 16;
    if (!schema)
    {
      static AvroSchema FileHeaderSchema = [SyncMarkerSize]() {
        std::vector<std::pair<std::string, AvroSchema>> fieldsSchema;
        fieldsSchema.push_back(std::make_pair("magic", AvroSchema::FixedSchema(4)));
        fieldsSchema.push_back(
            std::make_pair("meta", AvroSchema::MapSchema(AvroSchema::BytesSchema)));
        fieldsSchema.push_back(std::make_pair("sync", AvroSchema::FixedSchema(SyncMarkerSize)));
        return AvroSchema::RecordSchema(std::move(fieldsSchema));
      }();
      auto fileHeaderDatum = AvroDatum(FileHeaderSchema);
      fileHeaderDatum.Fill(*m_reader, context);
      auto fileHeader = fileHeaderDatum.Value<AvroRecord>();
      if (fileHeader.Field("magic").Value<std::string>() != "Obj\01")
      {
        throw std::runtime_error("Invalid Avro object container magic.");
      }
      AvroMap meta = fileHeader.Field("meta").Value<AvroMap>();
      std::string objectSchemaJson = meta["avro.schema"].Value<std::string>();
      std::string codec = "null";
      if (meta.count("avro.codec") != 0)
      {
        codec = meta["avro.codec"].Value<std::string>();
      }
      if (codec != "null")
      {
        throw std::runtime_error("Unsupported Avro codec: " + codec);
      }
      m_syncMarker = fileHeader.Field("sync").Value<std::string>();
      m_objectSchema = std::make_unique<AvroSchema>(ParseSchemaFromJsonString(objectSchemaJson));
      schema = m_objectSchema.get();
    }

    if (m_remainingObjectInCurrentBlock == 0)
    {
      m_reader->Discard();
      m_remainingObjectInCurrentBlock = m_reader->ParseInt(context);
      auto ObjectsSize = m_reader->ParseInt(context);
      m_reader->Preload(ObjectsSize, context);
    }

    auto objectDatum = AvroDatum(*m_objectSchema);
    objectDatum.Fill(*m_reader, context);
    if (--m_remainingObjectInCurrentBlock == 0)
    {
      auto markerDatum = AvroDatum(AvroSchema::FixedSchema(SyncMarkerSize));
      markerDatum.Fill(*m_reader, context);
      auto marker = markerDatum.Value<std::string>();
      if (marker != m_syncMarker)
      {
        throw std::runtime_error("Sync marker doesn't match.");
      }
      m_eof = m_reader->TryPreload(1, context) == 0;
    }
    return objectDatum;
  }

  size_t AvroStreamParser::OnRead(
      uint8_t* buffer,
      size_t count,
      Azure::Core::Context const& context)
  {
    if (m_parserBuffer.Length != 0)
    {
      size_t bytesToCopy = std::min(m_parserBuffer.Length, count);
      std::memcpy(buffer, m_parserBuffer.Data, bytesToCopy);
      m_parserBuffer.Data += bytesToCopy;
      m_parserBuffer.Length -= bytesToCopy;
      return bytesToCopy;
    }
    while (!m_parser.End())
    {
      auto datum = m_parser.Next(context);
      if (datum.Type() == AvroDatumType::Union)
      {
        datum = datum.Value<AvroDatum>();
      }
      if (datum.Type() != AvroDatumType::Record)
      {
        continue;
      }
      auto record = datum.Value<AvroRecord>();
      if (record.HasField("data"))
      {
        auto dataDatum = record.Field("data");
        m_parserBuffer = dataDatum.Value<AvroDatum::StringView>();
        return OnRead(buffer, count, context);
      }
      if (record.HasField("bytesScanned") && record.HasField("totalBytes"))
      {
        auto bytesScanned = record.Field("bytesScanned").Value<int64_t>();
        auto totalBytes = record.Field("totalBytes").Value<int64_t>();
        (void)bytesScanned;
        (void)totalBytes;
        // TODO
      }
      if (record.HasField("fatal") && record.HasField("name") && record.HasField("description")
          && record.HasField("position"))
      {
        auto fatal = record.Field("fatal").Value<bool>();
        auto name = record.Field("name").Value<std::string>();
        auto description = record.Field("description").Value<std::string>();
        auto position = record.Field("position").Value<int64_t>();
        (void)fatal;
        (void)name;
        (void)description;
        (void)position;
        // TODO
      }
    }
    return 0;
  }
}}}} // namespace Azure::Storage::Blobs::_detail
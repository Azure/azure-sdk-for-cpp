// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/storage/blobs/blob_options.hpp"

#include "private/token_credential_session_provider.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  SessionProvider::SessionProvider() = default;

  SessionProvider::~SessionProvider() = default;

  ContainerSessionProvider::ContainerSessionProvider(
      const std::string& serviceUrl,
      std::shared_ptr<const Azure::Core::Credentials::TokenCredential> credential,
      const BlobClientOptions& options)
  {
    m_provider = std::make_shared<_detail::TokenCredentialSessionProvider>(
        serviceUrl, std::move(credential), options);
  }

  ContainerSessionProvider::~ContainerSessionProvider() = default;

  const BlobAudience BlobAudience::DefaultAudience(_internal::StorageDefaultAudience);

  BlobQueryInputTextOptions BlobQueryInputTextOptions::CreateCsvTextOptions(
      const std::string& recordSeparator,
      const std::string& columnSeparator,
      const std::string& quotationCharacter,
      const std::string& escapeCharacter,
      bool hasHeaders)
  {
    BlobQueryInputTextOptions options;
    options.m_format = Models::_detail::QueryFormatType::Delimited;
    options.m_recordSeparator = recordSeparator;
    options.m_columnSeparator = columnSeparator;
    options.m_quotationCharacter = quotationCharacter;
    options.m_escapeCharacter = escapeCharacter;
    options.m_hasHeaders = hasHeaders;
    return options;
  }

  BlobQueryInputTextOptions BlobQueryInputTextOptions::CreateJsonTextOptions(
      const std::string& recordSeparator)
  {
    BlobQueryInputTextOptions options;
    options.m_format = Models::_detail::QueryFormatType::Json;
    options.m_recordSeparator = recordSeparator;
    return options;
  }

  BlobQueryInputTextOptions BlobQueryInputTextOptions::CreateParquetTextOptions()
  {
    BlobQueryInputTextOptions options;
    options.m_format = Models::_detail::QueryFormatType::Parquet;
    return options;
  }

  BlobQueryOutputTextOptions BlobQueryOutputTextOptions::CreateCsvTextOptions(
      const std::string& recordSeparator,
      const std::string& columnSeparator,
      const std::string& quotationCharacter,
      const std::string& escapeCharacter,
      bool hasHeaders)
  {
    BlobQueryOutputTextOptions options;
    options.m_format = Models::_detail::QueryFormatType::Delimited;
    options.m_recordSeparator = recordSeparator;
    options.m_columnSeparator = columnSeparator;
    options.m_quotationCharacter = quotationCharacter;
    options.m_escapeCharacter = escapeCharacter;
    options.m_hasHeaders = hasHeaders;
    return options;
  }

  BlobQueryOutputTextOptions BlobQueryOutputTextOptions::CreateJsonTextOptions(
      const std::string& recordSeparator)
  {
    BlobQueryOutputTextOptions options;
    options.m_format = Models::_detail::QueryFormatType::Json;
    options.m_recordSeparator = recordSeparator;
    return options;
  }

  BlobQueryOutputTextOptions BlobQueryOutputTextOptions::CreateArrowTextOptions(
      std::vector<Models::BlobQueryArrowField> schema)
  {
    BlobQueryOutputTextOptions options;
    options.m_format = Models::_detail::QueryFormatType::Arrow;
    options.m_schema = std::move(schema);
    return options;
  }

}}} // namespace Azure::Storage::Blobs

// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/core/base64.hpp>
#include <azure/core/exception.hpp>
#include <azure/storage/blobs/rest_client.hpp>
#include <azure/storage/common/storage_exception.hpp>

#include <limits>
#include <stdexcept>

// Parse legacy declarations before generated _detail::Models can shadow Models.
#include "generated/blobs_client.hpp"
#include "private/blobs_protocol.hpp"

namespace Azure { namespace Storage { namespace Blobs { namespace _detail {
  namespace PublicModels = Azure::Storage::Blobs::Models;
  Core::Context::Key const BlobsProtocolHeadersKey;
  namespace {
    using Pipeline = Core::Http::_internal::HttpPipeline;

    BlobsClient Client(Pipeline& pipeline, Core::Url const& url)
    {
      return BlobsClient(url, std::shared_ptr<Pipeline>(std::shared_ptr<Pipeline>{}, &pipeline));
    }

    template <class Operation> auto InvokeStorage(Operation&& operation) -> decltype(operation())
    {
      try
      {
        return operation();
      }
      catch (StorageException const&)
      {
        throw;
      }
      catch (Core::RequestFailedException& error)
      {
        if (!error.RawResponse)
        {
          throw;
        }
        throw StorageException::CreateFromResponse(std::move(error.RawResponse));
      }
    }

    template <class T> T Value(Nullable<T> const& source)
    {
      return source.HasValue() ? source.Value() : T{};
    }
    template <class T> T const& Required(Nullable<T> const& source)
    {
      if (!source.HasValue())
      {
        throw std::invalid_argument("Missing required protocol parameter.");
      }
      return source.Value();
    }
    template <class T> Nullable<T> NonEmpty(Nullable<T> const& source)
    {
      return source.HasValue() && !source.Value().empty() ? source : Nullable<T>{};
    }
    template <class T> Nullable<T> NonEmpty(T const& source)
    {
      return source.empty() ? Nullable<T>{} : Nullable<T>(source);
    }
    std::string const& Text(std::string const& source) { return source; }
    template <class T> std::string Text(T const& source) { return source.ToString(); }
    template <class T> std::string Text(Nullable<T> const& source)
    {
      return Text(Required(source));
    }
    template <class Target, class Source> Nullable<Target> Enum(Source const& source)
    {
      auto text = Text(source);
      return text.empty() ? Nullable<Target>{} : Nullable<Target>(Target(std::move(text)));
    }
    template <class Target, class Source> Nullable<Target> Enum(Nullable<Source> const& source)
    {
      return source.HasValue() ? Enum<Target>(source.Value()) : Nullable<Target>{};
    }
    Core::CaseInsensitiveMap Metadata(std::map<std::string, std::string> const& source)
    {
      return {source.begin(), source.end()};
    }
    Nullable<std::string> Encoded(Nullable<std::vector<std::uint8_t>> const& source)
    {
      return NonEmpty(source).HasValue()
          ? Nullable<std::string>(Core::Convert::Base64Encode(source.Value()))
          : Nullable<std::string>{};
    }

    Models::RetentionPolicy ToGenerated(PublicModels::RetentionPolicy const& source)
    {
      Models::RetentionPolicy result;
      result.Enabled = source.IsEnabled;
      result.Days = source.Days;
      return result;
    }
    PublicModels::RetentionPolicy FromGenerated(Models::RetentionPolicy const& source)
    {
      PublicModels::RetentionPolicy result;
      result.IsEnabled = source.Enabled;
      result.Days = source.Days;
      return result;
    }
    Models::Metrics ToGenerated(PublicModels::Metrics const& source)
    {
      Models::Metrics result;
      result.Version = source.Version;
      result.Enabled = source.IsEnabled;
      result.IncludeApis = source.IncludeApis;
      result.RetentionPolicy = ToGenerated(source.RetentionPolicy);
      return result;
    }
    PublicModels::Metrics FromGenerated(Models::Metrics const& source)
    {
      PublicModels::Metrics result;
      result.Version = Value(source.Version);
      result.IsEnabled = source.Enabled;
      result.IncludeApis = source.IncludeApis;
      if (source.RetentionPolicy.HasValue())
      {
        result.RetentionPolicy = FromGenerated(source.RetentionPolicy.Value());
      }
      return result;
    }
    Models::BlobServiceProperties ToGenerated(PublicModels::BlobServiceProperties const& source)
    {
      Models::BlobServiceProperties result;
      Models::Logging logging;
      logging.Version = source.Logging.Version;
      logging.Delete = source.Logging.Delete;
      logging.Read = source.Logging.Read;
      logging.Write = source.Logging.Write;
      logging.RetentionPolicy = ToGenerated(source.Logging.RetentionPolicy);
      result.Logging = std::move(logging);
      result.HourMetrics = ToGenerated(source.HourMetrics);
      result.MinuteMetrics = ToGenerated(source.MinuteMetrics);
      std::vector<Models::CorsRule> cors;
      for (auto const& item : source.Cors)
      {
        Models::CorsRule rule;
        rule.AllowedOrigins = item.AllowedOrigins;
        rule.AllowedMethods = item.AllowedMethods;
        rule.AllowedHeaders = item.AllowedHeaders;
        rule.ExposedHeaders = item.ExposedHeaders;
        rule.MaxAgeInSeconds = item.MaxAgeInSeconds;
        cors.push_back(std::move(rule));
      }
      result.Cors = std::move(cors);
      result.DefaultServiceVersion = source.DefaultServiceVersion;
      result.DeleteRetentionPolicy = ToGenerated(source.DeleteRetentionPolicy);
      Models::StaticWebsite website;
      website.Enabled = source.StaticWebsite.IsEnabled;
      website.IndexDocument = source.StaticWebsite.IndexDocument;
      website.ErrorDocument404Path = source.StaticWebsite.ErrorDocument404Path;
      website.DefaultIndexDocumentPath = source.StaticWebsite.DefaultIndexDocumentPath;
      result.StaticWebsite = std::move(website);
      return result;
    }
    PublicModels::BlobServiceProperties FromGenerated(
        Models::GetPropertiesServiceResult const& source)
    {
      PublicModels::BlobServiceProperties result;
      if (source.Logging.HasValue())
      {
        auto const& logging = source.Logging.Value();
        result.Logging.Version = logging.Version;
        result.Logging.Delete = logging.Delete;
        result.Logging.Read = logging.Read;
        result.Logging.Write = logging.Write;
        result.Logging.RetentionPolicy = FromGenerated(logging.RetentionPolicy);
      }
      if (source.HourMetrics.HasValue())
      {
        result.HourMetrics = FromGenerated(source.HourMetrics.Value());
      }
      if (source.MinuteMetrics.HasValue())
      {
        result.MinuteMetrics = FromGenerated(source.MinuteMetrics.Value());
      }
      if (source.Cors.HasValue())
      {
        for (auto const& rule : source.Cors.Value())
        {
          PublicModels::CorsRule item;
          item.AllowedOrigins = rule.AllowedOrigins;
          item.AllowedMethods = rule.AllowedMethods;
          item.AllowedHeaders = rule.AllowedHeaders;
          item.ExposedHeaders = rule.ExposedHeaders;
          item.MaxAgeInSeconds = rule.MaxAgeInSeconds;
          result.Cors.push_back(std::move(item));
        }
      }
      result.DefaultServiceVersion = source.DefaultServiceVersion;
      if (source.DeleteRetentionPolicy.HasValue())
      {
        result.DeleteRetentionPolicy = FromGenerated(source.DeleteRetentionPolicy.Value());
      }
      if (source.StaticWebsite.HasValue())
      {
        auto const& website = source.StaticWebsite.Value();
        result.StaticWebsite.IsEnabled = website.Enabled;
        result.StaticWebsite.IndexDocument = website.IndexDocument;
        result.StaticWebsite.ErrorDocument404Path = website.ErrorDocument404Path;
        result.StaticWebsite.DefaultIndexDocumentPath = website.DefaultIndexDocumentPath;
      }
      return result;
    }
    PublicModels::ServiceStatistics FromGenerated(Models::GetStatisticsServiceResult const& source)
    {
      PublicModels::ServiceStatistics result;
      if (source.GeoReplication.HasValue())
      {
        auto const& geo = source.GeoReplication.Value();
        result.GeoReplication.Status = PublicModels::GeoReplicationStatus(geo.Status.ToString());
        if (!geo.LastSyncTime.empty())
        {
          result.GeoReplication.LastSyncedOn
              = DateTime::Parse(geo.LastSyncTime, DateTime::DateFormat::Rfc1123);
        }
      }
      return result;
    }
    PublicModels::_detail::ListBlobContainersResult FromGenerated(
        Models::ListContainersServiceResult const& source)
    {
      PublicModels::_detail::ListBlobContainersResult result;
      result.ServiceEndpoint = source.ServiceEndpoint;
      result.Prefix = Value(source.Prefix);
      result.ContinuationToken = source.NextMarker;
      for (auto const& container : source.ContainerItems)
      {
        PublicModels::BlobContainerItem item;
        item.Name = container.Name;
        item.IsDeleted = Value(container.Deleted);
        item.VersionId = container.Version;
        auto const& properties = container.Properties;
        auto& details = item.Details;
        details.ETag = properties.ETag;
        details.LastModified = properties.LastModified;
        if (properties.LeaseStatus.HasValue())
        {
          details.LeaseStatus
              = PublicModels::LeaseStatus(properties.LeaseStatus.Value().ToString());
        }
        if (properties.LeaseState.HasValue())
        {
          details.LeaseState = PublicModels::LeaseState(properties.LeaseState.Value().ToString());
        }
        details.LeaseDuration = Enum<PublicModels::LeaseDurationType>(properties.LeaseDuration);
        if (properties.PublicAccess.HasValue())
        {
          details.AccessType
              = PublicModels::PublicAccessType(properties.PublicAccess.Value().ToString());
        }
        details.HasImmutabilityPolicy = Value(properties.HasImmutabilityPolicy);
        details.HasLegalHold = Value(properties.HasLegalHold);
        if (properties.DefaultEncryptionScope.HasValue())
        {
          details.DefaultEncryptionScope = properties.DefaultEncryptionScope.Value();
        }
        details.PreventEncryptionScopeOverride = Value(properties.PreventEncryptionScopeOverride);
        details.DeletedOn = properties.DeletedTime;
        details.RemainingRetentionDays = properties.RemainingRetentionDays;
        details.HasImmutableStorageWithVersioning
            = Value(properties.IsImmutableStorageWithVersioningEnabled);
        if (container.Metadata.HasValue())
        {
          details.Metadata = Metadata(container.Metadata.Value());
        }
        result.Items.push_back(std::move(item));
      }
      return result;
    }
    Models::KeyInfo ToGenerated(PublicModels::_detail::KeyInfo const& source)
    {
      Models::KeyInfo result;
      result.Start = source.Start;
      result.Expiry = source.Expiry;
      result.DelegatedUserTid = source.DelegatedUserTid;
      return result;
    }
    PublicModels::UserDelegationKey FromGenerated(
        Models::GetUserDelegationKeyServiceResult const& source)
    {
      PublicModels::UserDelegationKey result;
      result.SignedObjectId = source.SignedOid;
      result.SignedTenantId = source.SignedTid;
      result.SignedStartsOn = source.SignedStart;
      result.SignedExpiresOn = source.SignedExpiry;
      result.SignedService = source.SignedService;
      result.SignedVersion = source.SignedVersion;
      result.SignedDelegatedUserTid = source.SignedDelegatedUserTid;
      result.Value = source.Value;
      return result;
    }
    Models::BlobTags ToGenerated(std::map<std::string, std::string> const& source)
    {
      Models::BlobTags result;
      for (auto const& item : source)
      {
        Models::BlobTag tag;
        tag.Key = item.first;
        tag.Value = item.second;
        result.BlobTagSet.push_back(std::move(tag));
      }
      return result;
    }
    std::map<std::string, std::string> Tags(Models::BlobTags const& source)
    {
      std::map<std::string, std::string> result;
      for (auto const& tag : source.BlobTagSet)
      {
        result[tag.Key] = tag.Value;
      }
      return result;
    }
    template <class T> PublicModels::_detail::FindBlobsByTagsResult FindBlobs(T const& source)
    {
      PublicModels::_detail::FindBlobsByTagsResult result;
      result.ServiceEndpoint = source.ServiceEndpoint;
      result.ContinuationToken = source.NextMarker;
      for (auto const& blob : source.BlobItems)
      {
        PublicModels::TaggedBlobItem item;
        item.BlobName = blob.Name;
        item.BlobContainerName = blob.ContainerName;
        if (blob.Tags.HasValue())
        {
          item.Tags = Tags(blob.Tags.Value());
        }
        result.Items.push_back(std::move(item));
      }
      return result;
    }
    Models::SignedIdentifiers ToGenerated(std::vector<PublicModels::SignedIdentifier> const& source)
    {
      Models::SignedIdentifiers result;
      for (auto const& item : source)
      {
        Models::SignedIdentifier identifier;
        identifier.Id = item.Id;
        Models::AccessPolicy policy;
        policy.Start = item.StartsOn;
        policy.Expiry = item.ExpiresOn;
        policy.Permission = NonEmpty(item.Permissions);
        identifier.AccessPolicy = std::move(policy);
        result.Items.push_back(std::move(identifier));
      }
      return result;
    }
    PublicModels::BlobContainerAccessPolicy FromGenerated(
        Models::GetAccessPolicyContainerResult const& source)
    {
      PublicModels::BlobContainerAccessPolicy result;
      for (auto const& item : source.Items)
      {
        PublicModels::SignedIdentifier identifier;
        identifier.Id = item.Id;
        if (item.AccessPolicy.HasValue())
        {
          identifier.StartsOn = item.AccessPolicy.Value().Start;
          identifier.ExpiresOn = item.AccessPolicy.Value().Expiry;
          identifier.Permissions = Value(item.AccessPolicy.Value().Permission);
        }
        result.SignedIdentifiers.push_back(std::move(identifier));
      }
      return result;
    }
    std::map<std::string, std::string> FromGenerated(Models::GetTagsBlobResult const& source)
    {
      std::map<std::string, std::string> result;
      for (auto const& tag : source.BlobTagSet)
      {
        result[tag.Key] = tag.Value;
      }
      return result;
    }
    Models::BlockLookupList ToGenerated(PublicModels::_detail::BlockLookupList const& source)
    {
      Models::BlockLookupList result;
      result.Committed = NonEmpty(source.Committed);
      result.Uncommitted = NonEmpty(source.Uncommitted);
      result.Latest = NonEmpty(source.Latest);
      return result;
    }
    PublicModels::GetBlockListResult FromGenerated(
        Models::GetBlockListBlockBlobResult const& source)
    {
      PublicModels::GetBlockListResult result;
      if (source.CommittedBlocks.HasValue())
      {
        for (auto const& block : source.CommittedBlocks.Value())
        {
          PublicModels::BlobBlock item;
          item.Name = block.Name;
          item.Size = block.Size;
          result.CommittedBlocks.push_back(std::move(item));
        }
      }
      if (source.UncommittedBlocks.HasValue())
      {
        for (auto const& block : source.UncommittedBlocks.Value())
        {
          PublicModels::BlobBlock item;
          item.Name = block.Name;
          item.Size = block.Size;
          result.UncommittedBlocks.push_back(std::move(item));
        }
      }
      return result;
    }
    template <class Target, class T> Target PageRanges(T const& source)
    {
      Target result;
      auto append = [](auto const& ranges, std::vector<Core::Http::HttpRange>& target) {
        if (ranges.HasValue())
        {
          for (auto const& sourceRange : ranges.Value())
          {
            if (sourceRange.Start < 0 || sourceRange.End < sourceRange.Start
                || sourceRange.End - sourceRange.Start
                    == (std::numeric_limits<std::int64_t>::max)())
            {
              throw std::out_of_range("Invalid page range.");
            }
            Core::Http::HttpRange range;
            range.Offset = sourceRange.Start;
            range.Length = sourceRange.End - sourceRange.Start + 1;
            target.push_back(range);
          }
        }
      };
      append(source.PageRange, result.PageRanges);
      append(source.ClearRange, result.ClearRanges);
      result.ContinuationToken = Value(source.NextMarker);
      return result;
    }
    Models::QuerySerialization ToGenerated(PublicModels::_detail::QuerySerialization const& source)
    {
      Models::QuerySerialization result;
      auto const& format = source.Format;
      result.Format.Type = Models::QueryType(format.Type.ToString());
      if (format.DelimitedTextConfiguration.HasValue())
      {
        auto const& text = format.DelimitedTextConfiguration.Value();
        Models::DelimitedTextConfiguration config;
        config.ColumnSeparator = text.ColumnSeparator;
        config.FieldQuote = text.FieldQuote;
        config.RecordSeparator = text.RecordSeparator;
        config.EscapeChar = text.EscapeChar;
        config.HeadersPresent = text.HeadersPresent;
        result.Format.DelimitedTextConfiguration = std::move(config);
      }
      if (format.JsonTextConfiguration.HasValue())
      {
        Models::JsonTextConfiguration config;
        config.RecordSeparator = format.JsonTextConfiguration.Value().RecordSeparator;
        result.Format.JsonTextConfiguration = std::move(config);
      }
      if (format.ArrowConfiguration.HasValue())
      {
        Models::ArrowConfiguration config;
        for (auto const& field : format.ArrowConfiguration.Value().Schema)
        {
          Models::ArrowField item;
          item.Type = field.Type.ToString();
          item.Name = field.Name;
          item.Precision = field.Precision;
          item.Scale = field.Scale;
          config.Schema.push_back(std::move(item));
        }
        result.Format.ArrowConfiguration = std::move(config);
      }
      if (format.ParquetTextConfiguration.HasValue())
      {
        result.Format.ParquetTextConfiguration = Models::StorageCppParquetConfiguration{};
      }
      return result;
    }
    Models::QueryRequest ToGenerated(PublicModels::_detail::QueryRequest const& source)
    {
      Models::QueryRequest result;
      result.QueryType = Models::QueryRequestType(source.QueryType.ToString());
      result.Expression = source.Expression;
      if (source.InputSerialization.HasValue())
      {
        result.InputSerialization = ToGenerated(source.InputSerialization.Value());
      }
      if (source.OutputSerialization.HasValue())
      {
        result.OutputSerialization = ToGenerated(source.OutputSerialization.Value());
      }
      return result;
    }
    std::string ListBlobContainersIncludeFlagsToString(
        const Azure::Storage::Blobs::Models::ListBlobContainersIncludeFlags& val)
    {
      const Azure::Storage::Blobs::Models::ListBlobContainersIncludeFlags valueList[] = {
          Azure::Storage::Blobs::Models::ListBlobContainersIncludeFlags::Metadata,
          Azure::Storage::Blobs::Models::ListBlobContainersIncludeFlags::Deleted,
          Azure::Storage::Blobs::Models::ListBlobContainersIncludeFlags::System,
      };
      const char* stringList[] = {
          "metadata",
          "deleted",
          "system",
      };
      std::string ret;
      for (size_t i = 0; i < 3; ++i)
      {
        if ((val & valueList[i]) == valueList[i])
        {
          if (!ret.empty())
          {
            ret += ",";
          }
          ret += stringList[i];
        }
      }
      return ret;
    }
    std::string ListBlobsIncludeFlagsToString(
        const Azure::Storage::Blobs::Models::ListBlobsIncludeFlags& val)
    {
      const Azure::Storage::Blobs::Models::ListBlobsIncludeFlags valueList[] = {
          Azure::Storage::Blobs::Models::ListBlobsIncludeFlags::Copy,
          Azure::Storage::Blobs::Models::ListBlobsIncludeFlags::Deleted,
          Azure::Storage::Blobs::Models::ListBlobsIncludeFlags::Metadata,
          Azure::Storage::Blobs::Models::ListBlobsIncludeFlags::Snapshots,
          Azure::Storage::Blobs::Models::ListBlobsIncludeFlags::UncomittedBlobs,
          Azure::Storage::Blobs::Models::ListBlobsIncludeFlags::Versions,
          Azure::Storage::Blobs::Models::ListBlobsIncludeFlags::Tags,
          Azure::Storage::Blobs::Models::ListBlobsIncludeFlags::ImmutabilityPolicy,
          Azure::Storage::Blobs::Models::ListBlobsIncludeFlags::LegalHold,
          Azure::Storage::Blobs::Models::ListBlobsIncludeFlags::DeletedWithVersions,
      };
      const char* stringList[] = {
          "copy",
          "deleted",
          "metadata",
          "snapshots",
          "uncommittedblobs",
          "versions",
          "tags",
          "immutabilitypolicy",
          "legalhold",
          "deletedwithversions",
      };
      std::string ret;
      for (size_t i = 0; i < 10; ++i)
      {
        if ((val & valueList[i]) == valueList[i])
        {
          if (!ret.empty())
          {
            ret += ",";
          }
          ret += stringList[i];
        }
      }
      return ret;
    }

    std::string FlagsText(PublicModels::ListBlobContainersIncludeFlags value)
    {
      return ListBlobContainersIncludeFlagsToString(value);
    }
    std::string FlagsText(PublicModels::ListBlobsIncludeFlags value)
    {
      return ListBlobsIncludeFlagsToString(value);
    }
    template <class Target, class Flags>
    Nullable<std::vector<Target>> Include(Nullable<Flags> const& source)
    {
      if (!source.HasValue())
      {
        return {};
      }
      auto text = FlagsText(source.Value());
      if (text.empty())
      {
        return {};
      }
      std::vector<Target> result;
      std::size_t start = 0;
      while (start < text.size())
      {
        auto comma = text.find(',', start);
        result.emplace_back(text.substr(start, comma == std::string::npos ? comma : comma - start));
        if (comma == std::string::npos)
        {
          break;
        }
        start = comma + 1;
      }
      return result;
    }
  } // namespace

  Response<PublicModels::SetServicePropertiesResult> ServiceClient::SetProperties(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const SetServicePropertiesOptions& options,
      const Core::Context& context)
  {
    BlobsClientSetPropertiesServiceOptions generatedOptions;
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .SetPropertiesService(
              ToGenerated(options.BlobServiceProperties), generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Accepted)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::SetServicePropertiesResult response;
    return Response<PublicModels::SetServicePropertiesResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::BlobServiceProperties> ServiceClient::GetProperties(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const GetServicePropertiesOptions&,
      const Core::Context& context)
  {
    BlobsClientGetPropertiesServiceOptions generatedOptions;
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url).GetPropertiesService(generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    auto response = FromGenerated(wire.Value);
    return Response<PublicModels::BlobServiceProperties>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::ServiceStatistics> ServiceClient::GetStatistics(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const GetServiceStatisticsOptions&,
      const Core::Context& context)
  {
    BlobsClientGetStatisticsServiceOptions generatedOptions;
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url).GetStatisticsService(generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    auto response = FromGenerated(wire.Value);
    return Response<PublicModels::ServiceStatistics>(std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::_detail::ListBlobContainersResult> ServiceClient::ListBlobContainers(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const ListServiceBlobContainersOptions& options,
      const Core::Context& context)
  {
    BlobsClientListContainersServiceOptions generatedOptions;
    generatedOptions.Prefix = NonEmpty(options.Prefix);
    generatedOptions.Marker = NonEmpty(options.Marker);
    generatedOptions.Maxresults = options.MaxResults;
    generatedOptions.Include = Include<Models::ListContainersIncludeType>(options.Include);
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url).ListContainersService(generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    auto response = FromGenerated(wire.Value);
    return Response<PublicModels::_detail::ListBlobContainersResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::UserDelegationKey> ServiceClient::GetUserDelegationKey(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const GetServiceUserDelegationKeyOptions& options,
      const Core::Context& context)
  {
    BlobsClientGetUserDelegationKeyServiceOptions generatedOptions;
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .GetUserDelegationKeyService(
              ToGenerated(options.KeyInfo), generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    auto response = FromGenerated(wire.Value);
    return Response<PublicModels::UserDelegationKey>(std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::AccountInfo> ServiceClient::GetAccountInfo(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const GetServiceAccountInfoOptions&,
      const Core::Context& context)
  {
    BlobsClientGetAccountInfoServiceOptions generatedOptions;
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url).GetAccountInfoService(generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::AccountInfo response;
    response.SkuName = PublicModels::SkuName(pRawResponse->GetHeaders().at("x-ms-sku-name"));
    response.AccountKind
        = PublicModels::AccountKind(pRawResponse->GetHeaders().at("x-ms-account-kind"));
    response.IsHierarchicalNamespaceEnabled
        = pRawResponse->GetHeaders().at("x-ms-is-hns-enabled") == std::string("true");
    return Response<PublicModels::AccountInfo>(std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::_detail::SubmitBatchResult> ServiceClient::SubmitBatch(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      Core::IO::BodyStream& requestBody,
      const SubmitServiceBatchOptions& options,
      const Core::Context& context)
  {
    auto request = Core::Http::Request(Core::Http::HttpMethod::Post, url, &requestBody, false);
    request.GetUrl().AppendQueryParameter("comp", "batch");
    request.SetHeader("Content-Length", std::to_string(requestBody.Length()));
    if (!options.MultipartContentType.empty())
    {
      request.SetHeader("Content-Type", options.MultipartContentType);
    }
    request.SetHeader("x-ms-version", "2026-10-06");
    auto pRawResponse = pipeline.Send(request, context);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Accepted)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::_detail::SubmitBatchResult response;
    response.BodyStream = pRawResponse->ExtractBodyStream();
    response.ContentType = pRawResponse->GetHeaders().at("Content-Type");
    return Response<PublicModels::_detail::SubmitBatchResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::_detail::FindBlobsByTagsResult> ServiceClient::FindBlobsByTags(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const FindServiceBlobsByTagsOptions& options,
      const Core::Context& context)
  {
    BlobsClientFindBlobsByTagsServiceOptions generatedOptions;
    generatedOptions.Marker = NonEmpty(options.Marker);
    generatedOptions.Maxresults = options.MaxResults;
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .FindBlobsByTagsService(Required(options.Where), generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    auto response = FindBlobs(wire.Value);
    return Response<PublicModels::_detail::FindBlobsByTagsResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::CreateBlobContainerResult> BlobContainerClient::Create(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const CreateBlobContainerOptions& options,
      const Core::Context& context)
  {
    BlobsClientCreateContainerOptions generatedOptions;
    generatedOptions.Metadata = Metadata(options.Metadata);
    generatedOptions.XMsBlobPublicAccess = Enum<Models::PublicAccessType>(options.Access);
    generatedOptions.XMsDefaultEncryptionScope = NonEmpty(options.DefaultEncryptionScope);
    generatedOptions.XMsDenyEncryptionScopeOverride = options.PreventEncryptionScopeOverride;
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url).CreateContainer(generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Created)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::CreateBlobContainerResult response;
    response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    response.LastModified = DateTime::Parse(
        pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    return Response<PublicModels::CreateBlobContainerResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::BlobContainerProperties> BlobContainerClient::GetProperties(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const GetBlobContainerPropertiesOptions& options,
      const Core::Context& context)
  {
    BlobsClientGetPropertiesContainerOptions generatedOptions;
    generatedOptions.XMsLeaseId = NonEmpty(options.LeaseId);
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url).GetPropertiesContainer(generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::BlobContainerProperties response;
    for (auto i = pRawResponse->GetHeaders().lower_bound("x-ms-meta-");
         i != pRawResponse->GetHeaders().end() && i->first.substr(0, 10) == "x-ms-meta-";
         ++i)
    {
      response.Metadata.emplace(i->first.substr(10), i->second);
    }
    response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    response.LastModified = DateTime::Parse(
        pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    if (pRawResponse->GetHeaders().count("x-ms-lease-duration") != 0)
    {
      response.LeaseDuration
          = PublicModels::LeaseDurationType(pRawResponse->GetHeaders().at("x-ms-lease-duration"));
    }
    response.LeaseState
        = PublicModels::LeaseState(pRawResponse->GetHeaders().at("x-ms-lease-state"));
    response.LeaseStatus
        = PublicModels::LeaseStatus(pRawResponse->GetHeaders().at("x-ms-lease-status"));
    if (pRawResponse->GetHeaders().count("x-ms-blob-public-access") != 0)
    {
      response.AccessType = PublicModels::PublicAccessType(
          pRawResponse->GetHeaders().at("x-ms-blob-public-access"));
    }
    response.HasImmutabilityPolicy
        = pRawResponse->GetHeaders().at("x-ms-has-immutability-policy") == std::string("true");
    response.HasLegalHold
        = pRawResponse->GetHeaders().at("x-ms-has-legal-hold") == std::string("true");
    if (pRawResponse->GetHeaders().count("x-ms-default-encryption-scope") != 0)
    {
      response.DefaultEncryptionScope
          = pRawResponse->GetHeaders().at("x-ms-default-encryption-scope");
    }
    if (pRawResponse->GetHeaders().count("x-ms-deny-encryption-scope-override") != 0)
    {
      response.PreventEncryptionScopeOverride
          = pRawResponse->GetHeaders().at("x-ms-deny-encryption-scope-override")
          == std::string("true");
    }
    if (pRawResponse->GetHeaders().count("x-ms-immutable-storage-with-versioning-enabled") != 0)
    {
      response.HasImmutableStorageWithVersioning
          = pRawResponse->GetHeaders().at("x-ms-immutable-storage-with-versioning-enabled")
          == std::string("true");
    }
    return Response<PublicModels::BlobContainerProperties>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::DeleteBlobContainerResult> BlobContainerClient::Delete(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const DeleteBlobContainerOptions& options,
      const Core::Context& context)
  {
    BlobsClientDeleteContainerOptions generatedOptions;
    generatedOptions.XMsLeaseId = NonEmpty(options.LeaseId);
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url).DeleteContainer(generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Accepted)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::DeleteBlobContainerResult response;
    return Response<PublicModels::DeleteBlobContainerResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::SetBlobContainerMetadataResult> BlobContainerClient::SetMetadata(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const SetBlobContainerMetadataOptions& options,
      const Core::Context& context)
  {
    BlobsClientSetMetadataContainerOptions generatedOptions;
    generatedOptions.XMsLeaseId = NonEmpty(options.LeaseId);
    generatedOptions.Metadata = Metadata(options.Metadata);
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url).SetMetadataContainer(generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::SetBlobContainerMetadataResult response;
    response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    response.LastModified = DateTime::Parse(
        pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    return Response<PublicModels::SetBlobContainerMetadataResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::BlobContainerAccessPolicy> BlobContainerClient::GetAccessPolicy(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const GetBlobContainerAccessPolicyOptions& options,
      const Core::Context& context)
  {
    BlobsClientGetAccessPolicyContainerOptions generatedOptions;
    generatedOptions.XMsLeaseId = NonEmpty(options.LeaseId);
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url).GetAccessPolicyContainer(generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    auto response = FromGenerated(wire.Value);
    if (pRawResponse->GetHeaders().count("x-ms-blob-public-access") != 0)
    {
      response.AccessType = PublicModels::PublicAccessType(
          pRawResponse->GetHeaders().at("x-ms-blob-public-access"));
    }
    return Response<PublicModels::BlobContainerAccessPolicy>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::SetBlobContainerAccessPolicyResult> BlobContainerClient::SetAccessPolicy(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const SetBlobContainerAccessPolicyOptions& options,
      const Core::Context& context)
  {
    BlobsClientSetAccessPolicyContainerOptions generatedOptions;
    generatedOptions.XMsLeaseId = NonEmpty(options.LeaseId);
    generatedOptions.XMsBlobPublicAccess = Enum<Models::PublicAccessType>(options.Access);
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    generatedOptions.ContainerAcl = ToGenerated(options.ContainerAcl);
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url).SetAccessPolicyContainer(generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::SetBlobContainerAccessPolicyResult response;
    response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    response.LastModified = DateTime::Parse(
        pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    return Response<PublicModels::SetBlobContainerAccessPolicyResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::_detail::UndeleteBlobContainerResult> BlobContainerClient::Undelete(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const UndeleteBlobContainerOptions& options,
      const Core::Context& context)
  {
    BlobsClientRestoreContainerOptions generatedOptions;
    generatedOptions.XMsDeletedContainerName = NonEmpty(options.DeletedContainerName);
    generatedOptions.XMsDeletedContainerVersion = NonEmpty(options.DeletedContainerVersion);
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url).RestoreContainer(generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Created)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::_detail::UndeleteBlobContainerResult response;
    return Response<PublicModels::_detail::UndeleteBlobContainerResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::_detail::RenameBlobContainerResult> BlobContainerClient::Rename(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const RenameBlobContainerOptions& options,
      const Core::Context& context)
  {
    BlobsClientRenameContainerOptions generatedOptions;
    generatedOptions.XMsSourceLeaseId = NonEmpty(options.SourceLeaseId);
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .RenameContainer(options.SourceContainerName, generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::_detail::RenameBlobContainerResult response;
    return Response<PublicModels::_detail::RenameBlobContainerResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::_detail::SubmitBatchResult> BlobContainerClient::SubmitBatch(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      Core::IO::BodyStream& requestBody,
      const SubmitBlobContainerBatchOptions& options,
      const Core::Context& context)
  {
    auto request = Core::Http::Request(Core::Http::HttpMethod::Post, url, &requestBody, false);
    request.GetUrl().AppendQueryParameter("restype", "container");
    request.GetUrl().AppendQueryParameter("comp", "batch");
    request.SetHeader("Content-Length", std::to_string(requestBody.Length()));
    if (!options.MultipartContentType.empty())
    {
      request.SetHeader("Content-Type", options.MultipartContentType);
    }
    request.SetHeader("x-ms-version", "2026-10-06");
    auto pRawResponse = pipeline.Send(request, context);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Accepted)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::_detail::SubmitBatchResult response;
    response.BodyStream = pRawResponse->ExtractBodyStream();
    response.ContentType = pRawResponse->GetHeaders().at("Content-Type");
    return Response<PublicModels::_detail::SubmitBatchResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::_detail::FindBlobsByTagsResult> BlobContainerClient::FindBlobsByTags(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const FindBlobContainerBlobsByTagsOptions& options,
      const Core::Context& context)
  {
    BlobsClientFindBlobsByTagsContainerOptions generatedOptions;
    generatedOptions.Marker = NonEmpty(options.Marker);
    generatedOptions.Maxresults = options.MaxResults;
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .FindBlobsByTagsContainer(Required(options.Where), generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    auto response = FindBlobs(wire.Value);
    return Response<PublicModels::_detail::FindBlobsByTagsResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::_detail::AcquireBlobContainerLeaseResult>
  BlobContainerClient::AcquireLease(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const AcquireBlobContainerLeaseOptions& options,
      const Core::Context& context)
  {
    BlobsClientAcquireLeaseContainerOptions generatedOptions;
    generatedOptions.XMsProposedLeaseId = NonEmpty(options.ProposedLeaseId);
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .AcquireLeaseContainer(Required(options.Duration), generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Created)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::_detail::AcquireBlobContainerLeaseResult response;
    response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    response.LastModified = DateTime::Parse(
        pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    response.LeaseId = pRawResponse->GetHeaders().at("x-ms-lease-id");
    return Response<PublicModels::_detail::AcquireBlobContainerLeaseResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::_detail::ReleaseBlobContainerLeaseResult>
  BlobContainerClient::ReleaseLease(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const ReleaseBlobContainerLeaseOptions& options,
      const Core::Context& context)
  {
    BlobsClientReleaseLeaseContainerOptions generatedOptions;
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .ReleaseLeaseContainer(options.LeaseId, generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::_detail::ReleaseBlobContainerLeaseResult response;
    response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    response.LastModified = DateTime::Parse(
        pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    return Response<PublicModels::_detail::ReleaseBlobContainerLeaseResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::_detail::RenewBlobContainerLeaseResult> BlobContainerClient::RenewLease(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const RenewBlobContainerLeaseOptions& options,
      const Core::Context& context)
  {
    BlobsClientRenewLeaseContainerOptions generatedOptions;
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .RenewLeaseContainer(options.LeaseId, generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::_detail::RenewBlobContainerLeaseResult response;
    response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    response.LastModified = DateTime::Parse(
        pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    response.LeaseId = pRawResponse->GetHeaders().at("x-ms-lease-id");
    return Response<PublicModels::_detail::RenewBlobContainerLeaseResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::_detail::BreakBlobContainerLeaseResult> BlobContainerClient::BreakLease(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const BreakBlobContainerLeaseOptions& options,
      const Core::Context& context)
  {
    BlobsClientBreakLeaseContainerOptions generatedOptions;
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    generatedOptions.XMsLeaseBreakPeriod = options.BreakPeriod;
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url).BreakLeaseContainer(generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Accepted)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::_detail::BreakBlobContainerLeaseResult response;
    response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    response.LastModified = DateTime::Parse(
        pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    response.LeaseTime = std::stoi(pRawResponse->GetHeaders().at("x-ms-lease-time"));
    return Response<PublicModels::_detail::BreakBlobContainerLeaseResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::_detail::ChangeBlobContainerLeaseResult> BlobContainerClient::ChangeLease(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const ChangeBlobContainerLeaseOptions& options,
      const Core::Context& context)
  {
    BlobsClientChangeLeaseContainerOptions generatedOptions;
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .ChangeLeaseContainer(
              options.LeaseId, options.ProposedLeaseId, generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::_detail::ChangeBlobContainerLeaseResult response;
    response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    response.LastModified = DateTime::Parse(
        pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    response.LeaseId = pRawResponse->GetHeaders().at("x-ms-lease-id");
    return Response<PublicModels::_detail::ChangeBlobContainerLeaseResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::_detail::ListBlobsResult> BlobContainerClient::ListBlobs(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const ListBlobContainerBlobsOptions& options,
      const Core::Context& context)
  {
    BlobsClientListBlobsRawOptions generatedOptions;
    generatedOptions.Prefix = NonEmpty(options.Prefix);
    generatedOptions.Marker = NonEmpty(options.Marker);
    generatedOptions.Maxresults = options.MaxResults;
    generatedOptions.Include = Include<Models::ListBlobsIncludeItem>(options.Include);
    generatedOptions.StartFrom = NonEmpty(options.StartFrom);
    generatedOptions.EndBefore = NonEmpty(options.EndBefore);
    auto generatedContext = context;
    Core::CaseInsensitiveMap additionalHeaders;
    if (NonEmpty(options.Accept).HasValue())
    {
      additionalHeaders["Accept"] = options.Accept.Value();
    }
    generatedContext = generatedContext.WithValue(BlobsProtocolHeadersKey, additionalHeaders);
    auto wire = InvokeStorage(
        [&]() { return Client(pipeline, url).ListBlobsRaw(generatedOptions, generatedContext); });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::_detail::ListBlobsResult response;
    response.BodyStream = std::move(wire.Value);
    response.ContentType = pRawResponse->GetHeaders().at("Content-Type");
    return Response<PublicModels::_detail::ListBlobsResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::_detail::ListBlobsResult> BlobContainerClient::ListBlobsByHierarchy(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const ListBlobContainerBlobsByHierarchyOptions& options,
      const Core::Context& context)
  {
    BlobsClientListBlobsHierarchyRawOptions generatedOptions;
    generatedOptions.Delimiter = NonEmpty(options.Delimiter);
    generatedOptions.Prefix = NonEmpty(options.Prefix);
    generatedOptions.Marker = NonEmpty(options.Marker);
    generatedOptions.Maxresults = options.MaxResults;
    generatedOptions.Include = Include<Models::ListBlobsIncludeItem>(options.Include);
    generatedOptions.StartFrom = NonEmpty(options.StartFrom);
    generatedOptions.EndBefore = NonEmpty(options.EndBefore);
    generatedOptions.Showonly = NonEmpty(options.ShowOnly);
    auto generatedContext = context;
    Core::CaseInsensitiveMap additionalHeaders;
    if (NonEmpty(options.Accept).HasValue())
    {
      additionalHeaders["Accept"] = options.Accept.Value();
    }
    generatedContext = generatedContext.WithValue(BlobsProtocolHeadersKey, additionalHeaders);
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url).ListBlobsHierarchyRaw(generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::_detail::ListBlobsResult response;
    response.BodyStream = std::move(wire.Value);
    response.ContentType = pRawResponse->GetHeaders().at("Content-Type");
    return Response<PublicModels::_detail::ListBlobsResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::AccountInfo> BlobContainerClient::GetAccountInfo(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const GetBlobContainerAccountInfoOptions&,
      const Core::Context& context)
  {
    BlobsClientGetAccountInfoContainerOptions generatedOptions;
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url).GetAccountInfoContainer(generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::AccountInfo response;
    response.SkuName = PublicModels::SkuName(pRawResponse->GetHeaders().at("x-ms-sku-name"));
    response.AccountKind
        = PublicModels::AccountKind(pRawResponse->GetHeaders().at("x-ms-account-kind"));
    response.IsHierarchicalNamespaceEnabled
        = pRawResponse->GetHeaders().at("x-ms-is-hns-enabled") == std::string("true");
    return Response<PublicModels::AccountInfo>(std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::DownloadBlobResult> BlobClient::Download(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const DownloadBlobOptions& options,
      const Core::Context& context)
  {
    BlobsClientDownloadBlobOptions generatedOptions;
    generatedOptions.Snapshot = NonEmpty(options.Snapshot);
    generatedOptions.Versionid = NonEmpty(options.VersionId);
    generatedOptions.Range = NonEmpty(options.Range);
    generatedOptions.XMsLeaseId = NonEmpty(options.LeaseId);
    generatedOptions.XMsRangeGetContentMd5 = options.RangeGetContentMD5;
    generatedOptions.XMsRangeGetContentCrc64 = options.RangeGetContentCRC64;
    generatedOptions.XMsStructuredBody = NonEmpty(options.StructuredBodyType);
    generatedOptions.XMsEncryptionKey = NonEmpty(options.EncryptionKey);
    generatedOptions.XMsEncryptionKeySha256 = Encoded(options.EncryptionKeySha256);
    generatedOptions.XMsEncryptionAlgorithm
        = Enum<Models::EncryptionAlgorithmType>(options.EncryptionAlgorithm);
    generatedOptions.XMsIfTags = NonEmpty(options.IfTags);
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    generatedOptions.MatchConditions.IfMatch = options.IfMatch;
    generatedOptions.MatchConditions.IfNoneMatch = options.IfNoneMatch;
    auto generatedContext = context;
    Core::CaseInsensitiveMap additionalHeaders;
    if (options.UserPrincipalName.HasValue())
    {
      additionalHeaders["x-ms-upn"] = options.UserPrincipalName.Value() ? "true" : "false";
    }
    generatedContext = generatedContext.WithValue(BlobsProtocolHeadersKey, additionalHeaders);
    auto wire = InvokeStorage(
        [&]() { return Client(pipeline, url).DownloadBlob(generatedOptions, generatedContext); });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (!(httpStatusCode == Core::Http::HttpStatusCode::Ok
          || httpStatusCode == Core::Http::HttpStatusCode::PartialContent))
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::DownloadBlobResult response;
    response.BodyStream = std::move(wire.Value.BodyStream);
    if (pRawResponse->GetHeaders().count("Last-Modified") != 0)
    {
      response.Details.LastModified = DateTime::Parse(
          pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    }
    response.Details.CreatedOn = DateTime::Parse(
        pRawResponse->GetHeaders().at("x-ms-creation-time"), Azure::DateTime::DateFormat::Rfc1123);
    for (auto i = pRawResponse->GetHeaders().lower_bound("x-ms-meta-");
         i != pRawResponse->GetHeaders().end() && i->first.substr(0, 10) == "x-ms-meta-";
         ++i)
    {
      response.Details.Metadata.emplace(i->first.substr(10), i->second);
    }
    if (pRawResponse->GetHeaders().count("x-ms-or-policy-id") != 0)
    {
      response.Details.ObjectReplicationDestinationPolicyId
          = pRawResponse->GetHeaders().at("x-ms-or-policy-id");
    }
    if (pRawResponse->GetHeaders().count("Content-Type") != 0)
    {
      response.Details.HttpHeaders.ContentType = pRawResponse->GetHeaders().at("Content-Type");
    }
    if (pRawResponse->GetHeaders().count("ETag") != 0)
    {
      response.Details.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    }
    if (pRawResponse->GetHeaders().count("Content-Encoding") != 0)
    {
      response.Details.HttpHeaders.ContentEncoding
          = pRawResponse->GetHeaders().at("Content-Encoding");
    }
    if (pRawResponse->GetHeaders().count("Cache-Control") != 0)
    {
      response.Details.HttpHeaders.CacheControl = pRawResponse->GetHeaders().at("Cache-Control");
    }
    if (pRawResponse->GetHeaders().count("Content-Disposition") != 0)
    {
      response.Details.HttpHeaders.ContentDisposition
          = pRawResponse->GetHeaders().at("Content-Disposition");
    }
    if (pRawResponse->GetHeaders().count("Content-Language") != 0)
    {
      response.Details.HttpHeaders.ContentLanguage
          = pRawResponse->GetHeaders().at("Content-Language");
    }
    if (pRawResponse->GetHeaders().count("x-ms-blob-sequence-number") != 0)
    {
      response.Details.SequenceNumber
          = std::stoll(pRawResponse->GetHeaders().at("x-ms-blob-sequence-number"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-blob-type") != 0)
    {
      response.BlobType = PublicModels::BlobType(pRawResponse->GetHeaders().at("x-ms-blob-type"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-copy-completion-time") != 0)
    {
      response.Details.CopyCompletedOn = DateTime::Parse(
          pRawResponse->GetHeaders().at("x-ms-copy-completion-time"),
          Azure::DateTime::DateFormat::Rfc1123);
    }
    if (pRawResponse->GetHeaders().count("x-ms-copy-status-description") != 0)
    {
      response.Details.CopyStatusDescription
          = pRawResponse->GetHeaders().at("x-ms-copy-status-description");
    }
    if (pRawResponse->GetHeaders().count("x-ms-copy-id") != 0)
    {
      response.Details.CopyId = pRawResponse->GetHeaders().at("x-ms-copy-id");
    }
    if (pRawResponse->GetHeaders().count("x-ms-copy-progress") != 0)
    {
      response.Details.CopyProgress = pRawResponse->GetHeaders().at("x-ms-copy-progress");
    }
    if (pRawResponse->GetHeaders().count("x-ms-copy-source") != 0)
    {
      response.Details.CopySource = pRawResponse->GetHeaders().at("x-ms-copy-source");
    }
    if (pRawResponse->GetHeaders().count("x-ms-copy-status") != 0)
    {
      response.Details.CopyStatus
          = PublicModels::CopyStatus(pRawResponse->GetHeaders().at("x-ms-copy-status"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-lease-duration") != 0)
    {
      response.Details.LeaseDuration
          = PublicModels::LeaseDurationType(pRawResponse->GetHeaders().at("x-ms-lease-duration"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-lease-state") != 0)
    {
      response.Details.LeaseState
          = PublicModels::LeaseState(pRawResponse->GetHeaders().at("x-ms-lease-state"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-lease-status") != 0)
    {
      response.Details.LeaseStatus
          = PublicModels::LeaseStatus(pRawResponse->GetHeaders().at("x-ms-lease-status"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-version-id") != 0)
    {
      response.Details.VersionId = pRawResponse->GetHeaders().at("x-ms-version-id");
    }
    if (pRawResponse->GetHeaders().count("x-ms-is-current-version") != 0)
    {
      response.Details.IsCurrentVersion
          = pRawResponse->GetHeaders().at("x-ms-is-current-version") == std::string("true");
    }
    if (pRawResponse->GetHeaders().count("x-ms-blob-committed-block-count") != 0)
    {
      response.Details.CommittedBlockCount
          = std::stoi(pRawResponse->GetHeaders().at("x-ms-blob-committed-block-count"));
    }
    response.Details.IsServerEncrypted
        = pRawResponse->GetHeaders().at("x-ms-server-encrypted") == std::string("true");
    if (pRawResponse->GetHeaders().count("x-ms-encryption-key-sha256") != 0)
    {
      response.Details.EncryptionKeySha256 = Core::Convert::Base64Decode(
          pRawResponse->GetHeaders().at("x-ms-encryption-key-sha256"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-encryption-scope") != 0)
    {
      response.Details.EncryptionScope = pRawResponse->GetHeaders().at("x-ms-encryption-scope");
    }
    if (pRawResponse->GetHeaders().count("x-ms-tag-count") != 0)
    {
      response.Details.TagCount = std::stoi(pRawResponse->GetHeaders().at("x-ms-tag-count"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-blob-sealed") != 0)
    {
      response.Details.IsSealed
          = pRawResponse->GetHeaders().at("x-ms-blob-sealed") == std::string("true");
    }
    if (pRawResponse->GetHeaders().count("x-ms-last-access-time") != 0)
    {
      response.Details.LastAccessedOn = DateTime::Parse(
          pRawResponse->GetHeaders().at("x-ms-last-access-time"),
          Azure::DateTime::DateFormat::Rfc1123);
    }
    if (pRawResponse->GetHeaders().count("x-ms-immutability-policy-until-date") != 0)
    {
      if (!response.Details.ImmutabilityPolicy.HasValue())
      {
        response.Details.ImmutabilityPolicy = PublicModels::BlobImmutabilityPolicy();
      }
      response.Details.ImmutabilityPolicy.Value().ExpiresOn = DateTime::Parse(
          pRawResponse->GetHeaders().at("x-ms-immutability-policy-until-date"),
          Azure::DateTime::DateFormat::Rfc1123);
    }
    if (pRawResponse->GetHeaders().count("x-ms-immutability-policy-mode") != 0)
    {
      if (!response.Details.ImmutabilityPolicy.HasValue())
      {
        response.Details.ImmutabilityPolicy = PublicModels::BlobImmutabilityPolicy();
      }
      response.Details.ImmutabilityPolicy.Value().PolicyMode
          = PublicModels::BlobImmutabilityPolicyMode(
              pRawResponse->GetHeaders().at("x-ms-immutability-policy-mode"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-legal-hold") != 0)
    {
      response.Details.HasLegalHold
          = pRawResponse->GetHeaders().at("x-ms-legal-hold") == std::string("true");
    }
    if (pRawResponse->GetHeaders().count("x-ms-access-tier") != 0)
    {
      response.Details.AccessTier
          = PublicModels::AccessTier(pRawResponse->GetHeaders().at("x-ms-access-tier"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-access-tier-inferred") != 0)
    {
      response.Details.IsAccessTierInferred
          = pRawResponse->GetHeaders().at("x-ms-access-tier-inferred") == std::string("true");
    }
    if (pRawResponse->GetHeaders().count("x-ms-access-tier-change-time") != 0)
    {
      response.Details.AccessTierChangedOn = DateTime::Parse(
          pRawResponse->GetHeaders().at("x-ms-access-tier-change-time"),
          Azure::DateTime::DateFormat::Rfc1123);
    }
    if (pRawResponse->GetHeaders().count("x-ms-smart-access-tier") != 0)
    {
      response.Details.SmartAccessTier
          = PublicModels::AccessTier(pRawResponse->GetHeaders().at("x-ms-smart-access-tier"));
    }
    if (httpStatusCode == Core::Http::HttpStatusCode::Ok)
    {
      if (pRawResponse->GetHeaders().count("Content-MD5") != 0)
      {
        response.Details.HttpHeaders.ContentHash.Value
            = Core::Convert::Base64Decode(pRawResponse->GetHeaders().at("Content-MD5"));
        response.Details.HttpHeaders.ContentHash.Algorithm = HashAlgorithm::Md5;
      }
    }
    if (httpStatusCode == Core::Http::HttpStatusCode::Ok)
    {
      if (pRawResponse->GetHeaders().count("Content-MD5") != 0)
      {
        response.TransactionalContentHash = ContentHash();
        response.TransactionalContentHash.Value().Value
            = Core::Convert::Base64Decode(pRawResponse->GetHeaders().at("Content-MD5"));
        response.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Md5;
      }
    }
    if (pRawResponse->GetHeaders().count("x-ms-blob-content-md5") != 0)
    {
      response.Details.HttpHeaders.ContentHash.Value
          = Core::Convert::Base64Decode(pRawResponse->GetHeaders().at("x-ms-blob-content-md5"));
      response.Details.HttpHeaders.ContentHash.Algorithm = HashAlgorithm::Md5;
    }
    if (httpStatusCode == Core::Http::HttpStatusCode::PartialContent)
    {
      if (pRawResponse->GetHeaders().count("Content-MD5") != 0)
      {
        response.TransactionalContentHash = ContentHash();
        response.TransactionalContentHash.Value().Value
            = Core::Convert::Base64Decode(pRawResponse->GetHeaders().at("Content-MD5"));
        response.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Md5;
      }
    }
    if (httpStatusCode == Core::Http::HttpStatusCode::PartialContent)
    {
      if (pRawResponse->GetHeaders().count("x-ms-content-crc64") != 0)
      {
        response.TransactionalContentHash = ContentHash();
        response.TransactionalContentHash.Value().Value
            = Core::Convert::Base64Decode(pRawResponse->GetHeaders().at("x-ms-content-crc64"));
        response.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Crc64;
      }
    }
    return Response<PublicModels::DownloadBlobResult>(std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::BlobProperties> BlobClient::GetProperties(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const GetBlobPropertiesOptions& options,
      const Core::Context& context)
  {
    BlobsClientGetPropertiesBlobOptions generatedOptions;
    generatedOptions.Snapshot = NonEmpty(options.Snapshot);
    generatedOptions.Versionid = NonEmpty(options.VersionId);
    generatedOptions.XMsLeaseId = NonEmpty(options.LeaseId);
    generatedOptions.XMsEncryptionKey = NonEmpty(options.EncryptionKey);
    generatedOptions.XMsEncryptionKeySha256 = Encoded(options.EncryptionKeySha256);
    generatedOptions.XMsEncryptionAlgorithm
        = Enum<Models::EncryptionAlgorithmType>(options.EncryptionAlgorithm);
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    generatedOptions.MatchConditions.IfMatch = options.IfMatch;
    generatedOptions.MatchConditions.IfNoneMatch = options.IfNoneMatch;
    generatedOptions.XMsIfTags = NonEmpty(options.IfTags);
    auto generatedContext = context;
    Core::CaseInsensitiveMap additionalHeaders;
    if (options.UserPrincipalName.HasValue())
    {
      additionalHeaders["x-ms-upn"] = options.UserPrincipalName.Value() ? "true" : "false";
    }
    generatedContext = generatedContext.WithValue(BlobsProtocolHeadersKey, additionalHeaders);
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url).GetPropertiesBlob(generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::BlobProperties response;
    if (pRawResponse->GetHeaders().count("Last-Modified") != 0)
    {
      response.LastModified = DateTime::Parse(
          pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    }
    response.CreatedOn = DateTime::Parse(
        pRawResponse->GetHeaders().at("x-ms-creation-time"), Azure::DateTime::DateFormat::Rfc1123);
    for (auto i = pRawResponse->GetHeaders().lower_bound("x-ms-meta-");
         i != pRawResponse->GetHeaders().end() && i->first.substr(0, 10) == "x-ms-meta-";
         ++i)
    {
      response.Metadata.emplace(i->first.substr(10), i->second);
    }
    if (pRawResponse->GetHeaders().count("x-ms-or-policy-id") != 0)
    {
      response.ObjectReplicationDestinationPolicyId
          = pRawResponse->GetHeaders().at("x-ms-or-policy-id");
    }
    if (pRawResponse->GetHeaders().count("x-ms-blob-type") != 0)
    {
      response.BlobType = PublicModels::BlobType(pRawResponse->GetHeaders().at("x-ms-blob-type"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-copy-completion-time") != 0)
    {
      response.CopyCompletedOn = DateTime::Parse(
          pRawResponse->GetHeaders().at("x-ms-copy-completion-time"),
          Azure::DateTime::DateFormat::Rfc1123);
    }
    if (pRawResponse->GetHeaders().count("x-ms-copy-status-description") != 0)
    {
      response.CopyStatusDescription
          = pRawResponse->GetHeaders().at("x-ms-copy-status-description");
    }
    if (pRawResponse->GetHeaders().count("x-ms-copy-id") != 0)
    {
      response.CopyId = pRawResponse->GetHeaders().at("x-ms-copy-id");
    }
    if (pRawResponse->GetHeaders().count("x-ms-copy-progress") != 0)
    {
      response.CopyProgress = pRawResponse->GetHeaders().at("x-ms-copy-progress");
    }
    if (pRawResponse->GetHeaders().count("x-ms-copy-source") != 0)
    {
      response.CopySource = pRawResponse->GetHeaders().at("x-ms-copy-source");
    }
    if (pRawResponse->GetHeaders().count("x-ms-copy-status") != 0)
    {
      response.CopyStatus
          = PublicModels::CopyStatus(pRawResponse->GetHeaders().at("x-ms-copy-status"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-incremental-copy") != 0)
    {
      response.IsIncrementalCopy
          = pRawResponse->GetHeaders().at("x-ms-incremental-copy") == std::string("true");
    }
    if (pRawResponse->GetHeaders().count("x-ms-copy-destination-snapshot") != 0)
    {
      response.IncrementalCopyDestinationSnapshot
          = pRawResponse->GetHeaders().at("x-ms-copy-destination-snapshot");
    }
    if (pRawResponse->GetHeaders().count("x-ms-lease-duration") != 0)
    {
      response.LeaseDuration
          = PublicModels::LeaseDurationType(pRawResponse->GetHeaders().at("x-ms-lease-duration"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-lease-state") != 0)
    {
      response.LeaseState
          = PublicModels::LeaseState(pRawResponse->GetHeaders().at("x-ms-lease-state"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-lease-status") != 0)
    {
      response.LeaseStatus
          = PublicModels::LeaseStatus(pRawResponse->GetHeaders().at("x-ms-lease-status"));
    }
    response.BlobSize = std::stoll(pRawResponse->GetHeaders().at("Content-Length"));
    if (pRawResponse->GetHeaders().count("Content-Type") != 0)
    {
      response.HttpHeaders.ContentType = pRawResponse->GetHeaders().at("Content-Type");
    }
    if (pRawResponse->GetHeaders().count("ETag") != 0)
    {
      response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    }
    if (pRawResponse->GetHeaders().count("Content-MD5") != 0)
    {
      response.HttpHeaders.ContentHash.Value
          = Core::Convert::Base64Decode(pRawResponse->GetHeaders().at("Content-MD5"));
      response.HttpHeaders.ContentHash.Algorithm = HashAlgorithm::Md5;
    }
    if (pRawResponse->GetHeaders().count("Content-Encoding") != 0)
    {
      response.HttpHeaders.ContentEncoding = pRawResponse->GetHeaders().at("Content-Encoding");
    }
    if (pRawResponse->GetHeaders().count("Content-Disposition") != 0)
    {
      response.HttpHeaders.ContentDisposition
          = pRawResponse->GetHeaders().at("Content-Disposition");
    }
    if (pRawResponse->GetHeaders().count("Content-Language") != 0)
    {
      response.HttpHeaders.ContentLanguage = pRawResponse->GetHeaders().at("Content-Language");
    }
    if (pRawResponse->GetHeaders().count("Cache-Control") != 0)
    {
      response.HttpHeaders.CacheControl = pRawResponse->GetHeaders().at("Cache-Control");
    }
    if (pRawResponse->GetHeaders().count("x-ms-blob-sequence-number") != 0)
    {
      response.SequenceNumber
          = std::stoll(pRawResponse->GetHeaders().at("x-ms-blob-sequence-number"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-blob-committed-block-count") != 0)
    {
      response.CommittedBlockCount
          = std::stoi(pRawResponse->GetHeaders().at("x-ms-blob-committed-block-count"));
    }
    response.IsServerEncrypted
        = pRawResponse->GetHeaders().at("x-ms-server-encrypted") == std::string("true");
    if (pRawResponse->GetHeaders().count("x-ms-encryption-key-sha256") != 0)
    {
      response.EncryptionKeySha256 = Core::Convert::Base64Decode(
          pRawResponse->GetHeaders().at("x-ms-encryption-key-sha256"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-encryption-scope") != 0)
    {
      response.EncryptionScope = pRawResponse->GetHeaders().at("x-ms-encryption-scope");
    }
    if (pRawResponse->GetHeaders().count("x-ms-access-tier") != 0)
    {
      response.AccessTier
          = PublicModels::AccessTier(pRawResponse->GetHeaders().at("x-ms-access-tier"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-access-tier-inferred") != 0)
    {
      response.IsAccessTierInferred
          = pRawResponse->GetHeaders().at("x-ms-access-tier-inferred") == std::string("true");
    }
    if (pRawResponse->GetHeaders().count("x-ms-archive-status") != 0)
    {
      response.ArchiveStatus
          = PublicModels::ArchiveStatus(pRawResponse->GetHeaders().at("x-ms-archive-status"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-access-tier-change-time") != 0)
    {
      response.AccessTierChangedOn = DateTime::Parse(
          pRawResponse->GetHeaders().at("x-ms-access-tier-change-time"),
          Azure::DateTime::DateFormat::Rfc1123);
    }
    if (pRawResponse->GetHeaders().count("x-ms-smart-access-tier") != 0)
    {
      response.SmartAccessTier
          = PublicModels::AccessTier(pRawResponse->GetHeaders().at("x-ms-smart-access-tier"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-version-id") != 0)
    {
      response.VersionId = pRawResponse->GetHeaders().at("x-ms-version-id");
    }
    if (pRawResponse->GetHeaders().count("x-ms-is-current-version") != 0)
    {
      response.IsCurrentVersion
          = pRawResponse->GetHeaders().at("x-ms-is-current-version") == std::string("true");
    }
    if (pRawResponse->GetHeaders().count("x-ms-tag-count") != 0)
    {
      response.TagCount = std::stoi(pRawResponse->GetHeaders().at("x-ms-tag-count"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-expiry-time") != 0)
    {
      response.ExpiresOn = DateTime::Parse(
          pRawResponse->GetHeaders().at("x-ms-expiry-time"), Azure::DateTime::DateFormat::Rfc1123);
    }
    if (pRawResponse->GetHeaders().count("x-ms-blob-sealed") != 0)
    {
      response.IsSealed = pRawResponse->GetHeaders().at("x-ms-blob-sealed") == std::string("true");
    }
    if (pRawResponse->GetHeaders().count("x-ms-rehydrate-priority") != 0)
    {
      response.RehydratePriority = PublicModels::RehydratePriority(
          pRawResponse->GetHeaders().at("x-ms-rehydrate-priority"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-last-access-time") != 0)
    {
      response.LastAccessedOn = DateTime::Parse(
          pRawResponse->GetHeaders().at("x-ms-last-access-time"),
          Azure::DateTime::DateFormat::Rfc1123);
    }
    if (pRawResponse->GetHeaders().count("x-ms-immutability-policy-until-date") != 0)
    {
      if (!response.ImmutabilityPolicy.HasValue())
      {
        response.ImmutabilityPolicy = PublicModels::BlobImmutabilityPolicy();
      }
      response.ImmutabilityPolicy.Value().ExpiresOn = DateTime::Parse(
          pRawResponse->GetHeaders().at("x-ms-immutability-policy-until-date"),
          Azure::DateTime::DateFormat::Rfc1123);
    }
    if (pRawResponse->GetHeaders().count("x-ms-immutability-policy-mode") != 0)
    {
      if (!response.ImmutabilityPolicy.HasValue())
      {
        response.ImmutabilityPolicy = PublicModels::BlobImmutabilityPolicy();
      }
      response.ImmutabilityPolicy.Value().PolicyMode = PublicModels::BlobImmutabilityPolicyMode(
          pRawResponse->GetHeaders().at("x-ms-immutability-policy-mode"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-legal-hold") != 0)
    {
      response.HasLegalHold
          = pRawResponse->GetHeaders().at("x-ms-legal-hold") == std::string("true");
    }
    return Response<PublicModels::BlobProperties>(std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::DeleteBlobResult> BlobClient::Delete(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const DeleteBlobOptions& options,
      const Core::Context& context)
  {
    BlobsClientDeleteBlobOptions generatedOptions;
    generatedOptions.Snapshot = NonEmpty(options.Snapshot);
    generatedOptions.Versionid = NonEmpty(options.VersionId);
    generatedOptions.XMsLeaseId = NonEmpty(options.LeaseId);
    generatedOptions.XMsDeleteSnapshots
        = Enum<Models::DeleteSnapshotsOptionType>(options.DeleteSnapshots);
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    generatedOptions.MatchConditions.IfMatch = options.IfMatch;
    generatedOptions.MatchConditions.IfNoneMatch = options.IfNoneMatch;
    generatedOptions.XMsIfTags = NonEmpty(options.IfTags);
    generatedOptions.XMsAccessTierIfModifiedSince = options.AccessTierIfModifiedSince;
    generatedOptions.XMsAccessTierIfUnmodifiedSince = options.AccessTierIfUnmodifiedSince;
    auto generatedContext = context;
    auto wire = InvokeStorage(
        [&]() { return Client(pipeline, url).DeleteBlob(generatedOptions, generatedContext); });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Accepted)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::DeleteBlobResult response;
    return Response<PublicModels::DeleteBlobResult>(std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::UndeleteBlobResult> BlobClient::Undelete(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const UndeleteBlobOptions&,
      const Core::Context& context)
  {
    BlobsClientUndeleteBlobOptions generatedOptions;
    auto generatedContext = context;
    auto wire = InvokeStorage(
        [&]() { return Client(pipeline, url).UndeleteBlob(generatedOptions, generatedContext); });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::UndeleteBlobResult response;
    return Response<PublicModels::UndeleteBlobResult>(std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::SetBlobExpiryResult> BlobClient::SetExpiry(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const SetBlobExpiryOptions& options,
      const Core::Context& context)
  {
    BlobsClientSetExpiryBlobOptions generatedOptions;
    generatedOptions.XMsExpiryTime = NonEmpty(options.ExpiresOn);
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .SetExpiryBlob(
              Models::BlobExpiryOptions(Text(options.ExpiryOptions)),
              generatedOptions,
              generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::SetBlobExpiryResult response;
    if (pRawResponse->GetHeaders().count("ETag") != 0)
    {
      response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    }
    if (pRawResponse->GetHeaders().count("Last-Modified") != 0)
    {
      response.LastModified = DateTime::Parse(
          pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    }
    return Response<PublicModels::SetBlobExpiryResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::SetBlobHttpHeadersResult> BlobClient::SetHttpHeaders(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const SetBlobHttpHeadersOptions& options,
      const Core::Context& context)
  {
    BlobsClientSetPropertiesBlobOptions generatedOptions;
    generatedOptions.XMsBlobCacheControl = NonEmpty(options.BlobCacheControl);
    generatedOptions.XMsBlobContentType = NonEmpty(options.BlobContentType);
    generatedOptions.XMsBlobContentMd5 = NonEmpty(options.BlobContentMD5);
    generatedOptions.XMsBlobContentEncoding = NonEmpty(options.BlobContentEncoding);
    generatedOptions.XMsBlobContentLanguage = NonEmpty(options.BlobContentLanguage);
    generatedOptions.XMsLeaseId = NonEmpty(options.LeaseId);
    generatedOptions.XMsBlobContentDisposition = NonEmpty(options.BlobContentDisposition);
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    generatedOptions.MatchConditions.IfMatch = options.IfMatch;
    generatedOptions.MatchConditions.IfNoneMatch = options.IfNoneMatch;
    generatedOptions.XMsIfTags = NonEmpty(options.IfTags);
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url).SetPropertiesBlob(generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::SetBlobHttpHeadersResult response;
    if (pRawResponse->GetHeaders().count("ETag") != 0)
    {
      response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    }
    if (pRawResponse->GetHeaders().count("Last-Modified") != 0)
    {
      response.LastModified = DateTime::Parse(
          pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    }
    if (pRawResponse->GetHeaders().count("x-ms-blob-sequence-number") != 0)
    {
      response.SequenceNumber
          = std::stoll(pRawResponse->GetHeaders().at("x-ms-blob-sequence-number"));
    }
    return Response<PublicModels::SetBlobHttpHeadersResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::SetBlobImmutabilityPolicyResult> BlobClient::SetImmutabilityPolicy(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const SetBlobImmutabilityPolicyOptions& options,
      const Core::Context& context)
  {
    BlobsClientSetImmutabilityPolicyBlobOptions generatedOptions;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    generatedOptions.XMsImmutabilityPolicyMode
        = Enum<Models::ImmutabilityPolicyMode>(options.ImmutabilityPolicyMode);
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .SetImmutabilityPolicyBlob(
              Required(options.ImmutabilityPolicyExpiry), generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::SetBlobImmutabilityPolicyResult response;
    response.ImmutabilityPolicy.ExpiresOn = DateTime::Parse(
        pRawResponse->GetHeaders().at("x-ms-immutability-policy-until-date"),
        Azure::DateTime::DateFormat::Rfc1123);
    response.ImmutabilityPolicy.PolicyMode = PublicModels::BlobImmutabilityPolicyMode(
        pRawResponse->GetHeaders().at("x-ms-immutability-policy-mode"));
    return Response<PublicModels::SetBlobImmutabilityPolicyResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::DeleteBlobImmutabilityPolicyResult> BlobClient::DeleteImmutabilityPolicy(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const DeleteBlobImmutabilityPolicyOptions&,
      const Core::Context& context)
  {
    BlobsClientDeleteImmutabilityPolicyBlobOptions generatedOptions;
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url).DeleteImmutabilityPolicyBlob(generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::DeleteBlobImmutabilityPolicyResult response;
    return Response<PublicModels::DeleteBlobImmutabilityPolicyResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::SetBlobLegalHoldResult> BlobClient::SetLegalHold(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const SetBlobLegalHoldOptions& options,
      const Core::Context& context)
  {
    BlobsClientSetLegalHoldBlobOptions generatedOptions;
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .SetLegalHoldBlob(options.LegalHold, generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::SetBlobLegalHoldResult response;
    response.HasLegalHold = pRawResponse->GetHeaders().at("x-ms-legal-hold") == std::string("true");
    return Response<PublicModels::SetBlobLegalHoldResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::SetBlobMetadataResult> BlobClient::SetMetadata(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const SetBlobMetadataOptions& options,
      const Core::Context& context)
  {
    BlobsClientSetMetadataBlobOptions generatedOptions;
    generatedOptions.Metadata = Metadata(options.Metadata);
    generatedOptions.XMsLeaseId = NonEmpty(options.LeaseId);
    generatedOptions.XMsEncryptionKey = NonEmpty(options.EncryptionKey);
    generatedOptions.XMsEncryptionKeySha256 = Encoded(options.EncryptionKeySha256);
    generatedOptions.XMsEncryptionAlgorithm
        = Enum<Models::EncryptionAlgorithmType>(options.EncryptionAlgorithm);
    generatedOptions.XMsEncryptionScope = NonEmpty(options.EncryptionScope);
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    generatedOptions.MatchConditions.IfMatch = options.IfMatch;
    generatedOptions.MatchConditions.IfNoneMatch = options.IfNoneMatch;
    generatedOptions.XMsIfTags = NonEmpty(options.IfTags);
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url).SetMetadataBlob(generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::SetBlobMetadataResult response;
    if (pRawResponse->GetHeaders().count("ETag") != 0)
    {
      response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    }
    if (pRawResponse->GetHeaders().count("Last-Modified") != 0)
    {
      response.LastModified = DateTime::Parse(
          pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    }
    if (pRawResponse->GetHeaders().count("x-ms-version-id") != 0)
    {
      response.VersionId = pRawResponse->GetHeaders().at("x-ms-version-id");
    }
    response.IsServerEncrypted
        = pRawResponse->GetHeaders().at("x-ms-request-server-encrypted") == std::string("true");
    if (pRawResponse->GetHeaders().count("x-ms-encryption-key-sha256") != 0)
    {
      response.EncryptionKeySha256 = Core::Convert::Base64Decode(
          pRawResponse->GetHeaders().at("x-ms-encryption-key-sha256"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-encryption-scope") != 0)
    {
      response.EncryptionScope = pRawResponse->GetHeaders().at("x-ms-encryption-scope");
    }
    return Response<PublicModels::SetBlobMetadataResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::_detail::AcquireBlobLeaseResult> BlobClient::AcquireLease(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const AcquireBlobLeaseOptions& options,
      const Core::Context& context)
  {
    BlobsClientAcquireLeaseBlobOptions generatedOptions;
    generatedOptions.XMsProposedLeaseId = NonEmpty(options.ProposedLeaseId);
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    generatedOptions.MatchConditions.IfMatch = options.IfMatch;
    generatedOptions.MatchConditions.IfNoneMatch = options.IfNoneMatch;
    generatedOptions.XMsIfTags = NonEmpty(options.IfTags);
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .AcquireLeaseBlob(Required(options.Duration), generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Created)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::_detail::AcquireBlobLeaseResult response;
    if (pRawResponse->GetHeaders().count("ETag") != 0)
    {
      response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    }
    if (pRawResponse->GetHeaders().count("Last-Modified") != 0)
    {
      response.LastModified = DateTime::Parse(
          pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    }
    response.LeaseId = pRawResponse->GetHeaders().at("x-ms-lease-id");
    return Response<PublicModels::_detail::AcquireBlobLeaseResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::_detail::ReleaseBlobLeaseResult> BlobClient::ReleaseLease(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const ReleaseBlobLeaseOptions& options,
      const Core::Context& context)
  {
    BlobsClientReleaseLeaseBlobOptions generatedOptions;
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    generatedOptions.MatchConditions.IfMatch = options.IfMatch;
    generatedOptions.MatchConditions.IfNoneMatch = options.IfNoneMatch;
    generatedOptions.XMsIfTags = NonEmpty(options.IfTags);
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .ReleaseLeaseBlob(options.LeaseId, generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::_detail::ReleaseBlobLeaseResult response;
    if (pRawResponse->GetHeaders().count("ETag") != 0)
    {
      response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    }
    if (pRawResponse->GetHeaders().count("Last-Modified") != 0)
    {
      response.LastModified = DateTime::Parse(
          pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    }
    return Response<PublicModels::_detail::ReleaseBlobLeaseResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::_detail::RenewBlobLeaseResult> BlobClient::RenewLease(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const RenewBlobLeaseOptions& options,
      const Core::Context& context)
  {
    BlobsClientRenewLeaseBlobOptions generatedOptions;
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    generatedOptions.MatchConditions.IfMatch = options.IfMatch;
    generatedOptions.MatchConditions.IfNoneMatch = options.IfNoneMatch;
    generatedOptions.XMsIfTags = NonEmpty(options.IfTags);
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .RenewLeaseBlob(options.LeaseId, generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::_detail::RenewBlobLeaseResult response;
    if (pRawResponse->GetHeaders().count("ETag") != 0)
    {
      response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    }
    if (pRawResponse->GetHeaders().count("Last-Modified") != 0)
    {
      response.LastModified = DateTime::Parse(
          pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    }
    response.LeaseId = pRawResponse->GetHeaders().at("x-ms-lease-id");
    return Response<PublicModels::_detail::RenewBlobLeaseResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::_detail::ChangeBlobLeaseResult> BlobClient::ChangeLease(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const ChangeBlobLeaseOptions& options,
      const Core::Context& context)
  {
    BlobsClientChangeLeaseBlobOptions generatedOptions;
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    generatedOptions.MatchConditions.IfMatch = options.IfMatch;
    generatedOptions.MatchConditions.IfNoneMatch = options.IfNoneMatch;
    generatedOptions.XMsIfTags = NonEmpty(options.IfTags);
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .ChangeLeaseBlob(
              options.LeaseId, options.ProposedLeaseId, generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::_detail::ChangeBlobLeaseResult response;
    if (pRawResponse->GetHeaders().count("ETag") != 0)
    {
      response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    }
    if (pRawResponse->GetHeaders().count("Last-Modified") != 0)
    {
      response.LastModified = DateTime::Parse(
          pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    }
    response.LeaseId = pRawResponse->GetHeaders().at("x-ms-lease-id");
    return Response<PublicModels::_detail::ChangeBlobLeaseResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::_detail::BreakBlobLeaseResult> BlobClient::BreakLease(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const BreakBlobLeaseOptions& options,
      const Core::Context& context)
  {
    BlobsClientBreakLeaseBlobOptions generatedOptions;
    generatedOptions.XMsLeaseBreakPeriod = options.BreakPeriod;
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    generatedOptions.MatchConditions.IfMatch = options.IfMatch;
    generatedOptions.MatchConditions.IfNoneMatch = options.IfNoneMatch;
    generatedOptions.XMsIfTags = NonEmpty(options.IfTags);
    auto generatedContext = context;
    auto wire = InvokeStorage(
        [&]() { return Client(pipeline, url).BreakLeaseBlob(generatedOptions, generatedContext); });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Accepted)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::_detail::BreakBlobLeaseResult response;
    if (pRawResponse->GetHeaders().count("ETag") != 0)
    {
      response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    }
    if (pRawResponse->GetHeaders().count("Last-Modified") != 0)
    {
      response.LastModified = DateTime::Parse(
          pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    }
    response.LeaseTime = std::stoi(pRawResponse->GetHeaders().at("x-ms-lease-time"));
    return Response<PublicModels::_detail::BreakBlobLeaseResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::CreateBlobSnapshotResult> BlobClient::CreateSnapshot(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const CreateBlobSnapshotOptions& options,
      const Core::Context& context)
  {
    BlobsClientCreateSnapshotBlobOptions generatedOptions;
    generatedOptions.Metadata = Metadata(options.Metadata);
    generatedOptions.XMsEncryptionKey = NonEmpty(options.EncryptionKey);
    generatedOptions.XMsEncryptionKeySha256 = Encoded(options.EncryptionKeySha256);
    generatedOptions.XMsEncryptionAlgorithm
        = Enum<Models::EncryptionAlgorithmType>(options.EncryptionAlgorithm);
    generatedOptions.XMsEncryptionScope = NonEmpty(options.EncryptionScope);
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    generatedOptions.MatchConditions.IfMatch = options.IfMatch;
    generatedOptions.MatchConditions.IfNoneMatch = options.IfNoneMatch;
    generatedOptions.XMsIfTags = NonEmpty(options.IfTags);
    generatedOptions.XMsLeaseId = NonEmpty(options.LeaseId);
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url).CreateSnapshotBlob(generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Created)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::CreateBlobSnapshotResult response;
    response.Snapshot = pRawResponse->GetHeaders().at("x-ms-snapshot");
    if (pRawResponse->GetHeaders().count("ETag") != 0)
    {
      response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    }
    if (pRawResponse->GetHeaders().count("Last-Modified") != 0)
    {
      response.LastModified = DateTime::Parse(
          pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    }
    if (pRawResponse->GetHeaders().count("x-ms-version-id") != 0)
    {
      response.VersionId = pRawResponse->GetHeaders().at("x-ms-version-id");
    }
    response.IsServerEncrypted
        = pRawResponse->GetHeaders().at("x-ms-request-server-encrypted") == std::string("true");
    return Response<PublicModels::CreateBlobSnapshotResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::_detail::StartBlobCopyFromUriResult> BlobClient::StartCopyFromUri(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const StartBlobCopyFromUriOptions& options,
      const Core::Context& context)
  {
    BlobsClientStartCopyFromUrlBlobOptions generatedOptions;
    generatedOptions.Metadata = Metadata(options.Metadata);
    generatedOptions.XMsAccessTier = Enum<Models::AccessTier>(options.Tier);
    generatedOptions.XMsRehydratePriority
        = Enum<Models::RehydratePriority>(options.RehydratePriority);
    generatedOptions.XMsSourceIfModifiedSince = options.SourceIfModifiedSince;
    generatedOptions.XMsSourceIfUnmodifiedSince = options.SourceIfUnmodifiedSince;
    generatedOptions.XMsSourceIfMatch = options.SourceIfMatch;
    generatedOptions.XMsSourceIfNoneMatch = options.SourceIfNoneMatch;
    generatedOptions.XMsSourceIfTags = NonEmpty(options.SourceIfTags);
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    generatedOptions.MatchConditions.IfMatch = options.IfMatch;
    generatedOptions.MatchConditions.IfNoneMatch = options.IfNoneMatch;
    generatedOptions.XMsIfTags = NonEmpty(options.IfTags);
    generatedOptions.XMsLeaseId = NonEmpty(options.LeaseId);
    generatedOptions.XMsTags = NonEmpty(options.BlobTagsString);
    generatedOptions.XMsSealBlob = options.SealBlob;
    generatedOptions.XMsImmutabilityPolicyUntilDate = options.ImmutabilityPolicyExpiry;
    generatedOptions.XMsImmutabilityPolicyMode
        = Enum<Models::ImmutabilityPolicyMode>(options.ImmutabilityPolicyMode);
    generatedOptions.XMsLegalHold = options.LegalHold;
    auto generatedContext = context;
    Core::CaseInsensitiveMap additionalHeaders;
    if (NonEmpty(options.SourceLeaseId).HasValue())
    {
      additionalHeaders["x-ms-source-lease-id"] = options.SourceLeaseId.Value();
    }
    generatedContext = generatedContext.WithValue(BlobsProtocolHeadersKey, additionalHeaders);
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .StartCopyFromUrlBlob(options.CopySource, generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Accepted)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::_detail::StartBlobCopyFromUriResult response;
    if (pRawResponse->GetHeaders().count("ETag") != 0)
    {
      response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    }
    if (pRawResponse->GetHeaders().count("Last-Modified") != 0)
    {
      response.LastModified = DateTime::Parse(
          pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    }
    if (pRawResponse->GetHeaders().count("x-ms-version-id") != 0)
    {
      response.VersionId = pRawResponse->GetHeaders().at("x-ms-version-id");
    }
    response.CopyId = pRawResponse->GetHeaders().at("x-ms-copy-id");
    response.CopyStatus
        = PublicModels::CopyStatus(pRawResponse->GetHeaders().at("x-ms-copy-status"));
    return Response<PublicModels::_detail::StartBlobCopyFromUriResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::CopyBlobFromUriResult> BlobClient::CopyFromUri(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const CopyBlobFromUriOptions& options,
      const Core::Context& context)
  {
    BlobsClientCopyFromUrlBlobOptions generatedOptions;
    generatedOptions.Metadata = Metadata(options.Metadata);
    generatedOptions.XMsAccessTier = Enum<Models::AccessTier>(options.Tier);
    generatedOptions.XMsSourceIfModifiedSince = options.SourceIfModifiedSince;
    generatedOptions.XMsSourceIfUnmodifiedSince = options.SourceIfUnmodifiedSince;
    generatedOptions.XMsSourceIfMatch = options.SourceIfMatch;
    generatedOptions.XMsSourceIfNoneMatch = options.SourceIfNoneMatch;
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    generatedOptions.MatchConditions.IfMatch = options.IfMatch;
    generatedOptions.MatchConditions.IfNoneMatch = options.IfNoneMatch;
    generatedOptions.XMsIfTags = NonEmpty(options.IfTags);
    generatedOptions.XMsLeaseId = NonEmpty(options.LeaseId);
    generatedOptions.XMsSourceContentMd5 = NonEmpty(options.SourceContentMD5);
    generatedOptions.XMsTags = NonEmpty(options.BlobTagsString);
    generatedOptions.XMsImmutabilityPolicyUntilDate = options.ImmutabilityPolicyExpiry;
    generatedOptions.XMsImmutabilityPolicyMode
        = Enum<Models::ImmutabilityPolicyMode>(options.ImmutabilityPolicyMode);
    generatedOptions.XMsLegalHold = options.LegalHold;
    generatedOptions.XMsCopySourceAuthorization = NonEmpty(options.CopySourceAuthorization);
    generatedOptions.XMsEncryptionScope = NonEmpty(options.EncryptionScope);
    generatedOptions.XMsCopySourceTagOption
        = Enum<Models::BlobCopySourceTags>(options.CopySourceTags);
    generatedOptions.XMsFileRequestIntent
        = Enum<Models::FileShareTokenIntent>(options.FileRequestIntent);
    auto generatedContext = context;
    Core::CaseInsensitiveMap additionalHeaders;
    if (NonEmpty(options.SourceContentcrc64).HasValue())
    {
      additionalHeaders["x-ms-source-content-crc64"]
          = Core::Convert::Base64Encode(options.SourceContentcrc64.Value());
    }
    generatedContext = generatedContext.WithValue(BlobsProtocolHeadersKey, additionalHeaders);
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .CopyFromUrlBlob(options.CopySource, generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Accepted)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::CopyBlobFromUriResult response;
    if (pRawResponse->GetHeaders().count("ETag") != 0)
    {
      response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    }
    if (pRawResponse->GetHeaders().count("Last-Modified") != 0)
    {
      response.LastModified = DateTime::Parse(
          pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    }
    if (pRawResponse->GetHeaders().count("x-ms-version-id") != 0)
    {
      response.VersionId = pRawResponse->GetHeaders().at("x-ms-version-id");
    }
    response.CopyId = pRawResponse->GetHeaders().at("x-ms-copy-id");
    response.CopyStatus
        = PublicModels::CopyStatus(pRawResponse->GetHeaders().at("x-ms-copy-status"));
    if (pRawResponse->GetHeaders().count("Content-MD5") != 0)
    {
      response.TransactionalContentHash = ContentHash();
      response.TransactionalContentHash.Value().Value
          = Core::Convert::Base64Decode(pRawResponse->GetHeaders().at("Content-MD5"));
      response.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Md5;
    }
    if (pRawResponse->GetHeaders().count("x-ms-content-crc64") != 0)
    {
      response.TransactionalContentHash = ContentHash();
      response.TransactionalContentHash.Value().Value
          = Core::Convert::Base64Decode(pRawResponse->GetHeaders().at("x-ms-content-crc64"));
      response.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Crc64;
    }
    if (pRawResponse->GetHeaders().count("x-ms-encryption-scope") != 0)
    {
      response.EncryptionScope = pRawResponse->GetHeaders().at("x-ms-encryption-scope");
    }
    return Response<PublicModels::CopyBlobFromUriResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::AbortBlobCopyFromUriResult> BlobClient::AbortCopyFromUri(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const AbortBlobCopyFromUriOptions& options,
      const Core::Context& context)
  {
    BlobsClientAbortCopyFromUrlBlobOptions generatedOptions;
    generatedOptions.XMsLeaseId = NonEmpty(options.LeaseId);
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .AbortCopyFromUrlBlob(options.CopyId, generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::NoContent)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::AbortBlobCopyFromUriResult response;
    return Response<PublicModels::AbortBlobCopyFromUriResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::SetBlobAccessTierResult> BlobClient::SetTier(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const SetBlobTierOptions& options,
      const Core::Context& context)
  {
    BlobsClientSetTierBlobOptions generatedOptions;
    generatedOptions.Snapshot = NonEmpty(options.Snapshot);
    generatedOptions.Versionid = NonEmpty(options.VersionId);
    generatedOptions.XMsRehydratePriority
        = Enum<Models::RehydratePriority>(options.RehydratePriority);
    generatedOptions.XMsLeaseId = NonEmpty(options.LeaseId);
    generatedOptions.XMsIfTags = NonEmpty(options.IfTags);
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .SetTierBlob(Models::AccessTier(Text(options.Tier)), generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (!(httpStatusCode == Core::Http::HttpStatusCode::Ok
          || httpStatusCode == Core::Http::HttpStatusCode::Accepted))
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::SetBlobAccessTierResult response;
    return Response<PublicModels::SetBlobAccessTierResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::AccountInfo> BlobClient::GetAccountInfo(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const GetBlobAccountInfoOptions&,
      const Core::Context& context)
  {
    BlobsClientGetAccountInfoBlobOptions generatedOptions;
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url).GetAccountInfoBlob(generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::AccountInfo response;
    response.SkuName = PublicModels::SkuName(pRawResponse->GetHeaders().at("x-ms-sku-name"));
    response.AccountKind
        = PublicModels::AccountKind(pRawResponse->GetHeaders().at("x-ms-account-kind"));
    response.IsHierarchicalNamespaceEnabled
        = pRawResponse->GetHeaders().at("x-ms-is-hns-enabled") == std::string("true");
    return Response<PublicModels::AccountInfo>(std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::QueryBlobResult> BlobClient::Query(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const QueryBlobOptions& options,
      const Core::Context& context)
  {
    BlobsClientQueryBlockBlobOptions generatedOptions;
    generatedOptions.Snapshot = NonEmpty(options.Snapshot);
    generatedOptions.XMsLeaseId = NonEmpty(options.LeaseId);
    generatedOptions.XMsEncryptionKey = NonEmpty(options.EncryptionKey);
    generatedOptions.XMsEncryptionKeySha256 = Encoded(options.EncryptionKeySha256);
    generatedOptions.XMsEncryptionAlgorithm
        = Enum<Models::EncryptionAlgorithmType>(options.EncryptionAlgorithm);
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    generatedOptions.MatchConditions.IfMatch = options.IfMatch;
    generatedOptions.MatchConditions.IfNoneMatch = options.IfNoneMatch;
    generatedOptions.XMsIfTags = NonEmpty(options.IfTags);
    auto generatedContext = context;
    Core::CaseInsensitiveMap additionalHeaders;
    if (NonEmpty(options.EncryptionScope).HasValue())
    {
      additionalHeaders["x-ms-encryption-scope"] = options.EncryptionScope.Value();
    }
    generatedContext = generatedContext.WithValue(BlobsProtocolHeadersKey, additionalHeaders);
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .QueryBlockBlob(ToGenerated(options.QueryRequest), generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (!(httpStatusCode == Core::Http::HttpStatusCode::Ok
          || httpStatusCode == Core::Http::HttpStatusCode::PartialContent))
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::QueryBlobResult response;
    response.BodyStream = std::move(wire.Value.BodyStream);
    if (pRawResponse->GetHeaders().count("Last-Modified") != 0)
    {
      response.LastModified = DateTime::Parse(
          pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    }
    if (pRawResponse->GetHeaders().count("ETag") != 0)
    {
      response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-lease-duration") != 0)
    {
      response.LeaseDuration
          = PublicModels::LeaseDurationType(pRawResponse->GetHeaders().at("x-ms-lease-duration"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-lease-state") != 0)
    {
      response.LeaseState
          = PublicModels::LeaseState(pRawResponse->GetHeaders().at("x-ms-lease-state"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-lease-status") != 0)
    {
      response.LeaseStatus
          = PublicModels::LeaseStatus(pRawResponse->GetHeaders().at("x-ms-lease-status"));
    }
    response.IsServerEncrypted
        = pRawResponse->GetHeaders().at("x-ms-server-encrypted") == std::string("true");
    return Response<PublicModels::QueryBlobResult>(std::move(response), std::move(pRawResponse));
  }

  Response<std::map<std::string, std::string>> BlobClient::GetTags(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const GetBlobTagsOptions& options,
      const Core::Context& context)
  {
    BlobsClientGetTagsBlobOptions generatedOptions;
    generatedOptions.Snapshot = NonEmpty(options.Snapshot);
    generatedOptions.Versionid = NonEmpty(options.VersionId);
    generatedOptions.XMsLeaseId = NonEmpty(options.LeaseId);
    generatedOptions.XMsIfTags = NonEmpty(options.IfTags);
    generatedOptions.XMsBlobIfModifiedSince = options.IfModifiedSince;
    generatedOptions.XMsBlobIfUnmodifiedSince = options.IfUnmodifiedSince;
    generatedOptions.XMsBlobIfMatch = options.IfMatch;
    generatedOptions.XMsBlobIfNoneMatch = options.IfNoneMatch;
    auto generatedContext = context;
    auto wire = InvokeStorage(
        [&]() { return Client(pipeline, url).GetTagsBlob(generatedOptions, generatedContext); });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    auto response = FromGenerated(wire.Value);
    return Response<std::map<std::string, std::string>>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::SetBlobTagsResult> BlobClient::SetTags(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const SetBlobTagsOptions& options,
      const Core::Context& context)
  {
    BlobsClientSetTagsBlobOptions generatedOptions;
    generatedOptions.Versionid = NonEmpty(options.VersionId);
    generatedOptions.ContentMd5 = NonEmpty(options.TransactionalContentMD5);
    generatedOptions.XMsContentCrc64 = NonEmpty(options.TransactionalContentCrc64);
    generatedOptions.XMsIfTags = NonEmpty(options.IfTags);
    generatedOptions.XMsLeaseId = NonEmpty(options.LeaseId);
    generatedOptions.XMsBlobIfModifiedSince = options.IfModifiedSince;
    generatedOptions.XMsBlobIfUnmodifiedSince = options.IfUnmodifiedSince;
    generatedOptions.XMsBlobIfMatch = options.IfMatch;
    generatedOptions.XMsBlobIfNoneMatch = options.IfNoneMatch;
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .SetTagsBlob(ToGenerated(options.Tags), generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::NoContent)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::SetBlobTagsResult response;
    return Response<PublicModels::SetBlobTagsResult>(std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::CreatePageBlobResult> PageBlobClient::Create(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const CreatePageBlobOptions& options,
      const Core::Context& context)
  {
    BlobsClientCreatePageBlobOptions generatedOptions;
    generatedOptions.Metadata = Metadata(options.Metadata);
    generatedOptions.XMsAccessTier = Enum<Models::PremiumPageBlobAccessTier>(options.Tier);
    generatedOptions.XMsBlobContentType = NonEmpty(options.BlobContentType);
    generatedOptions.XMsBlobContentEncoding = NonEmpty(options.BlobContentEncoding);
    generatedOptions.XMsBlobContentLanguage = NonEmpty(options.BlobContentLanguage);
    generatedOptions.XMsBlobContentMd5 = NonEmpty(options.BlobContentMD5);
    generatedOptions.XMsBlobCacheControl = NonEmpty(options.BlobCacheControl);
    generatedOptions.XMsLeaseId = NonEmpty(options.LeaseId);
    generatedOptions.XMsBlobContentDisposition = NonEmpty(options.BlobContentDisposition);
    generatedOptions.XMsEncryptionKey = NonEmpty(options.EncryptionKey);
    generatedOptions.XMsEncryptionKeySha256 = Encoded(options.EncryptionKeySha256);
    generatedOptions.XMsEncryptionAlgorithm
        = Enum<Models::EncryptionAlgorithmType>(options.EncryptionAlgorithm);
    generatedOptions.XMsEncryptionScope = NonEmpty(options.EncryptionScope);
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    generatedOptions.MatchConditions.IfMatch = options.IfMatch;
    generatedOptions.MatchConditions.IfNoneMatch = options.IfNoneMatch;
    generatedOptions.XMsIfTags = NonEmpty(options.IfTags);
    generatedOptions.XMsBlobSequenceNumber = options.BlobSequenceNumber;
    generatedOptions.XMsTags = NonEmpty(options.BlobTagsString);
    generatedOptions.XMsImmutabilityPolicyUntilDate = options.ImmutabilityPolicyExpiry;
    generatedOptions.XMsImmutabilityPolicyMode
        = Enum<Models::ImmutabilityPolicyMode>(options.ImmutabilityPolicyMode);
    generatedOptions.XMsLegalHold = options.LegalHold;
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .CreatePageBlob(options.BlobContentLength, generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Created)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::CreatePageBlobResult response;
    if (pRawResponse->GetHeaders().count("ETag") != 0)
    {
      response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    }
    if (pRawResponse->GetHeaders().count("Last-Modified") != 0)
    {
      response.LastModified = DateTime::Parse(
          pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    }
    if (pRawResponse->GetHeaders().count("x-ms-version-id") != 0)
    {
      response.VersionId = pRawResponse->GetHeaders().at("x-ms-version-id");
    }
    response.IsServerEncrypted
        = pRawResponse->GetHeaders().at("x-ms-request-server-encrypted") == std::string("true");
    if (pRawResponse->GetHeaders().count("x-ms-encryption-key-sha256") != 0)
    {
      response.EncryptionKeySha256 = Core::Convert::Base64Decode(
          pRawResponse->GetHeaders().at("x-ms-encryption-key-sha256"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-encryption-scope") != 0)
    {
      response.EncryptionScope = pRawResponse->GetHeaders().at("x-ms-encryption-scope");
    }
    return Response<PublicModels::CreatePageBlobResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::UploadPagesResult> PageBlobClient::UploadPages(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      Core::IO::BodyStream& requestBody,
      const UploadPageBlobPagesOptions& options,
      const Core::Context& context)
  {
    BlobsClientUploadPagesPageBlobOptions generatedOptions;
    generatedOptions.ContentMd5 = NonEmpty(options.TransactionalContentMD5);
    generatedOptions.XMsContentCrc64 = NonEmpty(options.TransactionalContentCrc64);
    generatedOptions.XMsLeaseId = NonEmpty(options.LeaseId);
    generatedOptions.XMsEncryptionKey = NonEmpty(options.EncryptionKey);
    generatedOptions.XMsEncryptionKeySha256 = Encoded(options.EncryptionKeySha256);
    generatedOptions.XMsEncryptionAlgorithm
        = Enum<Models::EncryptionAlgorithmType>(options.EncryptionAlgorithm);
    generatedOptions.XMsEncryptionScope = NonEmpty(options.EncryptionScope);
    generatedOptions.XMsIfSequenceNumberLe = options.IfSequenceNumberLessThanOrEqualTo;
    generatedOptions.XMsIfSequenceNumberLt = options.IfSequenceNumberLessThan;
    generatedOptions.XMsIfSequenceNumberEq = options.IfSequenceNumberEqualTo;
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    generatedOptions.MatchConditions.IfMatch = options.IfMatch;
    generatedOptions.MatchConditions.IfNoneMatch = options.IfNoneMatch;
    generatedOptions.XMsIfTags = NonEmpty(options.IfTags);
    generatedOptions.XMsStructuredBody = NonEmpty(options.StructuredBodyType);
    generatedOptions.XMsStructuredContentLength = options.StructuredContentLength;
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .UploadPagesPageBlob(
              requestBody.Length(),
              Required(options.Range),
              requestBody,
              generatedOptions,
              generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Created)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::UploadPagesResult response;
    if (pRawResponse->GetHeaders().count("ETag") != 0)
    {
      response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    }
    if (pRawResponse->GetHeaders().count("Last-Modified") != 0)
    {
      response.LastModified = DateTime::Parse(
          pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    }
    if (pRawResponse->GetHeaders().count("Content-MD5") != 0)
    {
      response.TransactionalContentHash = ContentHash();
      response.TransactionalContentHash.Value().Value
          = Core::Convert::Base64Decode(pRawResponse->GetHeaders().at("Content-MD5"));
      response.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Md5;
    }
    if (pRawResponse->GetHeaders().count("x-ms-content-crc64") != 0)
    {
      response.AdditionalTransactionalContentHash = ContentHash();
      response.AdditionalTransactionalContentHash.Value().Value
          = Core::Convert::Base64Decode(pRawResponse->GetHeaders().at("x-ms-content-crc64"));
      response.AdditionalTransactionalContentHash.Value().Algorithm = HashAlgorithm::Crc64;
    }
    if (pRawResponse->GetHeaders().count("x-ms-blob-sequence-number") != 0)
    {
      response.SequenceNumber
          = std::stoll(pRawResponse->GetHeaders().at("x-ms-blob-sequence-number"));
    }
    response.IsServerEncrypted
        = pRawResponse->GetHeaders().at("x-ms-request-server-encrypted") == std::string("true");
    if (pRawResponse->GetHeaders().count("x-ms-encryption-key-sha256") != 0)
    {
      response.EncryptionKeySha256 = Core::Convert::Base64Decode(
          pRawResponse->GetHeaders().at("x-ms-encryption-key-sha256"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-encryption-scope") != 0)
    {
      response.EncryptionScope = pRawResponse->GetHeaders().at("x-ms-encryption-scope");
    }
    return Response<PublicModels::UploadPagesResult>(std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::ClearPagesResult> PageBlobClient::ClearPages(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const ClearPageBlobPagesOptions& options,
      const Core::Context& context)
  {
    BlobsClientClearPagesPageBlobOptions generatedOptions;
    generatedOptions.XMsLeaseId = NonEmpty(options.LeaseId);
    generatedOptions.XMsEncryptionKey = NonEmpty(options.EncryptionKey);
    generatedOptions.XMsEncryptionKeySha256 = Encoded(options.EncryptionKeySha256);
    generatedOptions.XMsEncryptionAlgorithm
        = Enum<Models::EncryptionAlgorithmType>(options.EncryptionAlgorithm);
    generatedOptions.XMsEncryptionScope = NonEmpty(options.EncryptionScope);
    generatedOptions.XMsIfSequenceNumberLe = options.IfSequenceNumberLessThanOrEqualTo;
    generatedOptions.XMsIfSequenceNumberLt = options.IfSequenceNumberLessThan;
    generatedOptions.XMsIfSequenceNumberEq = options.IfSequenceNumberEqualTo;
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    generatedOptions.MatchConditions.IfMatch = options.IfMatch;
    generatedOptions.MatchConditions.IfNoneMatch = options.IfNoneMatch;
    generatedOptions.XMsIfTags = NonEmpty(options.IfTags);
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .ClearPagesPageBlob(Required(options.Range), generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Created)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::ClearPagesResult response;
    if (pRawResponse->GetHeaders().count("ETag") != 0)
    {
      response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    }
    if (pRawResponse->GetHeaders().count("Last-Modified") != 0)
    {
      response.LastModified = DateTime::Parse(
          pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    }
    if (pRawResponse->GetHeaders().count("x-ms-blob-sequence-number") != 0)
    {
      response.SequenceNumber
          = std::stoll(pRawResponse->GetHeaders().at("x-ms-blob-sequence-number"));
    }
    return Response<PublicModels::ClearPagesResult>(std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::UploadPagesFromUriResult> PageBlobClient::UploadPagesFromUri(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const UploadPageBlobPagesFromUriOptions& options,
      const Core::Context& context)
  {
    BlobsClientUploadPagesFromUrlPageBlobOptions generatedOptions;
    generatedOptions.XMsSourceContentMd5 = NonEmpty(options.SourceContentMD5);
    generatedOptions.XMsSourceContentCrc64 = NonEmpty(options.SourceContentcrc64);
    generatedOptions.XMsEncryptionKey = NonEmpty(options.EncryptionKey);
    generatedOptions.XMsEncryptionKeySha256 = Encoded(options.EncryptionKeySha256);
    generatedOptions.XMsEncryptionAlgorithm
        = Enum<Models::EncryptionAlgorithmType>(options.EncryptionAlgorithm);
    generatedOptions.XMsEncryptionScope = NonEmpty(options.EncryptionScope);
    generatedOptions.XMsLeaseId = NonEmpty(options.LeaseId);
    generatedOptions.XMsIfSequenceNumberLe = options.IfSequenceNumberLessThanOrEqualTo;
    generatedOptions.XMsIfSequenceNumberLt = options.IfSequenceNumberLessThan;
    generatedOptions.XMsIfSequenceNumberEq = options.IfSequenceNumberEqualTo;
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    generatedOptions.MatchConditions.IfMatch = options.IfMatch;
    generatedOptions.MatchConditions.IfNoneMatch = options.IfNoneMatch;
    generatedOptions.XMsIfTags = NonEmpty(options.IfTags);
    generatedOptions.XMsSourceIfModifiedSince = options.SourceIfModifiedSince;
    generatedOptions.XMsSourceIfUnmodifiedSince = options.SourceIfUnmodifiedSince;
    generatedOptions.XMsSourceIfMatch = options.SourceIfMatch;
    generatedOptions.XMsSourceIfNoneMatch = options.SourceIfNoneMatch;
    generatedOptions.XMsCopySourceAuthorization = NonEmpty(options.CopySourceAuthorization);
    generatedOptions.XMsFileRequestIntent
        = Enum<Models::FileShareTokenIntent>(options.FileRequestIntent);
    generatedOptions.XMsSourceEncryptionKey = NonEmpty(options.SourceEncryptionKey);
    generatedOptions.XMsSourceEncryptionKeySha256 = Encoded(options.SourceEncryptionKeySha256);
    generatedOptions.XMsSourceEncryptionAlgorithm
        = Enum<Models::EncryptionAlgorithmType>(options.SourceEncryptionAlgorithm);
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .UploadPagesFromUrlPageBlob(
              options.SourceUrl,
              options.SourceRange,
              0,
              options.Range,
              generatedOptions,
              generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Created)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::UploadPagesFromUriResult response;
    if (pRawResponse->GetHeaders().count("ETag") != 0)
    {
      response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    }
    if (pRawResponse->GetHeaders().count("Last-Modified") != 0)
    {
      response.LastModified = DateTime::Parse(
          pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    }
    if (pRawResponse->GetHeaders().count("Content-MD5") != 0)
    {
      response.TransactionalContentHash = ContentHash();
      response.TransactionalContentHash.Value().Value
          = Core::Convert::Base64Decode(pRawResponse->GetHeaders().at("Content-MD5"));
      response.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Md5;
    }
    if (pRawResponse->GetHeaders().count("x-ms-content-crc64") != 0)
    {
      response.AdditionalTransactionalContentHash = ContentHash();
      response.AdditionalTransactionalContentHash.Value().Value
          = Core::Convert::Base64Decode(pRawResponse->GetHeaders().at("x-ms-content-crc64"));
      response.AdditionalTransactionalContentHash.Value().Algorithm = HashAlgorithm::Crc64;
    }
    if (pRawResponse->GetHeaders().count("x-ms-blob-sequence-number") != 0)
    {
      response.SequenceNumber
          = std::stoll(pRawResponse->GetHeaders().at("x-ms-blob-sequence-number"));
    }
    response.IsServerEncrypted
        = pRawResponse->GetHeaders().at("x-ms-request-server-encrypted") == std::string("true");
    if (pRawResponse->GetHeaders().count("x-ms-encryption-key-sha256") != 0)
    {
      response.EncryptionKeySha256 = Core::Convert::Base64Decode(
          pRawResponse->GetHeaders().at("x-ms-encryption-key-sha256"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-encryption-scope") != 0)
    {
      response.EncryptionScope = pRawResponse->GetHeaders().at("x-ms-encryption-scope");
    }
    return Response<PublicModels::UploadPagesFromUriResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::_detail::GetPageRangesResult> PageBlobClient::GetPageRanges(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const GetPageBlobPageRangesOptions& options,
      const Core::Context& context)
  {
    BlobsClientGetPageRangesPageBlobOptions generatedOptions;
    generatedOptions.Snapshot = NonEmpty(options.Snapshot);
    generatedOptions.Range = NonEmpty(options.Range);
    generatedOptions.XMsLeaseId = NonEmpty(options.LeaseId);
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    generatedOptions.MatchConditions.IfMatch = options.IfMatch;
    generatedOptions.MatchConditions.IfNoneMatch = options.IfNoneMatch;
    generatedOptions.XMsIfTags = NonEmpty(options.IfTags);
    generatedOptions.Marker = NonEmpty(options.Marker);
    generatedOptions.Maxresults = options.MaxResults;
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url).GetPageRangesPageBlob(generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    auto response = PageRanges<PublicModels::_detail::GetPageRangesResult>(wire.Value);
    if (pRawResponse->GetHeaders().count("Last-Modified") != 0)
    {
      response.LastModified = DateTime::Parse(
          pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    }
    if (pRawResponse->GetHeaders().count("ETag") != 0)
    {
      response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    }
    response.BlobSize = std::stoll(pRawResponse->GetHeaders().at("x-ms-blob-content-length"));
    return Response<PublicModels::_detail::GetPageRangesResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::_detail::GetPageRangesDiffResult> PageBlobClient::GetPageRangesDiff(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const GetPageBlobPageRangesDiffOptions& options,
      const Core::Context& context)
  {
    BlobsClientGetPageRangesDiffPageBlobOptions generatedOptions;
    generatedOptions.Snapshot = NonEmpty(options.Snapshot);
    generatedOptions.Prevsnapshot = NonEmpty(options.Prevsnapshot);
    generatedOptions.XMsPreviousSnapshotUrl = NonEmpty(options.PrevSnapshotUrl);
    generatedOptions.Range = NonEmpty(options.Range);
    generatedOptions.XMsLeaseId = NonEmpty(options.LeaseId);
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    generatedOptions.MatchConditions.IfMatch = options.IfMatch;
    generatedOptions.MatchConditions.IfNoneMatch = options.IfNoneMatch;
    generatedOptions.XMsIfTags = NonEmpty(options.IfTags);
    generatedOptions.Marker = NonEmpty(options.Marker);
    generatedOptions.Maxresults = options.MaxResults;
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url).GetPageRangesDiffPageBlob(generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    auto response = PageRanges<PublicModels::_detail::GetPageRangesDiffResult>(wire.Value);
    if (pRawResponse->GetHeaders().count("Last-Modified") != 0)
    {
      response.LastModified = DateTime::Parse(
          pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    }
    if (pRawResponse->GetHeaders().count("ETag") != 0)
    {
      response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    }
    response.BlobSize = std::stoll(pRawResponse->GetHeaders().at("x-ms-blob-content-length"));
    return Response<PublicModels::_detail::GetPageRangesDiffResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::ResizePageBlobResult> PageBlobClient::Resize(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const ResizePageBlobOptions& options,
      const Core::Context& context)
  {
    BlobsClientResizePageBlobOptions generatedOptions;
    generatedOptions.XMsLeaseId = NonEmpty(options.LeaseId);
    generatedOptions.XMsEncryptionKey = NonEmpty(options.EncryptionKey);
    generatedOptions.XMsEncryptionKeySha256 = Encoded(options.EncryptionKeySha256);
    generatedOptions.XMsEncryptionAlgorithm
        = Enum<Models::EncryptionAlgorithmType>(options.EncryptionAlgorithm);
    generatedOptions.XMsEncryptionScope = NonEmpty(options.EncryptionScope);
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    generatedOptions.MatchConditions.IfMatch = options.IfMatch;
    generatedOptions.MatchConditions.IfNoneMatch = options.IfNoneMatch;
    generatedOptions.XMsIfTags = NonEmpty(options.IfTags);
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .ResizePageBlob(options.BlobContentLength, generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::ResizePageBlobResult response;
    if (pRawResponse->GetHeaders().count("ETag") != 0)
    {
      response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    }
    if (pRawResponse->GetHeaders().count("Last-Modified") != 0)
    {
      response.LastModified = DateTime::Parse(
          pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    }
    response.SequenceNumber
        = std::stoll(pRawResponse->GetHeaders().at("x-ms-blob-sequence-number"));
    return Response<PublicModels::ResizePageBlobResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::UpdateSequenceNumberResult> PageBlobClient::UpdateSequenceNumber(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const UpdatePageBlobSequenceNumberOptions& options,
      const Core::Context& context)
  {
    BlobsClientSetSequenceNumberPageBlobOptions generatedOptions;
    generatedOptions.XMsLeaseId = NonEmpty(options.LeaseId);
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    generatedOptions.MatchConditions.IfMatch = options.IfMatch;
    generatedOptions.MatchConditions.IfNoneMatch = options.IfNoneMatch;
    generatedOptions.XMsIfTags = NonEmpty(options.IfTags);
    generatedOptions.XMsBlobSequenceNumber = options.BlobSequenceNumber;
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .SetSequenceNumberPageBlob(
              Models::SequenceNumberActionType(Text(options.SequenceNumberAction)),
              generatedOptions,
              generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::UpdateSequenceNumberResult response;
    if (pRawResponse->GetHeaders().count("ETag") != 0)
    {
      response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    }
    if (pRawResponse->GetHeaders().count("Last-Modified") != 0)
    {
      response.LastModified = DateTime::Parse(
          pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    }
    response.SequenceNumber
        = std::stoll(pRawResponse->GetHeaders().at("x-ms-blob-sequence-number"));
    return Response<PublicModels::UpdateSequenceNumberResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::_detail::StartBlobCopyIncrementalResult>
  PageBlobClient::StartCopyIncremental(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const StartPageBlobCopyIncrementalOptions& options,
      const Core::Context& context)
  {
    BlobsClientCopyIncrementalPageBlobOptions generatedOptions;
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    generatedOptions.MatchConditions.IfMatch = options.IfMatch;
    generatedOptions.MatchConditions.IfNoneMatch = options.IfNoneMatch;
    generatedOptions.XMsIfTags = NonEmpty(options.IfTags);
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .CopyIncrementalPageBlob(options.CopySource, generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Accepted)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::_detail::StartBlobCopyIncrementalResult response;
    if (pRawResponse->GetHeaders().count("ETag") != 0)
    {
      response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    }
    if (pRawResponse->GetHeaders().count("Last-Modified") != 0)
    {
      response.LastModified = DateTime::Parse(
          pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    }
    response.CopyId = pRawResponse->GetHeaders().at("x-ms-copy-id");
    response.CopyStatus
        = PublicModels::CopyStatus(pRawResponse->GetHeaders().at("x-ms-copy-status"));
    if (pRawResponse->GetHeaders().count("x-ms-version-id") != 0)
    {
      response.VersionId = pRawResponse->GetHeaders().at("x-ms-version-id");
    }
    return Response<PublicModels::_detail::StartBlobCopyIncrementalResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::CreateAppendBlobResult> AppendBlobClient::Create(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const CreateAppendBlobOptions& options,
      const Core::Context& context)
  {
    BlobsClientCreateAppendBlobOptions generatedOptions;
    generatedOptions.Metadata = Metadata(options.Metadata);
    generatedOptions.XMsBlobContentType = NonEmpty(options.BlobContentType);
    generatedOptions.XMsBlobContentEncoding = NonEmpty(options.BlobContentEncoding);
    generatedOptions.XMsBlobContentLanguage = NonEmpty(options.BlobContentLanguage);
    generatedOptions.XMsBlobContentMd5 = NonEmpty(options.BlobContentMD5);
    generatedOptions.XMsBlobCacheControl = NonEmpty(options.BlobCacheControl);
    generatedOptions.XMsLeaseId = NonEmpty(options.LeaseId);
    generatedOptions.XMsBlobContentDisposition = NonEmpty(options.BlobContentDisposition);
    generatedOptions.XMsEncryptionKey = NonEmpty(options.EncryptionKey);
    generatedOptions.XMsEncryptionKeySha256 = Encoded(options.EncryptionKeySha256);
    generatedOptions.XMsEncryptionAlgorithm
        = Enum<Models::EncryptionAlgorithmType>(options.EncryptionAlgorithm);
    generatedOptions.XMsEncryptionScope = NonEmpty(options.EncryptionScope);
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    generatedOptions.MatchConditions.IfMatch = options.IfMatch;
    generatedOptions.MatchConditions.IfNoneMatch = options.IfNoneMatch;
    generatedOptions.XMsIfTags = NonEmpty(options.IfTags);
    generatedOptions.XMsTags = NonEmpty(options.BlobTagsString);
    generatedOptions.XMsImmutabilityPolicyUntilDate = options.ImmutabilityPolicyExpiry;
    generatedOptions.XMsImmutabilityPolicyMode
        = Enum<Models::ImmutabilityPolicyMode>(options.ImmutabilityPolicyMode);
    generatedOptions.XMsLegalHold = options.LegalHold;
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url).CreateAppendBlob(generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Created)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::CreateAppendBlobResult response;
    response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    response.LastModified = DateTime::Parse(
        pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    if (pRawResponse->GetHeaders().count("x-ms-version-id") != 0)
    {
      response.VersionId = pRawResponse->GetHeaders().at("x-ms-version-id");
    }
    response.IsServerEncrypted
        = pRawResponse->GetHeaders().at("x-ms-request-server-encrypted") == std::string("true");
    if (pRawResponse->GetHeaders().count("x-ms-encryption-key-sha256") != 0)
    {
      response.EncryptionKeySha256 = Core::Convert::Base64Decode(
          pRawResponse->GetHeaders().at("x-ms-encryption-key-sha256"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-encryption-scope") != 0)
    {
      response.EncryptionScope = pRawResponse->GetHeaders().at("x-ms-encryption-scope");
    }
    return Response<PublicModels::CreateAppendBlobResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::AppendBlockResult> AppendBlobClient::AppendBlock(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      Core::IO::BodyStream& requestBody,
      const AppendAppendBlobBlockOptions& options,
      const Core::Context& context)
  {
    BlobsClientAppendBlockAppendBlobOptions generatedOptions;
    generatedOptions.ContentMd5 = NonEmpty(options.TransactionalContentMD5);
    generatedOptions.XMsContentCrc64 = NonEmpty(options.TransactionalContentCrc64);
    generatedOptions.XMsLeaseId = NonEmpty(options.LeaseId);
    generatedOptions.XMsBlobConditionMaxsize = options.MaxSize;
    generatedOptions.XMsBlobConditionAppendpos = options.AppendPosition;
    generatedOptions.XMsEncryptionKey = NonEmpty(options.EncryptionKey);
    generatedOptions.XMsEncryptionKeySha256 = Encoded(options.EncryptionKeySha256);
    generatedOptions.XMsEncryptionAlgorithm
        = Enum<Models::EncryptionAlgorithmType>(options.EncryptionAlgorithm);
    generatedOptions.XMsEncryptionScope = NonEmpty(options.EncryptionScope);
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    generatedOptions.MatchConditions.IfMatch = options.IfMatch;
    generatedOptions.MatchConditions.IfNoneMatch = options.IfNoneMatch;
    generatedOptions.XMsIfTags = NonEmpty(options.IfTags);
    generatedOptions.XMsStructuredBody = NonEmpty(options.StructuredBodyType);
    generatedOptions.XMsStructuredContentLength = options.StructuredContentLength;
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .AppendBlockAppendBlob(
              requestBody.Length(), requestBody, generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Created)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::AppendBlockResult response;
    response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    response.LastModified = DateTime::Parse(
        pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    if (pRawResponse->GetHeaders().count("Content-MD5") != 0)
    {
      response.TransactionalContentHash = ContentHash();
      response.TransactionalContentHash.Value().Value
          = Core::Convert::Base64Decode(pRawResponse->GetHeaders().at("Content-MD5"));
      response.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Md5;
    }
    if (pRawResponse->GetHeaders().count("x-ms-content-crc64") != 0)
    {
      response.AdditionalTransactionalContentHash = ContentHash();
      response.AdditionalTransactionalContentHash.Value().Value
          = Core::Convert::Base64Decode(pRawResponse->GetHeaders().at("x-ms-content-crc64"));
      response.AdditionalTransactionalContentHash.Value().Algorithm = HashAlgorithm::Crc64;
    }
    response.AppendOffset = std::stoll(pRawResponse->GetHeaders().at("x-ms-blob-append-offset"));
    response.CommittedBlockCount
        = std::stoi(pRawResponse->GetHeaders().at("x-ms-blob-committed-block-count"));
    response.IsServerEncrypted
        = pRawResponse->GetHeaders().at("x-ms-request-server-encrypted") == std::string("true");
    if (pRawResponse->GetHeaders().count("x-ms-encryption-key-sha256") != 0)
    {
      response.EncryptionKeySha256 = Core::Convert::Base64Decode(
          pRawResponse->GetHeaders().at("x-ms-encryption-key-sha256"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-encryption-scope") != 0)
    {
      response.EncryptionScope = pRawResponse->GetHeaders().at("x-ms-encryption-scope");
    }
    return Response<PublicModels::AppendBlockResult>(std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::AppendBlockFromUriResult> AppendBlobClient::AppendBlockFromUri(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const AppendAppendBlobBlockFromUriOptions& options,
      const Core::Context& context)
  {
    BlobsClientAppendBlockFromUrlAppendBlobOptions generatedOptions;
    generatedOptions.XMsSourceRange = NonEmpty(options.SourceRange);
    generatedOptions.XMsSourceContentMd5 = NonEmpty(options.SourceContentMD5);
    generatedOptions.XMsSourceContentCrc64 = NonEmpty(options.SourceContentcrc64);
    generatedOptions.ContentMd5 = NonEmpty(options.TransactionalContentMD5);
    generatedOptions.XMsEncryptionKey = NonEmpty(options.EncryptionKey);
    generatedOptions.XMsEncryptionKeySha256 = Encoded(options.EncryptionKeySha256);
    generatedOptions.XMsEncryptionAlgorithm
        = Enum<Models::EncryptionAlgorithmType>(options.EncryptionAlgorithm);
    generatedOptions.XMsEncryptionScope = NonEmpty(options.EncryptionScope);
    generatedOptions.XMsLeaseId = NonEmpty(options.LeaseId);
    generatedOptions.XMsBlobConditionMaxsize = options.MaxSize;
    generatedOptions.XMsBlobConditionAppendpos = options.AppendPosition;
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    generatedOptions.MatchConditions.IfMatch = options.IfMatch;
    generatedOptions.MatchConditions.IfNoneMatch = options.IfNoneMatch;
    generatedOptions.XMsIfTags = NonEmpty(options.IfTags);
    generatedOptions.XMsSourceIfModifiedSince = options.SourceIfModifiedSince;
    generatedOptions.XMsSourceIfUnmodifiedSince = options.SourceIfUnmodifiedSince;
    generatedOptions.XMsSourceIfMatch = options.SourceIfMatch;
    generatedOptions.XMsSourceIfNoneMatch = options.SourceIfNoneMatch;
    generatedOptions.XMsCopySourceAuthorization = NonEmpty(options.CopySourceAuthorization);
    generatedOptions.XMsFileRequestIntent
        = Enum<Models::FileShareTokenIntent>(options.FileRequestIntent);
    generatedOptions.XMsSourceEncryptionKey = NonEmpty(options.SourceEncryptionKey);
    generatedOptions.XMsSourceEncryptionKeySha256 = Encoded(options.SourceEncryptionKeySha256);
    generatedOptions.XMsSourceEncryptionAlgorithm
        = Enum<Models::EncryptionAlgorithmType>(options.SourceEncryptionAlgorithm);
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .AppendBlockFromUrlAppendBlob(options.SourceUrl, 0, generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Created)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::AppendBlockFromUriResult response;
    response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    response.LastModified = DateTime::Parse(
        pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    if (pRawResponse->GetHeaders().count("Content-MD5") != 0)
    {
      response.TransactionalContentHash = ContentHash();
      response.TransactionalContentHash.Value().Value
          = Core::Convert::Base64Decode(pRawResponse->GetHeaders().at("Content-MD5"));
      response.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Md5;
    }
    if (pRawResponse->GetHeaders().count("x-ms-content-crc64") != 0)
    {
      response.AdditionalTransactionalContentHash = ContentHash();
      response.AdditionalTransactionalContentHash.Value().Value
          = Core::Convert::Base64Decode(pRawResponse->GetHeaders().at("x-ms-content-crc64"));
      response.AdditionalTransactionalContentHash.Value().Algorithm = HashAlgorithm::Crc64;
    }
    response.AppendOffset = std::stoll(pRawResponse->GetHeaders().at("x-ms-blob-append-offset"));
    response.CommittedBlockCount
        = std::stoi(pRawResponse->GetHeaders().at("x-ms-blob-committed-block-count"));
    if (pRawResponse->GetHeaders().count("x-ms-encryption-key-sha256") != 0)
    {
      response.EncryptionKeySha256 = Core::Convert::Base64Decode(
          pRawResponse->GetHeaders().at("x-ms-encryption-key-sha256"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-encryption-scope") != 0)
    {
      response.EncryptionScope = pRawResponse->GetHeaders().at("x-ms-encryption-scope");
    }
    response.IsServerEncrypted
        = pRawResponse->GetHeaders().at("x-ms-request-server-encrypted") == std::string("true");
    return Response<PublicModels::AppendBlockFromUriResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::SealAppendBlobResult> AppendBlobClient::Seal(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const SealAppendBlobOptions& options,
      const Core::Context& context)
  {
    BlobsClientSealAppendBlobOptions generatedOptions;
    generatedOptions.XMsLeaseId = NonEmpty(options.LeaseId);
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    generatedOptions.MatchConditions.IfMatch = options.IfMatch;
    generatedOptions.MatchConditions.IfNoneMatch = options.IfNoneMatch;
    generatedOptions.XMsBlobConditionAppendpos = options.AppendPosition;
    auto generatedContext = context;
    auto wire = InvokeStorage(
        [&]() { return Client(pipeline, url).SealAppendBlob(generatedOptions, generatedContext); });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::SealAppendBlobResult response;
    response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    response.LastModified = DateTime::Parse(
        pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    response.IsSealed = pRawResponse->GetHeaders().at("x-ms-blob-sealed") == std::string("true");
    return Response<PublicModels::SealAppendBlobResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::UploadBlockBlobResult> BlockBlobClient::Upload(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      Core::IO::BodyStream& requestBody,
      const UploadBlockBlobOptions& options,
      const Core::Context& context)
  {
    BlobsClientUploadBlockBlobOptions generatedOptions;
    generatedOptions.Metadata = Metadata(options.Metadata);
    generatedOptions.ContentMd5 = NonEmpty(options.TransactionalContentMD5);
    generatedOptions.XMsBlobContentType = NonEmpty(options.BlobContentType);
    generatedOptions.XMsBlobContentEncoding = NonEmpty(options.BlobContentEncoding);
    generatedOptions.XMsBlobContentLanguage = NonEmpty(options.BlobContentLanguage);
    generatedOptions.XMsBlobContentMd5 = NonEmpty(options.BlobContentMD5);
    generatedOptions.XMsBlobCacheControl = NonEmpty(options.BlobCacheControl);
    generatedOptions.XMsLeaseId = NonEmpty(options.LeaseId);
    generatedOptions.XMsBlobContentDisposition = NonEmpty(options.BlobContentDisposition);
    generatedOptions.XMsEncryptionKey = NonEmpty(options.EncryptionKey);
    generatedOptions.XMsEncryptionKeySha256 = Encoded(options.EncryptionKeySha256);
    generatedOptions.XMsEncryptionAlgorithm
        = Enum<Models::EncryptionAlgorithmType>(options.EncryptionAlgorithm);
    generatedOptions.XMsEncryptionScope = NonEmpty(options.EncryptionScope);
    generatedOptions.XMsAccessTier = Enum<Models::AccessTier>(options.Tier);
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    generatedOptions.MatchConditions.IfMatch = options.IfMatch;
    generatedOptions.MatchConditions.IfNoneMatch = options.IfNoneMatch;
    generatedOptions.XMsIfTags = NonEmpty(options.IfTags);
    generatedOptions.XMsTags = NonEmpty(options.BlobTagsString);
    generatedOptions.XMsImmutabilityPolicyUntilDate = options.ImmutabilityPolicyExpiry;
    generatedOptions.XMsImmutabilityPolicyMode
        = Enum<Models::ImmutabilityPolicyMode>(options.ImmutabilityPolicyMode);
    generatedOptions.XMsLegalHold = options.LegalHold;
    generatedOptions.XMsContentCrc64 = NonEmpty(options.TransactionalContentCrc64);
    generatedOptions.XMsStructuredBody = NonEmpty(options.StructuredBodyType);
    generatedOptions.XMsStructuredContentLength = options.StructuredContentLength;
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .UploadBlockBlob(requestBody.Length(), requestBody, generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Created)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::UploadBlockBlobResult response;
    response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    response.LastModified = DateTime::Parse(
        pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    if (pRawResponse->GetHeaders().count("Content-MD5") != 0)
    {
      response.TransactionalContentHash = ContentHash();
      response.TransactionalContentHash.Value().Value
          = Core::Convert::Base64Decode(pRawResponse->GetHeaders().at("Content-MD5"));
      response.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Md5;
    }
    if (pRawResponse->GetHeaders().count("x-ms-content-crc64") != 0)
    {
      response.AdditionalTransactionalContentHash = ContentHash();
      response.AdditionalTransactionalContentHash.Value().Value
          = Core::Convert::Base64Decode(pRawResponse->GetHeaders().at("x-ms-content-crc64"));
      response.AdditionalTransactionalContentHash.Value().Algorithm = HashAlgorithm::Crc64;
    }
    if (pRawResponse->GetHeaders().count("x-ms-version-id") != 0)
    {
      response.VersionId = pRawResponse->GetHeaders().at("x-ms-version-id");
    }
    response.IsServerEncrypted
        = pRawResponse->GetHeaders().at("x-ms-request-server-encrypted") == std::string("true");
    if (pRawResponse->GetHeaders().count("x-ms-encryption-key-sha256") != 0)
    {
      response.EncryptionKeySha256 = Core::Convert::Base64Decode(
          pRawResponse->GetHeaders().at("x-ms-encryption-key-sha256"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-encryption-scope") != 0)
    {
      response.EncryptionScope = pRawResponse->GetHeaders().at("x-ms-encryption-scope");
    }
    return Response<PublicModels::UploadBlockBlobResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::UploadBlockBlobFromUriResult> BlockBlobClient::UploadFromUri(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const UploadBlockBlobFromUriOptions& options,
      const Core::Context& context)
  {
    BlobsClientUploadBlobFromUrlBlockBlobOptions generatedOptions;
    generatedOptions.Metadata = Metadata(options.Metadata);
    generatedOptions.XMsBlobContentType = NonEmpty(options.BlobContentType);
    generatedOptions.XMsBlobContentEncoding = NonEmpty(options.BlobContentEncoding);
    generatedOptions.XMsBlobContentLanguage = NonEmpty(options.BlobContentLanguage);
    generatedOptions.XMsBlobContentMd5 = NonEmpty(options.BlobContentMD5);
    generatedOptions.XMsBlobCacheControl = NonEmpty(options.BlobCacheControl);
    generatedOptions.XMsLeaseId = NonEmpty(options.LeaseId);
    generatedOptions.XMsBlobContentDisposition = NonEmpty(options.BlobContentDisposition);
    generatedOptions.XMsEncryptionKey = NonEmpty(options.EncryptionKey);
    generatedOptions.XMsEncryptionKeySha256 = Encoded(options.EncryptionKeySha256);
    generatedOptions.XMsEncryptionAlgorithm
        = Enum<Models::EncryptionAlgorithmType>(options.EncryptionAlgorithm);
    generatedOptions.XMsEncryptionScope = NonEmpty(options.EncryptionScope);
    generatedOptions.XMsAccessTier = Enum<Models::AccessTier>(options.Tier);
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    generatedOptions.MatchConditions.IfMatch = options.IfMatch;
    generatedOptions.MatchConditions.IfNoneMatch = options.IfNoneMatch;
    generatedOptions.XMsIfTags = NonEmpty(options.IfTags);
    generatedOptions.XMsSourceIfModifiedSince = options.SourceIfModifiedSince;
    generatedOptions.XMsSourceIfUnmodifiedSince = options.SourceIfUnmodifiedSince;
    generatedOptions.XMsSourceIfMatch = options.SourceIfMatch;
    generatedOptions.XMsSourceIfNoneMatch = options.SourceIfNoneMatch;
    generatedOptions.XMsSourceIfTags = NonEmpty(options.SourceIfTags);
    generatedOptions.XMsSourceContentMd5 = NonEmpty(options.SourceContentMD5);
    generatedOptions.XMsTags = NonEmpty(options.BlobTagsString);
    generatedOptions.XMsCopySourceBlobProperties = options.CopySourceBlobProperties;
    generatedOptions.XMsCopySourceAuthorization = NonEmpty(options.CopySourceAuthorization);
    generatedOptions.XMsCopySourceTagOption
        = Enum<Models::BlobCopySourceTags>(options.CopySourceTags);
    generatedOptions.XMsFileRequestIntent
        = Enum<Models::FileShareTokenIntent>(options.FileRequestIntent);
    generatedOptions.XMsSourceEncryptionKey = NonEmpty(options.SourceEncryptionKey);
    generatedOptions.XMsSourceEncryptionKeySha256 = Encoded(options.SourceEncryptionKeySha256);
    generatedOptions.XMsSourceEncryptionAlgorithm
        = Enum<Models::EncryptionAlgorithmType>(options.SourceEncryptionAlgorithm);
    auto generatedContext = context;
    Core::CaseInsensitiveMap additionalHeaders;
    if (NonEmpty(options.SourceContentcrc64).HasValue())
    {
      additionalHeaders["x-ms-source-content-crc64"]
          = Core::Convert::Base64Encode(options.SourceContentcrc64.Value());
    }
    generatedContext = generatedContext.WithValue(BlobsProtocolHeadersKey, additionalHeaders);
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .UploadBlobFromUrlBlockBlob(options.CopySource, generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Created)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::UploadBlockBlobFromUriResult response;
    response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    response.LastModified = DateTime::Parse(
        pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    if (pRawResponse->GetHeaders().count("Content-MD5") != 0)
    {
      response.TransactionalContentHash = ContentHash();
      response.TransactionalContentHash.Value().Value
          = Core::Convert::Base64Decode(pRawResponse->GetHeaders().at("Content-MD5"));
      response.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Md5;
    }
    if (pRawResponse->GetHeaders().count("x-ms-content-crc64") != 0)
    {
      response.AdditionalTransactionalContentHash = ContentHash();
      response.AdditionalTransactionalContentHash.Value().Value
          = Core::Convert::Base64Decode(pRawResponse->GetHeaders().at("x-ms-content-crc64"));
      response.AdditionalTransactionalContentHash.Value().Algorithm = HashAlgorithm::Crc64;
    }
    if (pRawResponse->GetHeaders().count("x-ms-version-id") != 0)
    {
      response.VersionId = pRawResponse->GetHeaders().at("x-ms-version-id");
    }
    response.IsServerEncrypted
        = pRawResponse->GetHeaders().at("x-ms-request-server-encrypted") == std::string("true");
    if (pRawResponse->GetHeaders().count("x-ms-encryption-key-sha256") != 0)
    {
      response.EncryptionKeySha256 = Core::Convert::Base64Decode(
          pRawResponse->GetHeaders().at("x-ms-encryption-key-sha256"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-encryption-scope") != 0)
    {
      response.EncryptionScope = pRawResponse->GetHeaders().at("x-ms-encryption-scope");
    }
    return Response<PublicModels::UploadBlockBlobFromUriResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::StageBlockResult> BlockBlobClient::StageBlock(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      Core::IO::BodyStream& requestBody,
      const StageBlockBlobBlockOptions& options,
      const Core::Context& context)
  {
    BlobsClientStageBlockBlockBlobOptions generatedOptions;
    generatedOptions.ContentMd5 = NonEmpty(options.TransactionalContentMD5);
    generatedOptions.XMsContentCrc64 = NonEmpty(options.TransactionalContentCrc64);
    generatedOptions.XMsLeaseId = NonEmpty(options.LeaseId);
    generatedOptions.XMsEncryptionKey = NonEmpty(options.EncryptionKey);
    generatedOptions.XMsEncryptionKeySha256 = Encoded(options.EncryptionKeySha256);
    generatedOptions.XMsEncryptionAlgorithm
        = Enum<Models::EncryptionAlgorithmType>(options.EncryptionAlgorithm);
    generatedOptions.XMsEncryptionScope = NonEmpty(options.EncryptionScope);
    generatedOptions.XMsStructuredBody = NonEmpty(options.StructuredBodyType);
    generatedOptions.XMsStructuredContentLength = options.StructuredContentLength;
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .StageBlockBlockBlob(
              options.BlockId,
              requestBody.Length(),
              requestBody,
              generatedOptions,
              generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Created)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::StageBlockResult response;
    if (pRawResponse->GetHeaders().count("Content-MD5") != 0)
    {
      response.TransactionalContentHash = ContentHash();
      response.TransactionalContentHash.Value().Value
          = Core::Convert::Base64Decode(pRawResponse->GetHeaders().at("Content-MD5"));
      response.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Md5;
    }
    if (pRawResponse->GetHeaders().count("x-ms-content-crc64") != 0)
    {
      response.AdditionalTransactionalContentHash = ContentHash();
      response.AdditionalTransactionalContentHash.Value().Value
          = Core::Convert::Base64Decode(pRawResponse->GetHeaders().at("x-ms-content-crc64"));
      response.AdditionalTransactionalContentHash.Value().Algorithm = HashAlgorithm::Crc64;
    }
    response.IsServerEncrypted
        = pRawResponse->GetHeaders().at("x-ms-request-server-encrypted") == std::string("true");
    if (pRawResponse->GetHeaders().count("x-ms-encryption-key-sha256") != 0)
    {
      response.EncryptionKeySha256 = Core::Convert::Base64Decode(
          pRawResponse->GetHeaders().at("x-ms-encryption-key-sha256"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-encryption-scope") != 0)
    {
      response.EncryptionScope = pRawResponse->GetHeaders().at("x-ms-encryption-scope");
    }
    return Response<PublicModels::StageBlockResult>(std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::StageBlockFromUriResult> BlockBlobClient::StageBlockFromUri(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const StageBlockBlobBlockFromUriOptions& options,
      const Core::Context& context)
  {
    BlobsClientStageBlockFromUrlBlockBlobOptions generatedOptions;
    generatedOptions.XMsSourceRange = NonEmpty(options.SourceRange);
    generatedOptions.XMsSourceContentMd5 = NonEmpty(options.SourceContentMD5);
    generatedOptions.XMsSourceContentCrc64 = NonEmpty(options.SourceContentcrc64);
    generatedOptions.XMsEncryptionKey = NonEmpty(options.EncryptionKey);
    generatedOptions.XMsEncryptionKeySha256 = Encoded(options.EncryptionKeySha256);
    generatedOptions.XMsEncryptionAlgorithm
        = Enum<Models::EncryptionAlgorithmType>(options.EncryptionAlgorithm);
    generatedOptions.XMsEncryptionScope = NonEmpty(options.EncryptionScope);
    generatedOptions.XMsLeaseId = NonEmpty(options.LeaseId);
    generatedOptions.XMsSourceIfModifiedSince = options.SourceIfModifiedSince;
    generatedOptions.XMsSourceIfUnmodifiedSince = options.SourceIfUnmodifiedSince;
    generatedOptions.XMsSourceIfMatch = options.SourceIfMatch;
    generatedOptions.XMsSourceIfNoneMatch = options.SourceIfNoneMatch;
    generatedOptions.XMsCopySourceAuthorization = NonEmpty(options.CopySourceAuthorization);
    generatedOptions.XMsFileRequestIntent
        = Enum<Models::FileShareTokenIntent>(options.FileRequestIntent);
    generatedOptions.XMsSourceEncryptionKey = NonEmpty(options.SourceEncryptionKey);
    generatedOptions.XMsSourceEncryptionKeySha256 = Encoded(options.SourceEncryptionKeySha256);
    generatedOptions.XMsSourceEncryptionAlgorithm
        = Enum<Models::EncryptionAlgorithmType>(options.SourceEncryptionAlgorithm);
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .StageBlockFromUrlBlockBlob(
              options.BlockId, 0, options.SourceUrl, generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Created)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::StageBlockFromUriResult response;
    if (pRawResponse->GetHeaders().count("Content-MD5") != 0)
    {
      response.TransactionalContentHash = ContentHash();
      response.TransactionalContentHash.Value().Value
          = Core::Convert::Base64Decode(pRawResponse->GetHeaders().at("Content-MD5"));
      response.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Md5;
    }
    if (pRawResponse->GetHeaders().count("x-ms-content-crc64") != 0)
    {
      response.AdditionalTransactionalContentHash = ContentHash();
      response.AdditionalTransactionalContentHash.Value().Value
          = Core::Convert::Base64Decode(pRawResponse->GetHeaders().at("x-ms-content-crc64"));
      response.AdditionalTransactionalContentHash.Value().Algorithm = HashAlgorithm::Crc64;
    }
    response.IsServerEncrypted
        = pRawResponse->GetHeaders().at("x-ms-request-server-encrypted") == std::string("true");
    if (pRawResponse->GetHeaders().count("x-ms-encryption-key-sha256") != 0)
    {
      response.EncryptionKeySha256 = Core::Convert::Base64Decode(
          pRawResponse->GetHeaders().at("x-ms-encryption-key-sha256"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-encryption-scope") != 0)
    {
      response.EncryptionScope = pRawResponse->GetHeaders().at("x-ms-encryption-scope");
    }
    return Response<PublicModels::StageBlockFromUriResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::CommitBlockListResult> BlockBlobClient::CommitBlockList(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const CommitBlockBlobBlockListOptions& options,
      const Core::Context& context)
  {
    BlobsClientCommitBlockListBlockBlobOptions generatedOptions;
    generatedOptions.XMsBlobCacheControl = NonEmpty(options.BlobCacheControl);
    generatedOptions.XMsBlobContentType = NonEmpty(options.BlobContentType);
    generatedOptions.XMsBlobContentEncoding = NonEmpty(options.BlobContentEncoding);
    generatedOptions.XMsBlobContentLanguage = NonEmpty(options.BlobContentLanguage);
    generatedOptions.XMsBlobContentMd5 = NonEmpty(options.BlobContentMD5);
    generatedOptions.ContentMd5 = NonEmpty(options.TransactionalContentMD5);
    generatedOptions.XMsContentCrc64 = NonEmpty(options.TransactionalContentCrc64);
    generatedOptions.Metadata = Metadata(options.Metadata);
    generatedOptions.XMsLeaseId = NonEmpty(options.LeaseId);
    generatedOptions.XMsBlobContentDisposition = NonEmpty(options.BlobContentDisposition);
    generatedOptions.XMsEncryptionKey = NonEmpty(options.EncryptionKey);
    generatedOptions.XMsEncryptionKeySha256 = Encoded(options.EncryptionKeySha256);
    generatedOptions.XMsEncryptionAlgorithm
        = Enum<Models::EncryptionAlgorithmType>(options.EncryptionAlgorithm);
    generatedOptions.XMsEncryptionScope = NonEmpty(options.EncryptionScope);
    generatedOptions.XMsAccessTier = Enum<Models::AccessTier>(options.Tier);
    generatedOptions.IfModifiedSince = options.IfModifiedSince;
    generatedOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    generatedOptions.MatchConditions.IfMatch = options.IfMatch;
    generatedOptions.MatchConditions.IfNoneMatch = options.IfNoneMatch;
    generatedOptions.XMsIfTags = NonEmpty(options.IfTags);
    generatedOptions.XMsTags = NonEmpty(options.BlobTagsString);
    generatedOptions.XMsImmutabilityPolicyUntilDate = options.ImmutabilityPolicyExpiry;
    generatedOptions.XMsImmutabilityPolicyMode
        = Enum<Models::ImmutabilityPolicyMode>(options.ImmutabilityPolicyMode);
    generatedOptions.XMsLegalHold = options.LegalHold;
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .CommitBlockListBlockBlob(
              ToGenerated(options.Blocks), generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Created)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    PublicModels::CommitBlockListResult response;
    response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    response.LastModified = DateTime::Parse(
        pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    if (pRawResponse->GetHeaders().count("Content-MD5") != 0)
    {
      response.TransactionalContentHash = ContentHash();
      response.TransactionalContentHash.Value().Value
          = Core::Convert::Base64Decode(pRawResponse->GetHeaders().at("Content-MD5"));
      response.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Md5;
    }
    if (pRawResponse->GetHeaders().count("x-ms-content-crc64") != 0)
    {
      response.TransactionalContentHash = ContentHash();
      response.TransactionalContentHash.Value().Value
          = Core::Convert::Base64Decode(pRawResponse->GetHeaders().at("x-ms-content-crc64"));
      response.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Crc64;
    }
    if (pRawResponse->GetHeaders().count("x-ms-version-id") != 0)
    {
      response.VersionId = pRawResponse->GetHeaders().at("x-ms-version-id");
    }
    response.IsServerEncrypted
        = pRawResponse->GetHeaders().at("x-ms-request-server-encrypted") == std::string("true");
    if (pRawResponse->GetHeaders().count("x-ms-encryption-key-sha256") != 0)
    {
      response.EncryptionKeySha256 = Core::Convert::Base64Decode(
          pRawResponse->GetHeaders().at("x-ms-encryption-key-sha256"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-encryption-scope") != 0)
    {
      response.EncryptionScope = pRawResponse->GetHeaders().at("x-ms-encryption-scope");
    }
    return Response<PublicModels::CommitBlockListResult>(
        std::move(response), std::move(pRawResponse));
  }

  Response<PublicModels::GetBlockListResult> BlockBlobClient::GetBlockList(
      Core::Http::_internal::HttpPipeline& pipeline,
      const Core::Url& url,
      const GetBlockBlobBlockListOptions& options,
      const Core::Context& context)
  {
    BlobsClientGetBlockListBlockBlobOptions generatedOptions;
    generatedOptions.Snapshot = NonEmpty(options.Snapshot);
    generatedOptions.XMsLeaseId = NonEmpty(options.LeaseId);
    generatedOptions.XMsIfTags = NonEmpty(options.IfTags);
    auto generatedContext = context;
    auto wire = InvokeStorage([&]() {
      return Client(pipeline, url)
          .GetBlockListBlockBlob(
              Models::BlockListType(Text(options.ListType)), generatedOptions, generatedContext);
    });
    auto pRawResponse = std::move(wire.RawResponse);
    auto httpStatusCode = pRawResponse->GetStatusCode();
    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw StorageException::CreateFromResponse(std::move(pRawResponse));
    }
    auto response = FromGenerated(wire.Value);
    if (pRawResponse->GetHeaders().count("Last-Modified") != 0)
    {
      response.LastModified = DateTime::Parse(
          pRawResponse->GetHeaders().at("Last-Modified"), Azure::DateTime::DateFormat::Rfc1123);
    }
    if (pRawResponse->GetHeaders().count("ETag") != 0)
    {
      response.ETag = ETag(pRawResponse->GetHeaders().at("ETag"));
    }
    if (pRawResponse->GetHeaders().count("x-ms-blob-content-length") != 0)
    {
      response.BlobSize = std::stoll(pRawResponse->GetHeaders().at("x-ms-blob-content-length"));
    }
    return Response<PublicModels::GetBlockListResult>(std::move(response), std::move(pRawResponse));
  }

}}}} // namespace Azure::Storage::Blobs::_detail

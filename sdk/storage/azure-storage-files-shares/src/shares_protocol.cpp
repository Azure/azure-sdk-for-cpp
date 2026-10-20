// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/core/base64.hpp>
#include <azure/core/exception.hpp>
#include <azure/storage/common/storage_exception.hpp>
#include <azure/storage/files/shares/rest_client.hpp>

#include <limits>
#include <stdexcept>

// Parse legacy declarations before the generated _detail::Models namespace can shadow Models.
#include "generated/file_shares_client.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares { namespace _detail {
  namespace PublicModels = Azure::Storage::Files::Shares::Models;
  namespace {
    using Pipeline = Core::Http::_internal::HttpPipeline;

    FileSharesClient Client(Pipeline& pipeline, Core::Url const& url)
    {
      // The legacy protocol signatures borrow the pipeline for one synchronous operation.
      return FileSharesClient(
          url, std::shared_ptr<Pipeline>(std::shared_ptr<Pipeline>{}, &pipeline));
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

    template <class T> T const& Required(Nullable<T> const& value, char const* name)
    {
      if (!value.HasValue())
      {
        throw std::out_of_range(std::string("Missing required response value: ") + name);
      }
      return value.Value();
    }

    template <class T> Nullable<T> NonEmpty(Nullable<T> const& value)
    {
      return value.HasValue() && !value.Value().empty() ? value : Nullable<T>{};
    }

    template <class Target, class Source> Nullable<Target> Enum(Nullable<Source> const& source)
    {
      return source.HasValue() && !source.Value().ToString().empty()
          ? Nullable<Target>(Target(source.Value().ToString()))
          : Nullable<Target>{};
    }

    template <class Target> Nullable<Target> Enum(Nullable<std::string> const& source)
    {
      return source.HasValue() ? Nullable<Target>(Target(source.Value())) : Nullable<Target>{};
    }

    template <class Target, class Source> Nullable<Target> Number(Nullable<Source> const& source)
    {
      if (!source.HasValue())
      {
        return {};
      }
      if (source.Value() < (std::numeric_limits<Target>::min)()
          || source.Value() > (std::numeric_limits<Target>::max)())
      {
        throw std::out_of_range("Response integer exceeds the public model's range.");
      }
      return static_cast<Target>(source.Value());
    }

    template <class T, class U> Response<T> Finish(T result, Response<U>& response)
    {
      return Response<T>(std::move(result), std::move(response.RawResponse));
    }

    template <class T, class U> T Entity(U const& source)
    {
      T result;
      result.ETag = Required(source.Etag, "ETag");
      result.LastModified = Required(source.LastModified, "Last-Modified");
      return result;
    }

    template <class T> PublicModels::FileSmbProperties FileIdentity(T const& source)
    {
      PublicModels::FileSmbProperties result;
      result.CreatedOn = source.FileCreationTime;
      result.LastWrittenOn = source.FileLastWriteTime;
      result.ChangedOn = source.FileChangeTime;
      result.FileId = Required(source.FileId, "x-ms-file-id");
      result.ParentFileId = Required(source.FileParentId, "x-ms-file-parent-id");
      return result;
    }

    template <class T> PublicModels::FileSmbProperties Smb(T const& source)
    {
      auto result = FileIdentity(source);
      result.PermissionKey = source.FilePermissionKey;
      if (source.FileAttributes.HasValue())
      {
        result.Attributes = PublicModels::FileAttributes(source.FileAttributes.Value());
      }
      return result;
    }

    template <class T, class U> T SmbEntity(U const& source)
    {
      auto result = Entity<T>(source);
      result.SmbProperties = Smb(source);
      return result;
    }

    template <class T, class U> void Posix(T& target, U const& source)
    {
      target.FileMode = source.FileMode;
      target.Owner = source.Owner;
      target.Group = source.Group;
    }

    template <class T, class U> void CopyProperties(T& target, U const& source)
    {
      target.CopyCompletedOn = source.CopyCompletionTime;
      target.CopyStatusDescription = source.CopyStatusDescription;
      target.CopyId = source.CopyId;
      target.CopyProgress = source.CopyProgress;
      target.CopySource = source.CopySource;
      target.CopyStatus = Enum<PublicModels::CopyStatus>(source.CopyStatus);
      target.LeaseDuration = Enum<PublicModels::LeaseDurationType>(source.LeaseDuration);
      target.LeaseState = Enum<PublicModels::LeaseState>(source.LeaseState);
      target.LeaseStatus = Enum<PublicModels::LeaseStatus>(source.LeaseStatus);
    }

    template <class T> PublicModels::FileHttpHeaders HttpHeaders(T const& source)
    {
      PublicModels::FileHttpHeaders result;
      if (source.ContentEncoding.HasValue())
      {
        result.ContentEncoding = source.ContentEncoding.Value();
      }
      if (source.ContentLanguage.HasValue())
      {
        result.ContentLanguage = source.ContentLanguage.Value();
      }
      if (source.CacheControl.HasValue())
      {
        result.CacheControl = source.CacheControl.Value();
      }
      if (source.ContentDisposition.HasValue())
      {
        result.ContentDisposition = source.ContentDisposition.Value();
      }
      return result;
    }

    Core::CaseInsensitiveMap Metadata(Nullable<Core::CaseInsensitiveMap> const& source)
    {
      return source.HasValue() ? source.Value() : Core::CaseInsensitiveMap{};
    }

    template <class T>
    void Intent(T& target, Nullable<PublicModels::ShareTokenIntent> const& intent)
    {
      target.XMsFileRequestIntent = Enum<Models::ShareTokenIntent>(intent);
    }

    template <class T, class U> void Permissions(T& target, U const& source)
    {
      target.XMsFilePermission = NonEmpty(source.FilePermission);
      target.XMsFilePermissionFormat
          = Enum<Models::FilePermissionFormat>(source.FilePermissionFormat);
      target.XMsFilePermissionKey = NonEmpty(source.FilePermissionKey);
      target.XMsFileCreationTime = NonEmpty(source.FileCreationTime);
      target.XMsFileLastWriteTime = NonEmpty(source.FileLastWriteTime);
      target.XMsFileChangeTime = NonEmpty(source.FileChangeTime);
    }

    template <class T, class U> void PosixOptions(T& target, U const& source)
    {
      target.XMsOwner = NonEmpty(source.Owner);
      target.XMsGroup = NonEmpty(source.Group);
      target.XMsMode = NonEmpty(source.FileMode);
    }

    template <class T, class U> void FileHeaders(T& target, U const& source)
    {
      target.XMsContentType = NonEmpty(source.FileContentType);
      target.XMsContentEncoding = NonEmpty(source.FileContentEncoding);
      target.XMsContentLanguage = NonEmpty(source.FileContentLanguage);
      target.XMsCacheControl = NonEmpty(source.FileCacheControl);
      target.XMsContentDisposition = NonEmpty(source.FileContentDisposition);
      target.XMsContentMd5 = NonEmpty(source.FileContentMD5);
    }

    template <class T, class U> void ShareOptions(T& target, U const& source)
    {
      target.XMsShareQuota = source.Quota;
      target.XMsAccessTier = Enum<Models::ShareAccessTier>(source.AccessTier);
      target.XMsEnabledProtocols = source.EnabledProtocols.HasValue()
          ? NonEmpty(Nullable<std::string>(source.EnabledProtocols.Value().ToString()))
          : Nullable<std::string>{};
      target.XMsRootSquash = Enum<Models::ShareRootSquash>(source.RootSquash);
      target.XMsEnableSnapshotVirtualDirectoryAccess = source.EnableSnapshotVirtualDirectoryAccess;
      target.XMsSharePaidBurstingEnabled = source.PaidBurstingEnabled;
      target.XMsSharePaidBurstingMaxBandwidthMibps = source.PaidBurstingMaxBandwidthMibps;
      target.XMsSharePaidBurstingMaxIops = source.PaidBurstingMaxIops;
      target.XMsShareProvisionedIops = source.ShareProvisionedIops;
      target.XMsShareProvisionedBandwidthMibps = source.ShareProvisionedBandwidthMibps;
      Intent(target, source.FileRequestIntent);
    }

    PublicModels::Metrics Metrics(Models::Metrics const& source)
    {
      PublicModels::Metrics result;
      result.Version = source.Version;
      result.Enabled = source.Enabled;
      result.IncludeApis = source.IncludeApis;
      if (source.RetentionPolicy.HasValue())
      {
        result.RetentionPolicy.Enabled = source.RetentionPolicy.Value().Enabled;
        result.RetentionPolicy.Days = source.RetentionPolicy.Value().Days;
      }
      return result;
    }

    Models::Metrics Metrics(PublicModels::Metrics const& source)
    {
      Models::Metrics result;
      result.Version = source.Version;
      result.Enabled = source.Enabled;
      result.IncludeApis = source.IncludeApis;
      Models::RetentionPolicy retention;
      retention.Enabled = source.RetentionPolicy.Enabled;
      retention.Days = source.RetentionPolicy.Days;
      result.RetentionPolicy = std::move(retention);
      return result;
    }

    template <class T, class U> void ServiceProperties(T& target, U const& source)
    {
      if (source.HourMetrics.HasValue())
      {
        target.HourMetrics = Metrics(source.HourMetrics.Value());
      }
      if (source.MinuteMetrics.HasValue())
      {
        target.MinuteMetrics = Metrics(source.MinuteMetrics.Value());
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
          target.Cors.emplace_back(std::move(item));
        }
      }
      if (source.Protocol.HasValue())
      {
        PublicModels::ProtocolSettings protocol;
        auto const& settings = source.Protocol.Value();
        if (settings.Smb.HasValue())
        {
          PublicModels::SmbSettings smb;
          if (settings.Smb.Value().Multichannel.HasValue())
          {
            PublicModels::SmbMultichannel multichannel;
            if (settings.Smb.Value().Multichannel.Value().Enabled.HasValue())
            {
              multichannel.Enabled = settings.Smb.Value().Multichannel.Value().Enabled.Value();
            }
            smb.Multichannel = multichannel;
          }
          if (settings.Smb.Value().EncryptionInTransit.HasValue())
          {
            PublicModels::SmbEncryptionInTransit encryption;
            if (settings.Smb.Value().EncryptionInTransit.Value().Required.HasValue())
            {
              encryption.Required
                  = settings.Smb.Value().EncryptionInTransit.Value().Required.Value();
            }
            smb.EncryptionInTransit = encryption;
          }
          protocol.Settings = std::move(smb);
        }
        if (settings.Nfs.HasValue())
        {
          PublicModels::NfsSettings nfs;
          if (settings.Nfs.Value().EncryptionInTransit.HasValue())
          {
            PublicModels::NfsEncryptionInTransit encryption;
            if (settings.Nfs.Value().EncryptionInTransit.Value().Required.HasValue())
            {
              encryption.Required
                  = settings.Nfs.Value().EncryptionInTransit.Value().Required.Value();
            }
            nfs.EncryptionInTransit = encryption;
          }
          protocol.NfsSettings = std::move(nfs);
        }
        target.Protocol = std::move(protocol);
      }
    }

    PublicModels::_detail::StringEncoded Encoded(Models::StringEncoded const& source)
    {
      PublicModels::_detail::StringEncoded result;
      if (source.Encoded.HasValue())
      {
        result.Encoded = source.Encoded.Value();
      }
      if (source.Content.HasValue())
      {
        result.Content = source.Content.Value();
      }
      return result;
    }

    template <class T> void ItemProperties(T& target, Models::FileProperty const& source)
    {
      target.LastAccessedOn = source.LastAccessTime;
      if (source.LastModified.HasValue())
      {
        target.LastModified = source.LastModified.Value();
      }
      if (source.ETag.HasValue())
      {
        target.Etag = source.ETag;
      }
      target.SmbProperties.CreatedOn = source.CreationTime;
      target.SmbProperties.LastWrittenOn = source.LastWriteTime;
      target.SmbProperties.ChangedOn = source.ChangeTime;
    }

    template <class T, class U> void ItemSmb(T& target, U const& source)
    {
      target.SmbProperties.PermissionKey = source.PermissionKey;
      if (source.Attributes.HasValue())
      {
        target.SmbProperties.Attributes = PublicModels::FileAttributes(source.Attributes.Value());
      }
      if (source.FileId.HasValue())
      {
        target.SmbProperties.FileId = source.FileId.Value();
      }
    }

    template <class T> PublicModels::_detail::ListHandlesResponse Handles(T const& source)
    {
      PublicModels::_detail::ListHandlesResponse result;
      result.NextMarker = source.NextMarker;
      if (source.HandleList.HasValue())
      {
        for (auto const& handle : source.HandleList.Value())
        {
          PublicModels::_detail::HandleItem item;
          item.HandleId = handle.HandleId;
          item.Path = Encoded(handle.Path);
          item.FileId = handle.FileId;
          if (handle.ParentId.HasValue())
          {
            item.ParentId = handle.ParentId.Value();
          }
          item.SessionId = handle.SessionId;
          item.ClientIp = handle.ClientIp;
          item.ClientName = handle.ClientName;
          item.OpenedOn = handle.OpenTime;
          if (handle.LastReconnectTime.HasValue())
          {
            item.LastReconnectedOn = handle.LastReconnectTime.Value();
          }
          if (handle.AccessRightList.HasValue())
          {
            for (auto const& right : handle.AccessRightList.Value())
            {
              item.AccessRightList.emplace_back(right.ToString());
            }
          }
          result.HandleList.emplace_back(std::move(item));
        }
      }
      return result;
    }

    template <class T, class U> void RenameOptions(T& target, U const& source)
    {
      Permissions(target, source);
      Intent(target, source.FileRequestIntent);
      target.XMsFileAttributes = NonEmpty(source.FileAttributes);
      target.XMsFileRenameReplaceIfExists = source.ReplaceIfExists;
      target.XMsFileRenameIgnoreReadonly = source.IgnoreReadOnly;
      target.XMsSourceLeaseId = NonEmpty(source.SourceLeaseId);
      target.XMsDestinationLeaseId = NonEmpty(source.DestinationLeaseId);
      target.Metadata = Core::CaseInsensitiveMap(source.Metadata.begin(), source.Metadata.end());
      target.XMsAllowTrailingDot = source.AllowTrailingDot;
      target.XMsSourceAllowTrailingDot = source.AllowSourceTrailingDot;
    }

    template <class T, class U> T RenameResult(U const& source)
    {
      auto result = Entity<T>(source);
      result.IsServerEncrypted
          = Required(source.RequestServerEncrypted, "x-ms-request-server-encrypted");
      result.FilePermissionKey = Required(source.FilePermissionKey, "x-ms-file-permission-key");
      result.FileAttributes = Required(source.FileAttributes, "x-ms-file-attributes");
      result.FileCreationTime = Required(source.FileCreationTime, "x-ms-file-creation-time");
      result.FileLastWriteTime = Required(source.FileLastWriteTime, "x-ms-file-last-write-time");
      result.FileChangeTime = Required(source.FileChangeTime, "x-ms-file-change-time");
      result.FileId = Required(source.FileId, "x-ms-file-id");
      result.FileParentId = Required(source.FileParentId, "x-ms-file-parent-id");
      return result;
    }
  } // namespace

  Response<PublicModels::SetServicePropertiesResult> ServiceClient::SetProperties(
      Pipeline& pipeline,
      Core::Url const& url,
      SetServicePropertiesOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientSetPropertiesServiceOptions generated;
    Intent(generated, options.FileRequestIntent);
    Models::StorageServiceProperties properties;
    properties.HourMetrics = Metrics(options.ShareServiceProperties.HourMetrics);
    properties.MinuteMetrics = Metrics(options.ShareServiceProperties.MinuteMetrics);
    std::vector<Models::CorsRule> cors;
    for (auto const& rule : options.ShareServiceProperties.Cors)
    {
      Models::CorsRule item;
      item.AllowedOrigins = rule.AllowedOrigins;
      item.AllowedMethods = rule.AllowedMethods;
      item.AllowedHeaders = rule.AllowedHeaders;
      item.ExposedHeaders = rule.ExposedHeaders;
      item.MaxAgeInSeconds = rule.MaxAgeInSeconds;
      cors.emplace_back(std::move(item));
    }
    properties.Cors = std::move(cors);
    if (options.ShareServiceProperties.Protocol.HasValue())
    {
      Models::ShareProtocolSettings protocol;
      auto const& settings = options.ShareServiceProperties.Protocol.Value();
      if (settings.Settings.HasValue())
      {
        Models::ShareSmbSettings smb;
        if (settings.Settings.Value().Multichannel.HasValue())
        {
          Models::SmbMultichannel multichannel;
          multichannel.Enabled = settings.Settings.Value().Multichannel.Value().Enabled;
          smb.Multichannel = multichannel;
        }
        if (settings.Settings.Value().EncryptionInTransit.HasValue())
        {
          Models::ShareSmbSettingsEncryptionInTransit encryption;
          encryption.Required = settings.Settings.Value().EncryptionInTransit.Value().Required;
          smb.EncryptionInTransit = encryption;
        }
        protocol.Smb = std::move(smb);
      }
      if (settings.NfsSettings.HasValue())
      {
        Models::ShareNfsSettings nfs;
        if (settings.NfsSettings.Value().EncryptionInTransit.HasValue())
        {
          Models::ShareNfsSettingsEncryptionInTransit encryption;
          encryption.Required = settings.NfsSettings.Value().EncryptionInTransit.Value().Required;
          nfs.EncryptionInTransit = encryption;
        }
        protocol.Nfs = std::move(nfs);
      }
      properties.Protocol = std::move(protocol);
    }
    auto response = InvokeStorage(
        [&] { return Client(pipeline, url).SetPropertiesService(properties, generated, context); });
    return Finish(PublicModels::SetServicePropertiesResult{}, response);
  }

  Response<PublicModels::ShareServiceProperties> ServiceClient::GetProperties(
      Pipeline& pipeline,
      Core::Url const& url,
      GetServicePropertiesOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientGetPropertiesServiceOptions generated;
    Intent(generated, options.FileRequestIntent);
    auto response = InvokeStorage(
        [&] { return Client(pipeline, url).GetPropertiesService(generated, context); });
    PublicModels::ShareServiceProperties result;
    ServiceProperties(result, response.Value);
    return Finish(std::move(result), response);
  }

  Response<PublicModels::_detail::ListSharesResponse> ServiceClient::ListSharesSegment(
      Pipeline& pipeline,
      Core::Url const& url,
      ListServiceSharesSegmentOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientListSharesSegmentServiceOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.Prefix = NonEmpty(options.Prefix);
    generated.Marker = NonEmpty(options.Marker);
    generated.Maxresults = options.MaxResults;
    std::vector<Models::ListSharesIncludeType> include;
    if (options.Include.HasValue())
    {
      for (auto const& flag :
           {std::make_pair(PublicModels::ListSharesIncludeFlags::Snapshots, "snapshots"),
            std::make_pair(PublicModels::ListSharesIncludeFlags::Metadata, "metadata"),
            std::make_pair(PublicModels::ListSharesIncludeFlags::Deleted, "deleted")})
      {
        if ((options.Include.Value() & flag.first) == flag.first)
        {
          include.emplace_back(flag.second);
        }
      }
    }
    if (!include.empty())
    {
      generated.Include = std::move(include);
    }
    auto response = InvokeStorage(
        [&] { return Client(pipeline, url).ListSharesSegmentService(generated, context); });
    PublicModels::_detail::ListSharesResponse result;
    result.ServiceEndpoint = response.Value.ServiceEndpoint;
    result.Prefix = response.Value.Prefix;
    result.Marker = response.Value.Marker;
    result.MaxResults = response.Value.MaxResults;
    result.NextMarker = response.Value.NextMarker;
    if (response.Value.ShareItems.HasValue())
    {
      for (auto const& share : response.Value.ShareItems.Value())
      {
        PublicModels::ShareItem item;
        item.Name = share.Name;
        if (share.Snapshot.HasValue())
        {
          item.Snapshot = share.Snapshot.Value();
        }
        if (share.Deleted.HasValue())
        {
          item.Deleted = share.Deleted.Value();
        }
        if (share.Version.HasValue())
        {
          item.Version = share.Version.Value();
        }
        if (share.Metadata.HasValue())
        {
          item.Metadata.insert(share.Metadata.Value().begin(), share.Metadata.Value().end());
        }
        auto const& source = share.Properties;
        auto& target = item.Details;
        target.LastModified = source.LastModified;
        target.Etag = source.ETag;
        target.Quota = source.Quota;
        target.ProvisionedIops = source.ProvisionedIops;
        target.ProvisionedIngressMBps = source.ProvisionedIngressMBps;
        target.ProvisionedEgressMBps = source.ProvisionedEgressMBps;
        target.ProvisionedBandwidthMBps = source.ProvisionedBandwidthMiBps;
        target.NextAllowedQuotaDowngradeTime = source.NextAllowedQuotaDowngradeTime;
        target.DeletedOn = source.DeletedTime;
        if (source.RemainingRetentionDays.HasValue())
        {
          target.RemainingRetentionDays = source.RemainingRetentionDays.Value();
        }
        target.AccessTier = Enum<PublicModels::AccessTier>(source.AccessTier);
        target.AccessTierChangedOn = source.AccessTierChangeTime;
        target.AccessTierTransitionState = source.AccessTierTransitionState;
        if (source.LeaseStatus.HasValue())
        {
          target.LeaseStatus = PublicModels::LeaseStatus(source.LeaseStatus.Value().ToString());
        }
        if (source.LeaseState.HasValue())
        {
          target.LeaseState = PublicModels::LeaseState(source.LeaseState.Value().ToString());
        }
        if (source.LeaseDuration.HasValue())
        {
          target.LeaseDuration
              = PublicModels::LeaseDurationType(source.LeaseDuration.Value().ToString());
        }
        target.EnabledProtocols = Enum<PublicModels::ShareProtocols>(source.EnabledProtocols);
        target.RootSquash = Enum<PublicModels::ShareRootSquash>(source.RootSquash);
        target.EnableSnapshotVirtualDirectoryAccess = source.EnableSnapshotVirtualDirectoryAccess;
        target.PaidBurstingEnabled = source.PaidBurstingEnabled;
        target.PaidBurstingMaxIops = source.PaidBurstingMaxIops;
        target.PaidBurstingMaxBandwidthMibps = source.PaidBurstingMaxBandwidthMibps;
        target.IncludedBurstIops = source.IncludedBurstIops;
        target.MaxBurstCreditsForIops = source.MaxBurstCreditsForIops;
        target.NextAllowedProvisionedIopsDowngradeTime
            = source.NextAllowedProvisionedIopsDowngradeTime;
        target.NextAllowedProvisionedBandwidthDowngradeTime
            = source.NextAllowedProvisionedBandwidthDowngradeTime;
        result.ShareItems.emplace_back(std::move(item));
      }
    }
    return Finish(std::move(result), response);
  }

  Response<PublicModels::UserDelegationKey> ServiceClient::GetUserDelegationKey(
      Pipeline& pipeline,
      Core::Url const& url,
      GetServiceUserDelegationKeyOptions const& options,
      Core::Context const& context)
  {
    Models::KeyInfo key;
    key.Start = NonEmpty(options.KeyInfo.Start);
    key.Expiry = options.KeyInfo.Expiry;
    key.DelegatedUserTid = NonEmpty(options.KeyInfo.DelegatedUserTid);
    auto response = InvokeStorage(
        [&] { return Client(pipeline, url).GetUserDelegationKeyService(key, {}, context); });
    PublicModels::UserDelegationKey result;
    result.SignedObjectId = response.Value.SignedOid;
    result.SignedTenantId = response.Value.SignedTid;
    result.SignedStartsOn = response.Value.SignedStart;
    result.SignedExpiresOn = response.Value.SignedExpiry;
    result.SignedService = response.Value.SignedService;
    result.SignedVersion = response.Value.SignedVersion;
    result.SignedDelegatedUserTid = response.Value.SignedDelegatedUserTid;
    result.Value = response.Value.Value;
    return Finish(std::move(result), response);
  }

  Response<PublicModels::CreateShareResult> ShareClient::Create(
      Pipeline& pipeline,
      Core::Url const& url,
      CreateShareOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientCreateShareOptions generated;
    ShareOptions(generated, options);
    generated.Metadata = Core::CaseInsensitiveMap(options.Metadata.begin(), options.Metadata.end());
    auto response
        = InvokeStorage([&] { return Client(pipeline, url).CreateShare(generated, context); });
    auto result = Entity<PublicModels::CreateShareResult>(response.Value);
    result.Quota = response.Value.Quota;
    result.ShareProvisionedIops = response.Value.ShareProvisionedIops;
    result.ShareProvisionedBandwidthMibps = response.Value.ShareProvisionedBandwidthMibps;
    result.ShareIncludedBurstIops = response.Value.ShareIncludedBurstIops;
    result.MaxBurstCreditsForIops = response.Value.MaxBurstCreditsForIops;
    return Finish(std::move(result), response);
  }

  Response<PublicModels::ShareProperties> ShareClient::GetProperties(
      Pipeline& pipeline,
      Core::Url const& url,
      GetSharePropertiesOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientGetPropertiesShareOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.Sharesnapshot = NonEmpty(options.Sharesnapshot);
    generated.XMsLeaseId = NonEmpty(options.LeaseId);
    auto response = InvokeStorage(
        [&] { return Client(pipeline, url).GetPropertiesShare(generated, context); });
    auto result = Entity<PublicModels::ShareProperties>(response.Value);
    auto const& source = response.Value;
    result.Metadata = Metadata(source.Metadata);
    result.Quota = Required(source.Quota, "x-ms-share-quota");
    result.ProvisionedIops = Number<int32_t>(source.ProvisionedIops);
    result.ProvisionedIngressMBps = Number<int32_t>(source.ProvisionedIngressMBps);
    result.ProvisionedEgressMBps = Number<int32_t>(source.ProvisionedEgressMBps);
    result.ProvisionedBandwidthMBps = Number<int32_t>(source.ProvisionedBandwidthMibps);
    result.NextAllowedQuotaDowngradeTime = source.NextAllowedQuotaDowngradeTime;
    result.LeaseDuration = Enum<PublicModels::LeaseDurationType>(source.LeaseDuration);
    result.LeaseState = Enum<PublicModels::LeaseState>(source.LeaseState);
    result.LeaseStatus = Enum<PublicModels::LeaseStatus>(source.LeaseStatus);
    result.AccessTier = Enum<PublicModels::AccessTier>(source.AccessTier);
    result.AccessTierChangedOn = source.AccessTierChangeTime;
    result.AccessTierTransitionState = source.AccessTierTransitionState;
    result.EnabledProtocols = Enum<PublicModels::ShareProtocols>(source.EnabledProtocols);
    result.RootSquash = Enum<PublicModels::ShareRootSquash>(source.RootSquash);
    result.EnableSnapshotVirtualDirectoryAccess = source.EnableSnapshotVirtualDirectoryAccess;
    result.PaidBurstingEnabled = source.PaidBurstingEnabled;
    result.PaidBurstingMaxIops = source.PaidBurstingMaxIops;
    result.PaidBurstingMaxBandwidthMibps = source.PaidBurstingMaxBandwidthMibps;
    result.IncludedBurstIops = source.IncludedBurstIops;
    result.MaxBurstCreditsForIops = source.MaxBurstCreditsForIops;
    result.NextAllowedProvisionedIopsDowngradeTime = source.NextAllowedProvisionedIopsDowngradeTime;
    result.NextAllowedProvisionedBandwidthDowngradeTime
        = source.NextAllowedProvisionedBandwidthDowngradeTime;
    return Finish(std::move(result), response);
  }

  Response<PublicModels::DeleteShareResult> ShareClient::Delete(
      Pipeline& pipeline,
      Core::Url const& url,
      DeleteShareOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientDeleteShareOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.Sharesnapshot = NonEmpty(options.Sharesnapshot);
    generated.XMsLeaseId = NonEmpty(options.LeaseId);
    generated.XMsDeleteSnapshots = Enum<Models::DeleteSnapshotsOptionType>(options.DeleteSnapshots);
    auto response
        = InvokeStorage([&] { return Client(pipeline, url).DeleteShare(generated, context); });
    PublicModels::DeleteShareResult result;
    result.ShareUsageBytes = response.Value.ShareUsageBytes;
    result.ShareSnapshotUsageBytes = response.Value.ShareSnapshotUsageBytes;
    return Finish(std::move(result), response);
  }

  Response<PublicModels::_detail::AcquireShareLeaseResult> ShareClient::AcquireLease(
      Pipeline& pipeline,
      Core::Url const& url,
      AcquireShareLeaseOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientAcquireLeaseShareOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.Sharesnapshot = NonEmpty(options.Sharesnapshot);
    generated.XMsLeaseDuration = options.Duration;
    generated.XMsProposedLeaseId = NonEmpty(options.ProposedLeaseId);
    auto response = InvokeStorage(
        [&] { return Client(pipeline, url).AcquireLeaseShare(generated, context); });
    auto result = Entity<PublicModels::_detail::AcquireShareLeaseResult>(response.Value);
    result.LeaseId = Required(response.Value.LeaseId, "x-ms-lease-id");
    return Finish(std::move(result), response);
  }

  Response<PublicModels::_detail::ReleaseShareLeaseResult> ShareClient::ReleaseLease(
      Pipeline& pipeline,
      Core::Url const& url,
      ReleaseShareLeaseOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientReleaseLeaseShareOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.Sharesnapshot = NonEmpty(options.Sharesnapshot);
    auto response = InvokeStorage([&] {
      return Client(pipeline, url).ReleaseLeaseShare(options.LeaseId, generated, context);
    });
    return Finish(Entity<PublicModels::_detail::ReleaseShareLeaseResult>(response.Value), response);
  }

  Response<PublicModels::_detail::ChangeShareLeaseResult> ShareClient::ChangeLease(
      Pipeline& pipeline,
      Core::Url const& url,
      ChangeShareLeaseOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientChangeLeaseShareOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.Sharesnapshot = NonEmpty(options.Sharesnapshot);
    generated.XMsProposedLeaseId = NonEmpty(options.ProposedLeaseId);
    auto response = InvokeStorage([&] {
      return Client(pipeline, url).ChangeLeaseShare(options.LeaseId, generated, context);
    });
    auto result = Entity<PublicModels::_detail::ChangeShareLeaseResult>(response.Value);
    result.LeaseId = Required(response.Value.LeaseId, "x-ms-lease-id");
    return Finish(std::move(result), response);
  }

  Response<PublicModels::_detail::RenewShareLeaseResult> ShareClient::RenewLease(
      Pipeline& pipeline,
      Core::Url const& url,
      RenewShareLeaseOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientRenewLeaseShareOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.Sharesnapshot = NonEmpty(options.Sharesnapshot);
    auto response = InvokeStorage(
        [&] { return Client(pipeline, url).RenewLeaseShare(options.LeaseId, generated, context); });
    auto result = Entity<PublicModels::_detail::RenewShareLeaseResult>(response.Value);
    result.LeaseId = Required(response.Value.LeaseId, "x-ms-lease-id");
    return Finish(std::move(result), response);
  }

  Response<PublicModels::_detail::BreakShareLeaseResult> ShareClient::BreakLease(
      Pipeline& pipeline,
      Core::Url const& url,
      BreakShareLeaseOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientBreakLeaseShareOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.Sharesnapshot = NonEmpty(options.Sharesnapshot);
    generated.XMsLeaseId = NonEmpty(options.LeaseId);
    generated.XMsLeaseBreakPeriod = options.BreakPeriod;
    auto response
        = InvokeStorage([&] { return Client(pipeline, url).BreakLeaseShare(generated, context); });
    auto result = Entity<PublicModels::_detail::BreakShareLeaseResult>(response.Value);
    result.LeaseTime = Required(response.Value.LeaseTime, "x-ms-lease-time");
    return Finish(std::move(result), response);
  }

  Response<PublicModels::CreateShareSnapshotResult> ShareClient::CreateSnapshot(
      Pipeline& pipeline,
      Core::Url const& url,
      CreateShareSnapshotOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientCreateSnapshotShareOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.Metadata = Core::CaseInsensitiveMap(options.Metadata.begin(), options.Metadata.end());
    auto response = InvokeStorage(
        [&] { return Client(pipeline, url).CreateSnapshotShare(generated, context); });
    auto result = Entity<PublicModels::CreateShareSnapshotResult>(response.Value);
    result.Snapshot = response.Value.Snapshot;
    return Finish(std::move(result), response);
  }

  Response<PublicModels::CreateSharePermissionResult> ShareClient::CreatePermission(
      Pipeline& pipeline,
      Core::Url const& url,
      CreateSharePermissionOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientCreatePermissionShareOptions generated;
    Intent(generated, options.FileRequestIntent);
    Models::SharePermission permission;
    permission.Permission = options.SharePermission.Permission;
    permission.Format = Enum<Models::FilePermissionFormat>(options.SharePermission.Format);
    auto response = InvokeStorage([&] {
      return Client(pipeline, url).CreatePermissionShare(permission, generated, context);
    });
    PublicModels::CreateSharePermissionResult result;
    result.FilePermissionKey = response.Value.FilePermissionKey;
    return Finish(std::move(result), response);
  }

  Response<PublicModels::_detail::SharePermission> ShareClient::GetPermission(
      Pipeline& pipeline,
      Core::Url const& url,
      GetSharePermissionOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientGetPermissionShareOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.XMsFilePermissionFormat
        = Enum<Models::FilePermissionFormat>(options.FilePermissionFormat);
    auto response = InvokeStorage([&] {
      return Client(pipeline, url)
          .GetPermissionShare(options.FilePermissionKey, generated, context);
    });
    PublicModels::_detail::SharePermission result;
    result.Permission = response.Value.Permission;
    result.Format = Enum<PublicModels::FilePermissionFormat>(response.Value.Format);
    return Finish(std::move(result), response);
  }

  Response<PublicModels::SetSharePropertiesResult> ShareClient::SetProperties(
      Pipeline& pipeline,
      Core::Url const& url,
      SetSharePropertiesOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientSetPropertiesShareOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.XMsShareQuota = options.Quota;
    generated.XMsAccessTier = Enum<Models::ShareAccessTier>(options.AccessTier);
    generated.XMsLeaseId = NonEmpty(options.LeaseId);
    generated.XMsRootSquash = Enum<Models::ShareRootSquash>(options.RootSquash);
    generated.XMsEnableSnapshotVirtualDirectoryAccess
        = options.EnableSnapshotVirtualDirectoryAccess;
    generated.XMsSharePaidBurstingEnabled = options.PaidBurstingEnabled;
    generated.XMsSharePaidBurstingMaxBandwidthMibps = options.PaidBurstingMaxBandwidthMibps;
    generated.XMsSharePaidBurstingMaxIops = options.PaidBurstingMaxIops;
    generated.XMsShareProvisionedIops = options.ShareProvisionedIops;
    generated.XMsShareProvisionedBandwidthMibps = options.ShareProvisionedBandwidthMibps;
    auto response = InvokeStorage(
        [&] { return Client(pipeline, url).SetPropertiesShare(generated, context); });
    auto result = Entity<PublicModels::SetSharePropertiesResult>(response.Value);
    result.Quota = response.Value.Quota;
    result.ProvisionedIops = response.Value.ProvisionedIops;
    result.ProvisionedBandwidthMibps = response.Value.ProvisionedBandwidthMibps;
    result.IncludedBurstIops = response.Value.IncludedBurstIops;
    result.MaxBurstCreditsForIops = response.Value.MaxBurstCreditsForIops;
    result.NextAllowedQuotaDowngradeTime = response.Value.NextAllowedQuotaDowngradeTime;
    result.NextAllowedProvisionedIopsDowngradeTime
        = response.Value.NextAllowedProvisionedIopsDowngradeTime;
    result.NextAllowedProvisionedBandwidthDowngradeTime
        = response.Value.NextAllowedProvisionedBandwidthDowngradeTime;
    return Finish(std::move(result), response);
  }

  Response<PublicModels::SetShareMetadataResult> ShareClient::SetMetadata(
      Pipeline& pipeline,
      Core::Url const& url,
      SetShareMetadataOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientSetMetadataShareOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.XMsLeaseId = NonEmpty(options.LeaseId);
    generated.Metadata = Core::CaseInsensitiveMap(options.Metadata.begin(), options.Metadata.end());
    auto response
        = InvokeStorage([&] { return Client(pipeline, url).SetMetadataShare(generated, context); });
    return Finish(Entity<PublicModels::SetShareMetadataResult>(response.Value), response);
  }

  Response<PublicModels::ShareAccessPolicy> ShareClient::GetAccessPolicy(
      Pipeline& pipeline,
      Core::Url const& url,
      GetShareAccessPolicyOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientGetAccessPolicyShareOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.XMsLeaseId = NonEmpty(options.LeaseId);
    auto response = InvokeStorage(
        [&] { return Client(pipeline, url).GetAccessPolicyShare(generated, context); });
    PublicModels::ShareAccessPolicy result;
    for (auto const& identifier : response.Value.Items)
    {
      PublicModels::SignedIdentifier item;
      item.Id = identifier.Id;
      if (identifier.AccessPolicy.HasValue())
      {
        item.Policy.StartsOn = identifier.AccessPolicy.Value().Start;
        item.Policy.ExpiresOn = identifier.AccessPolicy.Value().Expiry;
        if (identifier.AccessPolicy.Value().Permission.HasValue())
        {
          item.Policy.Permission = identifier.AccessPolicy.Value().Permission.Value();
        }
      }
      result.SignedIdentifiers.emplace_back(std::move(item));
    }
    return Finish(std::move(result), response);
  }

  Response<PublicModels::SetShareAccessPolicyResult> ShareClient::SetAccessPolicy(
      Pipeline& pipeline,
      Core::Url const& url,
      SetShareAccessPolicyOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientSetAccessPolicyShareOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.XMsLeaseId = NonEmpty(options.LeaseId);
    Models::SignedIdentifiers identifiers;
    for (auto const& identifier : options.ShareAcl)
    {
      Models::SignedIdentifier item;
      item.Id = identifier.Id;
      Models::AccessPolicy policy;
      policy.Start = identifier.Policy.StartsOn;
      policy.Expiry = identifier.Policy.ExpiresOn;
      policy.Permission = NonEmpty(Nullable<std::string>(identifier.Policy.Permission));
      item.AccessPolicy = std::move(policy);
      identifiers.Items.emplace_back(std::move(item));
    }
    generated.ShareAcl = std::move(identifiers);
    auto response = InvokeStorage(
        [&] { return Client(pipeline, url).SetAccessPolicyShare(generated, context); });
    return Finish(Entity<PublicModels::SetShareAccessPolicyResult>(response.Value), response);
  }

  Response<PublicModels::ShareStatistics> ShareClient::GetStatistics(
      Pipeline& pipeline,
      Core::Url const& url,
      GetShareStatisticsOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientGetStatisticsShareOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.XMsLeaseId = NonEmpty(options.LeaseId);
    auto response = InvokeStorage(
        [&] { return Client(pipeline, url).GetStatisticsShare(generated, context); });
    PublicModels::ShareStatistics result;
    if (response.Value.Etag.HasValue())
    {
      result.ETag = response.Value.Etag.Value();
    }
    if (response.Value.LastModified.HasValue())
    {
      result.LastModified = response.Value.LastModified.Value();
    }
    result.ShareUsageInBytes = response.Value.ShareUsageBytes;
    return Finish(std::move(result), response);
  }

  Response<PublicModels::_detail::CreateDirectoryResult> DirectoryClient::Create(
      Pipeline& pipeline,
      Core::Url const& url,
      CreateDirectoryOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientCreateDirectoryOptions generated;
    Permissions(generated, options);
    PosixOptions(generated, options);
    Intent(generated, options.FileRequestIntent);
    generated.XMsAllowTrailingDot = options.AllowTrailingDot;
    generated.Metadata = Core::CaseInsensitiveMap(options.Metadata.begin(), options.Metadata.end());
    generated.XMsFileAttributes = NonEmpty(Nullable<std::string>(options.FileAttributes));
    generated.XMsFilePropertySemantics
        = Enum<Models::FilePropertySemantics>(options.FilePropertySemantics);
    auto response
        = InvokeStorage([&] { return Client(pipeline, url).CreateDirectory(generated, context); });
    auto result = SmbEntity<PublicModels::_detail::CreateDirectoryResult>(response.Value);
    Posix(result, response.Value);
    result.NfsFileType = Enum<PublicModels::NfsFileType>(response.Value.NfsFileType);
    result.IsServerEncrypted
        = Required(response.Value.RequestServerEncrypted, "x-ms-request-server-encrypted");
    return Finish(std::move(result), response);
  }

  Response<PublicModels::_detail::DirectoryProperties> DirectoryClient::GetProperties(
      Pipeline& pipeline,
      Core::Url const& url,
      GetDirectoryPropertiesOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientGetPropertiesDirectoryOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.XMsAllowTrailingDot = options.AllowTrailingDot;
    generated.Sharesnapshot = NonEmpty(options.Sharesnapshot);
    auto response = InvokeStorage(
        [&] { return Client(pipeline, url).GetPropertiesDirectory(generated, context); });
    auto result = SmbEntity<PublicModels::_detail::DirectoryProperties>(response.Value);
    Posix(result, response.Value);
    result.NfsFileType = Enum<PublicModels::NfsFileType>(response.Value.NfsFileType);
    result.Metadata = Metadata(response.Value.Metadata);
    result.IsServerEncrypted = Required(response.Value.ServerEncrypted, "x-ms-server-encrypted");
    return Finish(std::move(result), response);
  }

  Response<PublicModels::DeleteDirectoryResult> DirectoryClient::Delete(
      Pipeline& pipeline,
      Core::Url const& url,
      DeleteDirectoryOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientDeleteDirectoryOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.XMsAllowTrailingDot = options.AllowTrailingDot;
    auto response
        = InvokeStorage([&] { return Client(pipeline, url).DeleteDirectory(generated, context); });
    return Finish(PublicModels::DeleteDirectoryResult{}, response);
  }

  Response<PublicModels::_detail::SetDirectoryPropertiesResult> DirectoryClient::SetProperties(
      Pipeline& pipeline,
      Core::Url const& url,
      SetDirectoryPropertiesOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientSetPropertiesDirectoryOptions generated;
    Permissions(generated, options);
    PosixOptions(generated, options);
    Intent(generated, options.FileRequestIntent);
    generated.XMsAllowTrailingDot = options.AllowTrailingDot;
    generated.XMsFileAttributes = NonEmpty(Nullable<std::string>(options.FileAttributes));
    auto response = InvokeStorage(
        [&] { return Client(pipeline, url).SetPropertiesDirectory(generated, context); });
    auto result = SmbEntity<PublicModels::_detail::SetDirectoryPropertiesResult>(response.Value);
    Posix(result, response.Value);
    result.IsServerEncrypted
        = Required(response.Value.RequestServerEncrypted, "x-ms-request-server-encrypted");
    return Finish(std::move(result), response);
  }

  Response<PublicModels::SetDirectoryMetadataResult> DirectoryClient::SetMetadata(
      Pipeline& pipeline,
      Core::Url const& url,
      SetDirectoryMetadataOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientSetMetadataDirectoryOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.XMsAllowTrailingDot = options.AllowTrailingDot;
    generated.Metadata = Core::CaseInsensitiveMap(options.Metadata.begin(), options.Metadata.end());
    auto response = InvokeStorage(
        [&] { return Client(pipeline, url).SetMetadataDirectory(generated, context); });
    PublicModels::SetDirectoryMetadataResult result;
    result.ETag = Required(response.Value.Etag, "ETag");
    result.IsServerEncrypted
        = Required(response.Value.RequestServerEncrypted, "x-ms-request-server-encrypted");
    return Finish(std::move(result), response);
  }

  Response<PublicModels::_detail::ListFilesAndDirectoriesSegmentResponse>
  DirectoryClient::ListFilesAndDirectoriesSegment(
      Pipeline& pipeline,
      Core::Url const& url,
      ListDirectoryFilesAndDirectoriesSegmentOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientListFilesAndDirectoriesSegmentDirectoryOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.Prefix = NonEmpty(options.Prefix);
    generated.Sharesnapshot = NonEmpty(options.Sharesnapshot);
    generated.Marker = NonEmpty(options.Marker);
    generated.Maxresults = options.MaxResults;
    generated.XMsFileExtendedInfo = options.IncludeExtendedInfo;
    generated.XMsAllowTrailingDot = options.AllowTrailingDot;
    std::vector<Models::ListFilesIncludeType> include;
    if (options.Include.HasValue())
    {
      for (auto const& flag :
           {std::make_pair(PublicModels::ListFilesIncludeFlags::Timestamps, "Timestamps"),
            std::make_pair(PublicModels::ListFilesIncludeFlags::ETag, "Etag"),
            std::make_pair(PublicModels::ListFilesIncludeFlags::Attributes, "Attributes"),
            std::make_pair(PublicModels::ListFilesIncludeFlags::PermissionKey, "PermissionKey")})
      {
        if ((options.Include.Value() & flag.first) == flag.first)
        {
          include.emplace_back(flag.second);
        }
      }
    }
    if (!include.empty())
    {
      generated.Include = std::move(include);
    }
    auto response = InvokeStorage([&] {
      return Client(pipeline, url).ListFilesAndDirectoriesSegmentDirectory(generated, context);
    });
    PublicModels::_detail::ListFilesAndDirectoriesSegmentResponse result;
    auto const& source = response.Value;
    result.ServiceEndpoint = source.ServiceEndpoint;
    result.ShareName = source.ShareName;
    result.ShareSnapshot = source.ShareSnapshot;
    result.Encoded = source.Encoded;
    result.DirectoryPath = source.DirectoryPath;
    result.Prefix = Encoded(source.Prefix);
    result.Marker = source.Marker;
    result.MaxResults = Number<int32_t>(source.MaxResults);
    result.NextMarker = source.NextMarker;
    result.DirectoryId = source.DirectoryId;
    for (auto const& directory : source.Segment.DirectoryItems)
    {
      PublicModels::_detail::DirectoryItem item;
      item.Name = Encoded(directory.Name);
      if (directory.Properties.HasValue())
      {
        ItemProperties(item.Details, directory.Properties.Value());
      }
      ItemSmb(item.Details, directory);
      result.Segment.DirectoryItems.emplace_back(std::move(item));
    }
    for (auto const& file : source.Segment.FileItems)
    {
      PublicModels::_detail::FileItem item;
      item.Name = Encoded(file.Name);
      ItemProperties(item.Details, file.Properties);
      ItemSmb(item.Details, file);
      item.Details.FileSize = file.Properties.ContentLength;
      result.Segment.FileItems.emplace_back(std::move(item));
    }
    return Finish(std::move(result), response);
  }

  Response<PublicModels::_detail::ListHandlesResponse> DirectoryClient::ListHandles(
      Pipeline& pipeline,
      Core::Url const& url,
      ListDirectoryHandlesOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientListHandlesDirectoryOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.Marker = NonEmpty(options.Marker);
    generated.Maxresults = options.MaxResults;
    generated.Sharesnapshot = NonEmpty(options.Sharesnapshot);
    generated.XMsRecursive = options.Recursive;
    generated.XMsAllowTrailingDot = options.AllowTrailingDot;
    auto response = InvokeStorage(
        [&] { return Client(pipeline, url).ListHandlesDirectory(generated, context); });
    return Finish(Handles(response.Value), response);
  }

  Response<PublicModels::_detail::ForceCloseDirectoryHandlesResult>
  DirectoryClient::ForceCloseHandles(
      Pipeline& pipeline,
      Core::Url const& url,
      ForceDirectoryCloseHandlesOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientForceCloseHandlesDirectoryOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.Marker = NonEmpty(options.Marker);
    generated.Sharesnapshot = NonEmpty(options.Sharesnapshot);
    generated.XMsRecursive = options.Recursive;
    generated.XMsAllowTrailingDot = options.AllowTrailingDot;
    auto response = InvokeStorage([&] {
      return Client(pipeline, url).ForceCloseHandlesDirectory(options.HandleId, generated, context);
    });
    PublicModels::_detail::ForceCloseDirectoryHandlesResult result;
    result.ContinuationToken = response.Value.Marker;
    result.NumberOfHandlesClosed = response.Value.NumberOfHandlesClosed;
    result.NumberOfHandlesFailedToClose = response.Value.NumberOfHandlesFailedToClose;
    return Finish(std::move(result), response);
  }

  Response<PublicModels::_detail::RenameDirectoryResult> DirectoryClient::Rename(
      Pipeline& pipeline,
      Core::Url const& url,
      RenameDirectoryOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientRenameDirectoryOptions generated;
    RenameOptions(generated, options);
    auto response = InvokeStorage([&] {
      return Client(pipeline, url).RenameDirectory(options.RenameSource, generated, context);
    });
    return Finish(
        RenameResult<PublicModels::_detail::RenameDirectoryResult>(response.Value), response);
  }

  Response<PublicModels::_detail::CreateFileResult> FileClient::Create(
      Pipeline& pipeline,
      Core::Url const& url,
      Core::IO::BodyStream& requestBody,
      CreateFileOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientCreateFileOptions generated;
    Permissions(generated, options);
    PosixOptions(generated, options);
    FileHeaders(generated, options);
    Intent(generated, options.FileRequestIntent);
    generated.XMsAllowTrailingDot = options.AllowTrailingDot;
    generated.Metadata = Core::CaseInsensitiveMap(options.Metadata.begin(), options.Metadata.end());
    generated.XMsFileAttributes = NonEmpty(Nullable<std::string>(options.FileAttributes));
    generated.XMsLeaseId = NonEmpty(options.LeaseId);
    generated.XMsFileFileType = Enum<Models::NfsFileType>(options.NfsFileType);
    generated.ContentMd5 = NonEmpty(options.ContentMD5);
    generated.XMsFilePropertySemantics
        = Enum<Models::FilePropertySemantics>(options.FilePropertySemantics);
    generated.XMsStructuredBody = NonEmpty(options.StructuredBodyType);
    generated.XMsStructuredContentLength = options.StructuredContentLength;
    generated.ContentLength = requestBody.Length();
    auto response = InvokeStorage([&] {
      return Client(pipeline, url)
          .CreateFile(options.FileContentLength, requestBody, generated, context);
    });
    auto result = SmbEntity<PublicModels::_detail::CreateFileResult>(response.Value);
    Posix(result, response.Value);
    result.NfsFileType = Enum<PublicModels::NfsFileType>(response.Value.NfsFileType);
    result.IsServerEncrypted
        = Required(response.Value.RequestServerEncrypted, "x-ms-request-server-encrypted");
    result.StructuredBodyType = response.Value.StructuredBodyType;
    return Finish(std::move(result), response);
  }

  Response<PublicModels::_detail::DownloadFileResult> FileClient::Download(
      Pipeline& pipeline,
      Core::Url const& url,
      DownloadFileOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientDownloadFileOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.XMsAllowTrailingDot = options.AllowTrailingDot;
    generated.XMsRange = NonEmpty(options.Range);
    generated.XMsRangeGetContentMd5 = options.RangeGetContentMD5;
    generated.XMsStructuredBody = NonEmpty(options.StructuredBodyType);
    generated.XMsLeaseId = NonEmpty(options.LeaseId);
    auto response
        = InvokeStorage([&] { return Client(pipeline, url).DownloadFile(generated, context); });
    PublicModels::_detail::DownloadFileResult result;
    auto const& source = response.Value;
    result.BodyStream = std::move(response.Value.BodyStream);
    result.Details = SmbEntity<PublicModels::_detail::DownloadFileDetails>(source);
    Posix(result.Details, source);
    CopyProperties(result.Details, source);
    result.Details.Metadata = Metadata(source.Metadata);
    result.Details.IsServerEncrypted = Required(source.ServerEncrypted, "x-ms-server-encrypted");
    result.Details.LinkCount = source.LinkCount;
    result.HttpHeaders = HttpHeaders(source);
    auto const contentType = response.RawResponse->GetHeaders().find("Content-Type");
    if (contentType != response.RawResponse->GetHeaders().end())
    {
      result.HttpHeaders.ContentType = contentType->second;
    }
    if (source.ContentMd5.HasValue())
    {
      ContentHash hash;
      hash.Algorithm = HashAlgorithm::Md5;
      hash.Value = Core::Convert::Base64Decode(source.ContentMd5.Value());
      result.TransactionalContentHash = hash;
      if (response.RawResponse->GetStatusCode() == Core::Http::HttpStatusCode::Ok)
      {
        result.HttpHeaders.ContentHash = std::move(hash);
      }
    }
    if (source.FileContentMd5.HasValue())
    {
      result.HttpHeaders.ContentHash.Algorithm = HashAlgorithm::Md5;
      result.HttpHeaders.ContentHash.Value
          = Core::Convert::Base64Decode(source.FileContentMd5.Value());
    }
    return Finish(std::move(result), response);
  }

  Response<PublicModels::_detail::FileProperties> FileClient::GetProperties(
      Pipeline& pipeline,
      Core::Url const& url,
      GetFilePropertiesOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientGetPropertiesFileOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.XMsAllowTrailingDot = options.AllowTrailingDot;
    generated.Sharesnapshot = NonEmpty(options.Sharesnapshot);
    generated.XMsLeaseId = NonEmpty(options.LeaseId);
    auto response = InvokeStorage(
        [&] { return Client(pipeline, url).GetPropertiesFile(generated, context); });
    auto const& source = response.Value;
    auto result = SmbEntity<PublicModels::_detail::FileProperties>(source);
    Posix(result, source);
    CopyProperties(result, source);
    result.HttpHeaders = HttpHeaders(source);
    if (source.ContentType.HasValue())
    {
      result.HttpHeaders.ContentType = source.ContentType.Value();
    }
    if (source.ContentMd5.HasValue())
    {
      result.HttpHeaders.ContentHash.Algorithm = HashAlgorithm::Md5;
      result.HttpHeaders.ContentHash.Value = Core::Convert::Base64Decode(source.ContentMd5.Value());
    }
    result.FileSize = Required(source.ContentLength, "Content-Length");
    result.Metadata = Metadata(source.Metadata);
    result.IsServerEncrypted = Required(source.ServerEncrypted, "x-ms-server-encrypted");
    result.LinkCount = source.LinkCount;
    result.NfsFileType = Enum<PublicModels::NfsFileType>(source.NfsFileType);
    return Finish(std::move(result), response);
  }

  Response<PublicModels::DeleteFileResult> FileClient::Delete(
      Pipeline& pipeline,
      Core::Url const& url,
      DeleteFileOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientDeleteFileOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.XMsAllowTrailingDot = options.AllowTrailingDot;
    generated.XMsLeaseId = NonEmpty(options.LeaseId);
    auto response
        = InvokeStorage([&] { return Client(pipeline, url).DeleteFile(generated, context); });
    PublicModels::DeleteFileResult result;
    result.LinkCount = response.Value.LinkCount;
    return Finish(std::move(result), response);
  }

  Response<PublicModels::_detail::SetFilePropertiesResult> FileClient::SetHttpHeaders(
      Pipeline& pipeline,
      Core::Url const& url,
      SetFileHttpHeadersOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientSetHttpHeadersFileOptions generated;
    Permissions(generated, options);
    PosixOptions(generated, options);
    FileHeaders(generated, options);
    Intent(generated, options.FileRequestIntent);
    generated.XMsAllowTrailingDot = options.AllowTrailingDot;
    generated.XMsLeaseId = NonEmpty(options.LeaseId);
    generated.XMsContentLength = options.FileContentLength;
    generated.XMsFileAttributes = NonEmpty(Nullable<std::string>(options.FileAttributes));
    auto response = InvokeStorage(
        [&] { return Client(pipeline, url).SetHttpHeadersFile(generated, context); });
    auto result = SmbEntity<PublicModels::_detail::SetFilePropertiesResult>(response.Value);
    Posix(result, response.Value);
    result.IsServerEncrypted
        = Required(response.Value.RequestServerEncrypted, "x-ms-request-server-encrypted");
    result.LinkCount = response.Value.LinkCount;
    return Finish(std::move(result), response);
  }

  Response<PublicModels::SetFileMetadataResult> FileClient::SetMetadata(
      Pipeline& pipeline,
      Core::Url const& url,
      SetFileMetadataOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientSetMetadataFileOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.XMsAllowTrailingDot = options.AllowTrailingDot;
    generated.XMsLeaseId = NonEmpty(options.LeaseId);
    generated.Metadata = Core::CaseInsensitiveMap(options.Metadata.begin(), options.Metadata.end());
    auto response
        = InvokeStorage([&] { return Client(pipeline, url).SetMetadataFile(generated, context); });
    PublicModels::SetFileMetadataResult result;
    result.ETag = Required(response.Value.Etag, "ETag");
    result.IsServerEncrypted
        = Required(response.Value.RequestServerEncrypted, "x-ms-request-server-encrypted");
    return Finish(std::move(result), response);
  }

  Response<PublicModels::_detail::AcquireFileLeaseResult> FileClient::AcquireLease(
      Pipeline& pipeline,
      Core::Url const& url,
      AcquireFileLeaseOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientAcquireLeaseFileOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.XMsAllowTrailingDot = options.AllowTrailingDot;
    generated.XMsLeaseDuration = options.Duration;
    generated.XMsProposedLeaseId = NonEmpty(options.ProposedLeaseId);
    auto response
        = InvokeStorage([&] { return Client(pipeline, url).AcquireLeaseFile(generated, context); });
    auto result = Entity<PublicModels::_detail::AcquireFileLeaseResult>(response.Value);
    result.LeaseId = Required(response.Value.LeaseId, "x-ms-lease-id");
    return Finish(std::move(result), response);
  }

  Response<PublicModels::_detail::ReleaseFileLeaseResult> FileClient::ReleaseLease(
      Pipeline& pipeline,
      Core::Url const& url,
      ReleaseFileLeaseOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientReleaseLeaseFileOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.XMsAllowTrailingDot = options.AllowTrailingDot;
    auto response = InvokeStorage([&] {
      return Client(pipeline, url).ReleaseLeaseFile(options.LeaseId, generated, context);
    });
    return Finish(Entity<PublicModels::_detail::ReleaseFileLeaseResult>(response.Value), response);
  }

  Response<PublicModels::_detail::ChangeFileLeaseResult> FileClient::ChangeLease(
      Pipeline& pipeline,
      Core::Url const& url,
      ChangeFileLeaseOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientChangeLeaseFileOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.XMsAllowTrailingDot = options.AllowTrailingDot;
    generated.XMsProposedLeaseId = NonEmpty(options.ProposedLeaseId);
    auto response = InvokeStorage(
        [&] { return Client(pipeline, url).ChangeLeaseFile(options.LeaseId, generated, context); });
    auto result = Entity<PublicModels::_detail::ChangeFileLeaseResult>(response.Value);
    result.LeaseId = Required(response.Value.LeaseId, "x-ms-lease-id");
    return Finish(std::move(result), response);
  }

  Response<PublicModels::_detail::BreakFileLeaseResult> FileClient::BreakLease(
      Pipeline& pipeline,
      Core::Url const& url,
      BreakFileLeaseOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientBreakLeaseFileOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.XMsAllowTrailingDot = options.AllowTrailingDot;
    generated.XMsLeaseId = NonEmpty(options.LeaseId);
    auto response
        = InvokeStorage([&] { return Client(pipeline, url).BreakLeaseFile(generated, context); });
    return Finish(Entity<PublicModels::_detail::BreakFileLeaseResult>(response.Value), response);
  }

  Response<PublicModels::UploadFileRangeResult> FileClient::UploadRange(
      Pipeline& pipeline,
      Core::Url const& url,
      Core::IO::BodyStream& requestBody,
      UploadFileRangeOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientUploadRangeFileOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.XMsAllowTrailingDot = options.AllowTrailingDot;
    generated.ContentMd5 = NonEmpty(options.ContentMD5);
    generated.XMsLeaseId = NonEmpty(options.LeaseId);
    generated.XMsFileLastWriteTime = Enum<Models::FileLastWrittenMode>(options.FileLastWrittenMode);
    generated.XMsStructuredBody = NonEmpty(options.StructuredBodyType);
    generated.XMsStructuredContentLength = options.StructuredContentLength;
    auto response = InvokeStorage([&] {
      return Client(pipeline, url)
          .UploadRangeFile(
              options.Range,
              Models::FileRangeWriteType(options.FileRangeWrite),
              requestBody.Length(),
              requestBody,
              generated,
              context);
    });
    auto result = Entity<PublicModels::UploadFileRangeResult>(response.Value);
    if (response.Value.ContentMd5.HasValue())
    {
      result.TransactionalContentHash.Algorithm = HashAlgorithm::Md5;
      result.TransactionalContentHash.Value
          = Core::Convert::Base64Decode(response.Value.ContentMd5.Value());
    }
    if (response.Value.RequestServerEncrypted.HasValue())
    {
      result.IsServerEncrypted = response.Value.RequestServerEncrypted.Value();
    }
    return Finish(std::move(result), response);
  }

  Response<PublicModels::UploadFileRangeFromUriResult> FileClient::UploadRangeFromUri(
      Pipeline& pipeline,
      Core::Url const& url,
      UploadFileRangeFromUriOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientUploadRangeFromUrlFileOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.XMsAllowTrailingDot = options.AllowTrailingDot;
    generated.XMsSourceAllowTrailingDot = options.AllowSourceTrailingDot;
    generated.XMsSourceRange = NonEmpty(options.SourceRange);
    generated.XMsSourceContentCrc64 = NonEmpty(options.SourceContentCrc64);
    generated.XMsSourceIfMatchCrc64 = NonEmpty(options.SourceIfMatchCrc64);
    generated.XMsSourceIfNoneMatchCrc64 = NonEmpty(options.SourceIfNoneMatchCrc64);
    generated.XMsLeaseId = NonEmpty(options.LeaseId);
    generated.XMsCopySourceAuthorization = NonEmpty(options.CopySourceAuthorization);
    generated.XMsFileLastWriteTime = Enum<Models::FileLastWrittenMode>(options.FileLastWrittenMode);
    auto response = InvokeStorage([&] {
      return Client(pipeline, url)
          .UploadRangeFromUrlFile(
              options.Range,
              options.CopySource,
              Models::FileRangeWriteFromUrlType("update"),
              0,
              generated,
              context);
    });
    auto result = Entity<PublicModels::UploadFileRangeFromUriResult>(response.Value);
    if (response.Value.ContentMd5.HasValue())
    {
      result.TransactionalContentHash.Algorithm = HashAlgorithm::Md5;
      result.TransactionalContentHash.Value
          = Core::Convert::Base64Decode(response.Value.ContentMd5.Value());
    }
    if (response.Value.ContentCrc64.HasValue())
    {
      result.TransactionalContentHash.Algorithm = HashAlgorithm::Crc64;
      result.TransactionalContentHash.Value
          = Core::Convert::Base64Decode(response.Value.ContentCrc64.Value());
    }
    if (response.Value.RequestServerEncrypted.HasValue())
    {
      result.IsServerEncrypted = response.Value.RequestServerEncrypted.Value();
    }
    return Finish(std::move(result), response);
  }

  Response<PublicModels::_detail::GetFileRangeListResult> FileClient::GetRangeList(
      Pipeline& pipeline,
      Core::Url const& url,
      GetFileRangeListOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientListAllRangesFileOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.Sharesnapshot = NonEmpty(options.Sharesnapshot);
    generated.Prevsharesnapshot = NonEmpty(options.Prevsharesnapshot);
    generated.XMsRange = NonEmpty(options.Range);
    generated.XMsLeaseId = NonEmpty(options.LeaseId);
    generated.XMsAllowTrailingDot = options.AllowTrailingDot;
    generated.XMsFileSupportRename = options.SupportRename;
    generated.Marker = NonEmpty(options.Marker);
    generated.Maxresults = options.MaxResults;
    auto response = InvokeStorage(
        [&] { return Client(pipeline, url).ListAllRangesFile(generated, context); });
    auto result = Entity<PublicModels::_detail::GetFileRangeListResult>(response.Value);
    result.FileSize = response.Value.FileContentLength;
    if (response.Value.NextMarker.HasValue())
    {
      result.NextMarker = response.Value.NextMarker.Value();
    }
    if (response.Value.Ranges.HasValue())
    {
      for (auto const& range : response.Value.Ranges.Value())
      {
        Core::Http::HttpRange item;
        item.Offset = range.Start;
        item.Length = range.End - range.Start + 1;
        result.Ranges.emplace_back(std::move(item));
      }
    }
    if (response.Value.ClearRanges.HasValue())
    {
      for (auto const& range : response.Value.ClearRanges.Value())
      {
        Core::Http::HttpRange item;
        item.Offset = range.Start;
        item.Length = range.End - range.Start + 1;
        result.ClearRanges.emplace_back(std::move(item));
      }
    }
    return Finish(std::move(result), response);
  }

  Response<PublicModels::_detail::StartFileCopyResult> FileClient::StartCopy(
      Pipeline& pipeline,
      Core::Url const& url,
      StartFileCopyOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientStartCopyFileOptions generated;
    Permissions(generated, options);
    PosixOptions(generated, options);
    Intent(generated, options.FileRequestIntent);
    generated.Metadata = Core::CaseInsensitiveMap(options.Metadata.begin(), options.Metadata.end());
    generated.XMsFilePermissionCopyMode
        = Enum<Models::PermissionCopyModeType>(options.FilePermissionCopyMode);
    generated.XMsFileCopyIgnoreReadonly = options.IgnoreReadOnly;
    generated.XMsFileAttributes = NonEmpty(options.FileAttributes);
    generated.XMsFileCopySetArchive = options.SetArchiveAttribute;
    generated.XMsLeaseId = NonEmpty(options.LeaseId);
    generated.XMsAllowTrailingDot = options.AllowTrailingDot;
    generated.XMsSourceAllowTrailingDot = options.AllowSourceTrailingDot;
    generated.XMsFileModeCopyMode = Enum<Models::ModeCopyMode>(options.FileModeCopyMode);
    generated.XMsFileOwnerCopyMode = Enum<Models::OwnerCopyMode>(options.FileOwnerCopyMode);
    auto response = InvokeStorage([&] {
      return Client(pipeline, url).StartCopyFile(options.CopySource, generated, context);
    });
    auto result = Entity<PublicModels::_detail::StartFileCopyResult>(response.Value);
    result.CopyId = response.Value.CopyId;
    result.CopyStatus = PublicModels::CopyStatus(response.Value.CopyStatus.ToString());
    return Finish(std::move(result), response);
  }

  Response<PublicModels::AbortFileCopyResult> FileClient::AbortCopy(
      Pipeline& pipeline,
      Core::Url const& url,
      AbortFileCopyOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientAbortCopyFileOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.XMsLeaseId = NonEmpty(options.LeaseId);
    generated.XMsAllowTrailingDot = options.AllowTrailingDot;
    auto response = InvokeStorage(
        [&] { return Client(pipeline, url).AbortCopyFile(options.CopyId, generated, context); });
    return Finish(PublicModels::AbortFileCopyResult{}, response);
  }

  Response<PublicModels::_detail::ListHandlesResponse> FileClient::ListHandles(
      Pipeline& pipeline,
      Core::Url const& url,
      ListFileHandlesOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientListHandlesFileOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.Marker = NonEmpty(options.Marker);
    generated.Maxresults = options.MaxResults;
    generated.Sharesnapshot = NonEmpty(options.Sharesnapshot);
    generated.XMsAllowTrailingDot = options.AllowTrailingDot;
    auto response
        = InvokeStorage([&] { return Client(pipeline, url).ListHandlesFile(generated, context); });
    return Finish(Handles(response.Value), response);
  }

  Response<PublicModels::_detail::ForceFileCloseHandlesResult> FileClient::ForceCloseHandles(
      Pipeline& pipeline,
      Core::Url const& url,
      ForceFileCloseHandlesOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientForceCloseHandlesFileOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.Marker = NonEmpty(options.Marker);
    generated.Sharesnapshot = NonEmpty(options.Sharesnapshot);
    generated.XMsAllowTrailingDot = options.AllowTrailingDot;
    auto response = InvokeStorage([&] {
      return Client(pipeline, url).ForceCloseHandlesFile(options.HandleId, generated, context);
    });
    PublicModels::_detail::ForceFileCloseHandlesResult result;
    result.ContinuationToken = response.Value.Marker;
    result.NumberOfHandlesClosed = response.Value.NumberOfHandlesClosed;
    result.NumberOfHandlesFailedToClose = response.Value.NumberOfHandlesFailedToClose;
    return Finish(std::move(result), response);
  }

  Response<PublicModels::_detail::RenameFileResult> FileClient::Rename(
      Pipeline& pipeline,
      Core::Url const& url,
      RenameFileOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientRenameFileOptions generated;
    RenameOptions(generated, options);
    generated.XMsContentType = NonEmpty(options.FileContentType);
    auto response = InvokeStorage(
        [&] { return Client(pipeline, url).RenameFile(options.RenameSource, generated, context); });
    return Finish(RenameResult<PublicModels::_detail::RenameFileResult>(response.Value), response);
  }

  Response<PublicModels::_detail::CreateFileSymbolicLinkResult> FileClient::CreateSymbolicLink(
      Pipeline& pipeline,
      Core::Url const& url,
      CreateFileSymbolicLinkOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientCreateSymbolicLinkFileOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.Metadata = Core::CaseInsensitiveMap(options.Metadata.begin(), options.Metadata.end());
    generated.XMsFileCreationTime = NonEmpty(options.FileCreationTime);
    generated.XMsFileLastWriteTime = NonEmpty(options.FileLastWriteTime);
    generated.XMsLeaseId = NonEmpty(options.LeaseId);
    generated.XMsOwner = NonEmpty(options.Owner);
    generated.XMsGroup = NonEmpty(options.Group);
    auto response = InvokeStorage([&] {
      return Client(pipeline, url).CreateSymbolicLinkFile(options.LinkText, generated, context);
    });
    auto result = Entity<PublicModels::_detail::CreateFileSymbolicLinkResult>(response.Value);
    result.SmbProperties = FileIdentity(response.Value);
    result.FileMode = Required(response.Value.FileMode, "x-ms-mode");
    result.Owner = Required(response.Value.Owner, "x-ms-owner");
    result.Group = Required(response.Value.Group, "x-ms-group");
    result.NfsFileType = PublicModels::NfsFileType(
        Required(response.Value.NfsFileType, "x-ms-file-file-type").ToString());
    return Finish(std::move(result), response);
  }

  Response<PublicModels::_detail::GetFileSymbolicLinkResult> FileClient::GetSymbolicLink(
      Pipeline& pipeline,
      Core::Url const& url,
      GetFileSymbolicLinkOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientGetSymbolicLinkFileOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.Sharesnapshot = NonEmpty(options.Sharesnapshot);
    auto response = InvokeStorage(
        [&] { return Client(pipeline, url).GetSymbolicLinkFile(generated, context); });
    auto result = Entity<PublicModels::_detail::GetFileSymbolicLinkResult>(response.Value);
    result.LinkText = response.Value.LinkText;
    return Finish(std::move(result), response);
  }

  Response<PublicModels::_detail::CreateFileHardLinkResult> FileClient::CreateHardLink(
      Pipeline& pipeline,
      Core::Url const& url,
      CreateFileHardLinkOptions const& options,
      Core::Context const& context)
  {
    FileSharesClientCreateHardLinkFileOptions generated;
    Intent(generated, options.FileRequestIntent);
    generated.XMsLeaseId = NonEmpty(options.LeaseId);
    auto response = InvokeStorage([&] {
      return Client(pipeline, url).CreateHardLinkFile(options.TargetFile, generated, context);
    });
    auto result = Entity<PublicModels::_detail::CreateFileHardLinkResult>(response.Value);
    result.SmbProperties = FileIdentity(response.Value);
    result.LinkCount = Required(response.Value.LinkCount, "x-ms-link-count");
    result.FileMode = Required(response.Value.FileMode, "x-ms-mode");
    result.Owner = Required(response.Value.Owner, "x-ms-owner");
    result.Group = Required(response.Value.Group, "x-ms-group");
    result.NfsFileType = PublicModels::NfsFileType(
        Required(response.Value.NfsFileType, "x-ms-file-file-type").ToString());
    return Finish(std::move(result), response);
  }

}}}}} // namespace Azure::Storage::Files::Shares::_detail

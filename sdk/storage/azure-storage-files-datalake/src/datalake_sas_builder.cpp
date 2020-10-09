// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/datalake/datalake_sas_builder.hpp"
#include "azure/core/http/http.hpp"
#include "azure/storage/common/crypt.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake {
  namespace {
    std::string DataLakeSasResourceToString(DataLakeSasResource resource)
    {
      if (resource == DataLakeSasResource::FileSystem)
      {
        return "c";
      }
      else if (resource == DataLakeSasResource::File)
      {
        return "b";
      }
      else if (resource == DataLakeSasResource::Directory)
      {
        return "d";
      }
      else
      {
        throw std::runtime_error("unknown DataLakeSasResource value");
      }
    }
  } // namespace

  std::string DataLakeFileSystemSasPermissionsToString(DataLakeFileSystemSasPermissions permissions)
  {
    std::string permissions_str;
    // The order matters
    if ((permissions & DataLakeFileSystemSasPermissions::Read)
        == DataLakeFileSystemSasPermissions::Read)
    {
      permissions_str += "r";
    }
    if ((permissions & DataLakeFileSystemSasPermissions::Add)
        == DataLakeFileSystemSasPermissions::Add)
    {
      permissions_str += "a";
    }
    if ((permissions & DataLakeFileSystemSasPermissions::Create)
        == DataLakeFileSystemSasPermissions::Create)
    {
      permissions_str += "c";
    }
    if ((permissions & DataLakeFileSystemSasPermissions::Write)
        == DataLakeFileSystemSasPermissions::Write)
    {
      permissions_str += "w";
    }
    if ((permissions & DataLakeFileSystemSasPermissions::Delete)
        == DataLakeFileSystemSasPermissions::Delete)
    {
      permissions_str += "d";
    }
    if ((permissions & DataLakeFileSystemSasPermissions::List)
        == DataLakeFileSystemSasPermissions::List)
    {
      permissions_str += "l";
    }
    return permissions_str;
  }

  void DataLakeSasBuilder::SetPermissions(DataLakeSasPermissions permissions)
  {
    Permissions.clear();
    // The order matters
    if ((permissions & DataLakeSasPermissions::Read) == DataLakeSasPermissions::Read)
    {
      Permissions += "r";
    }
    if ((permissions & DataLakeSasPermissions::Add) == DataLakeSasPermissions::Add)
    {
      Permissions += "a";
    }
    if ((permissions & DataLakeSasPermissions::Create) == DataLakeSasPermissions::Create)
    {
      Permissions += "c";
    }
    if ((permissions & DataLakeSasPermissions::Write) == DataLakeSasPermissions::Write)
    {
      Permissions += "w";
    }
    if ((permissions & DataLakeSasPermissions::Delete) == DataLakeSasPermissions::Delete)
    {
      Permissions += "d";
    }
    if ((permissions & DataLakeSasPermissions::List) == DataLakeSasPermissions::List)
    {
      Permissions += "l";
    }
    if ((permissions & DataLakeSasPermissions::Move) == DataLakeSasPermissions::Move)
    {
      Permissions += "m";
    }
    if ((permissions & DataLakeSasPermissions::Execute) == DataLakeSasPermissions::Execute)
    {
      Permissions += "e";
    }
    if ((permissions & DataLakeSasPermissions::ManageOwnership)
        == DataLakeSasPermissions::ManageOwnership)
    {
      Permissions += "o";
    }
    if ((permissions & DataLakeSasPermissions::ManageAccessControl)
        == DataLakeSasPermissions::ManageAccessControl)
    {
      Permissions += "p";
    }
  }

  std::string DataLakeSasBuilder::ToSasQueryParameters(const SharedKeyCredential& credential)
  {
    std::string canonicalName = "/blob/" + credential.AccountName + "/" + FileSystemName;
    if (Resource == DataLakeSasResource::File)
    {
      canonicalName += "/" + Path;
    }
    std::string protocol = SasProtocolToString(Protocol);
    std::string resource = DataLakeSasResourceToString(Resource);

    std::string stringToSign = Permissions + "\n" + (StartsOn.HasValue() ? StartsOn.GetValue() : "")
        + "\n" + ExpiresOn + "\n" + canonicalName + "\n" + Identifier + "\n"
        + (IPRange.HasValue() ? IPRange.GetValue() : "") + "\n" + protocol + "\n" + Version + "\n"
        + resource + "\n" + "\n" + CacheControl + "\n" + ContentDisposition + "\n" + ContentEncoding
        + "\n" + ContentLanguage + "\n" + ContentType;

    std::string signature = Base64Encode(
        Storage::Details::HmacSha256(stringToSign, Base64Decode(credential.GetAccountKey())));

    Azure::Core::Http::Url builder;
    builder.AppendQueryParameter("sv", Version);
    builder.AppendQueryParameter("spr", protocol);
    if (StartsOn.HasValue())
    {
      builder.AppendQueryParameter("st", StartsOn.GetValue());
    }
    if (!ExpiresOn.empty())
    {
      builder.AppendQueryParameter("se", ExpiresOn);
    }
    if (IPRange.HasValue())
    {
      builder.AppendQueryParameter("sip", IPRange.GetValue());
    }
    if (!Identifier.empty())
    {
      builder.AppendQueryParameter("si", Identifier);
    }
    builder.AppendQueryParameter("sr", resource);
    if (!Permissions.empty())
    {
      builder.AppendQueryParameter("sp", Permissions);
    }
    builder.AppendQueryParameter("sig", signature);
    if (!CacheControl.empty())
    {
      builder.AppendQueryParameter("rscc", CacheControl);
    }
    if (!ContentDisposition.empty())
    {
      builder.AppendQueryParameter("rscd", ContentDisposition);
    }
    if (!ContentEncoding.empty())
    {
      builder.AppendQueryParameter("rsce", ContentEncoding);
    }
    if (!ContentLanguage.empty())
    {
      builder.AppendQueryParameter("rscl", ContentLanguage);
    }
    if (!ContentType.empty())
    {
      builder.AppendQueryParameter("rsct", ContentType);
    }

    return builder.GetAbsoluteUrl();
  }

  std::string DataLakeSasBuilder::ToSasQueryParameters(
      const UserDelegationKey& userDelegationKey,
      const std::string& accountName)
  {
    std::string canonicalName = "/blob/" + accountName + "/" + FileSystemName;
    if (Resource == DataLakeSasResource::File || Resource == DataLakeSasResource::Directory)
    {
      canonicalName += "/" + Path;
    }
    std::string protocol = SasProtocolToString(Protocol);
    std::string resource = DataLakeSasResourceToString(Resource);

    std::string stringToSign = Permissions + "\n" + (StartsOn.HasValue() ? StartsOn.GetValue() : "")
        + "\n" + ExpiresOn + "\n" + canonicalName + "\n" + userDelegationKey.SignedObjectId + "\n"
        + userDelegationKey.SignedTenantId + "\n" + userDelegationKey.SignedStartsOn + "\n"
        + userDelegationKey.SignedExpiresOn + "\n" + userDelegationKey.SignedService + "\n"
        + userDelegationKey.SignedVersion + "\n" + PreauthorizedAgentObjectId + "\n" + AgentObjectId
        + "\n" + CorrelationId + "\n" + (IPRange.HasValue() ? IPRange.GetValue() : "") + "\n"
        + protocol + "\n" + Version + "\n" + resource + "\n" + "\n" + CacheControl + "\n"
        + ContentDisposition + "\n" + ContentEncoding + "\n" + ContentLanguage + "\n" + ContentType;

    std::string signature = Base64Encode(
        Storage::Details::HmacSha256(stringToSign, Base64Decode(userDelegationKey.Value)));

    Azure::Core::Http::Url builder;
    builder.AppendQueryParameter("sv", Version);
    builder.AppendQueryParameter("sr", resource);
    if (StartsOn.HasValue())
    {
      builder.AppendQueryParameter("st", StartsOn.GetValue());
    }
    builder.AppendQueryParameter("se", ExpiresOn);
    builder.AppendQueryParameter("sp", Permissions);
    if (IPRange.HasValue())
    {
      builder.AppendQueryParameter("sip", IPRange.GetValue());
    }
    builder.AppendQueryParameter("spr", protocol);
    builder.AppendQueryParameter("skoid", userDelegationKey.SignedObjectId);
    builder.AppendQueryParameter("sktid", userDelegationKey.SignedTenantId);
    builder.AppendQueryParameter("skt", userDelegationKey.SignedStartsOn);
    builder.AppendQueryParameter("ske", userDelegationKey.SignedExpiresOn);
    builder.AppendQueryParameter("sks", userDelegationKey.SignedService);
    builder.AppendQueryParameter("skv", userDelegationKey.SignedVersion);
    if (!PreauthorizedAgentObjectId.empty())
    {
      builder.AppendQueryParameter("saoid", PreauthorizedAgentObjectId);
    }
    if (!AgentObjectId.empty())
    {
      builder.AppendQueryParameter("suoid", AgentObjectId);
    }
    if (!CorrelationId.empty())
    {
      builder.AppendQueryParameter("scid", CorrelationId);
    }
    if (DirectoryDepth.HasValue())
    {
      builder.AppendQueryParameter("sdd", std::to_string(DirectoryDepth.GetValue()));
    }
    if (!CacheControl.empty())
    {
      builder.AppendQueryParameter("rscc", CacheControl);
    }
    if (!ContentDisposition.empty())
    {
      builder.AppendQueryParameter("rscd", ContentDisposition);
    }
    if (!ContentEncoding.empty())
    {
      builder.AppendQueryParameter("rsce", ContentEncoding);
    }
    if (!ContentLanguage.empty())
    {
      builder.AppendQueryParameter("rscl", ContentLanguage);
    }
    if (!ContentType.empty())
    {
      builder.AppendQueryParameter("rsct", ContentType);
    }
    builder.AppendQueryParameter("sig", signature);

    return builder.GetAbsoluteUrl();
  }

}}}} // namespace Azure::Storage::Files::DataLake

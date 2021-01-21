// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/datalake/datalake_sas_builder.hpp"

#include <azure/core/http/http.hpp>
#include <azure/storage/common/crypt.hpp>

namespace Azure { namespace Storage { namespace Sas {
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

  void DataLakeSasBuilder::SetPermissions(DataLakeFileSystemSasPermissions permissions)
  {
    Permissions.clear();
    // The order matters
    if ((permissions & DataLakeFileSystemSasPermissions::Read)
        == DataLakeFileSystemSasPermissions::Read)
    {
      Permissions += "r";
    }
    if ((permissions & DataLakeFileSystemSasPermissions::Add)
        == DataLakeFileSystemSasPermissions::Add)
    {
      Permissions += "a";
    }
    if ((permissions & DataLakeFileSystemSasPermissions::Create)
        == DataLakeFileSystemSasPermissions::Create)
    {
      Permissions += "c";
    }
    if ((permissions & DataLakeFileSystemSasPermissions::Write)
        == DataLakeFileSystemSasPermissions::Write)
    {
      Permissions += "w";
    }
    if ((permissions & DataLakeFileSystemSasPermissions::Delete)
        == DataLakeFileSystemSasPermissions::Delete)
    {
      Permissions += "d";
    }
    if ((permissions & DataLakeFileSystemSasPermissions::List)
        == DataLakeFileSystemSasPermissions::List)
    {
      Permissions += "l";
    }
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

  std::string DataLakeSasBuilder::GenerateSasToken(const StorageSharedKeyCredential& credential)
  {
    std::string canonicalName = "/blob/" + credential.AccountName + "/" + FileSystemName;
    if (Resource == DataLakeSasResource::File)
    {
      canonicalName += "/" + Path;
    }
    std::string protocol = Details::SasProtocolToString(Protocol);
    std::string resource = DataLakeSasResourceToString(Resource);

    std::string startsOnStr = StartsOn.HasValue()
        ? StartsOn.GetValue().GetRfc3339String(Azure::Core::DateTime::TimeFractionFormat::Truncate)
        : "";
    std::string expiresOnStr = Identifier.empty()
        ? ExpiresOn.GetRfc3339String(Azure::Core::DateTime::TimeFractionFormat::Truncate)
        : "";

    std::string stringToSign = Permissions + "\n" + startsOnStr + "\n" + expiresOnStr + "\n"
        + canonicalName + "\n" + Identifier + "\n" + (IPRange.HasValue() ? IPRange.GetValue() : "")
        + "\n" + protocol + "\n" + Storage::Details::DefaultSasVersion + "\n" + resource + "\n"
        + "\n" + CacheControl + "\n" + ContentDisposition + "\n" + ContentEncoding + "\n"
        + ContentLanguage + "\n" + ContentType;

    std::string signature = Azure::Core::Base64Encode(Storage::Details::HmacSha256(
        std::vector<uint8_t>(stringToSign.begin(), stringToSign.end()),
        Azure::Core::Base64Decode(credential.GetAccountKey())));

    Azure::Core::Http::Url builder;
    builder.AppendQueryParameter(
        "sv", Storage::Details::UrlEncodeQueryParameter(Storage::Details::DefaultSasVersion));
    builder.AppendQueryParameter("spr", Storage::Details::UrlEncodeQueryParameter(protocol));
    if (!startsOnStr.empty())
    {
      builder.AppendQueryParameter("st", Storage::Details::UrlEncodeQueryParameter(startsOnStr));
    }
    if (!expiresOnStr.empty())
    {
      builder.AppendQueryParameter("se", Storage::Details::UrlEncodeQueryParameter(expiresOnStr));
    }
    if (IPRange.HasValue())
    {
      builder.AppendQueryParameter(
          "sip", Storage::Details::UrlEncodeQueryParameter(IPRange.GetValue()));
    }
    if (!Identifier.empty())
    {
      builder.AppendQueryParameter("si", Storage::Details::UrlEncodeQueryParameter(Identifier));
    }
    builder.AppendQueryParameter("sr", Storage::Details::UrlEncodeQueryParameter(resource));
    if (!Permissions.empty())
    {
      builder.AppendQueryParameter("sp", Storage::Details::UrlEncodeQueryParameter(Permissions));
    }
    builder.AppendQueryParameter("sig", Storage::Details::UrlEncodeQueryParameter(signature));
    if (!CacheControl.empty())
    {
      builder.AppendQueryParameter("rscc", Storage::Details::UrlEncodeQueryParameter(CacheControl));
    }
    if (!ContentDisposition.empty())
    {
      builder.AppendQueryParameter(
          "rscd", Storage::Details::UrlEncodeQueryParameter(ContentDisposition));
    }
    if (!ContentEncoding.empty())
    {
      builder.AppendQueryParameter(
          "rsce", Storage::Details::UrlEncodeQueryParameter(ContentEncoding));
    }
    if (!ContentLanguage.empty())
    {
      builder.AppendQueryParameter(
          "rscl", Storage::Details::UrlEncodeQueryParameter(ContentLanguage));
    }
    if (!ContentType.empty())
    {
      builder.AppendQueryParameter("rsct", Storage::Details::UrlEncodeQueryParameter(ContentType));
    }

    return builder.GetAbsoluteUrl();
  }

  std::string DataLakeSasBuilder::GenerateSasToken(
      const Files::DataLake::Models::UserDelegationKey& userDelegationKey,
      const std::string& accountName)
  {
    std::string canonicalName = "/blob/" + accountName + "/" + FileSystemName;
    if (Resource == DataLakeSasResource::File || Resource == DataLakeSasResource::Directory)
    {
      canonicalName += "/" + Path;
    }
    std::string protocol = Details::SasProtocolToString(Protocol);
    std::string resource = DataLakeSasResourceToString(Resource);

    std::string startsOnStr = StartsOn.HasValue()
        ? StartsOn.GetValue().GetRfc3339String(Azure::Core::DateTime::TimeFractionFormat::Truncate)
        : "";
    std::string expiresOnStr
        = ExpiresOn.GetRfc3339String(Azure::Core::DateTime::TimeFractionFormat::Truncate);
    std::string signedStartsOnStr = userDelegationKey.SignedStartsOn.GetRfc3339String(
        Azure::Core::DateTime::TimeFractionFormat::Truncate);
    std::string signedExpiresOnStr = userDelegationKey.SignedExpiresOn.GetRfc3339String(
        Azure::Core::DateTime::TimeFractionFormat::Truncate);

    std::string stringToSign = Permissions + "\n" + startsOnStr + "\n" + expiresOnStr + "\n"
        + canonicalName + "\n" + userDelegationKey.SignedObjectId + "\n"
        + userDelegationKey.SignedTenantId + "\n" + signedStartsOnStr + "\n" + signedExpiresOnStr
        + "\n" + userDelegationKey.SignedService + "\n" + userDelegationKey.SignedVersion + "\n"
        + PreauthorizedAgentObjectId + "\n" + AgentObjectId + "\n" + CorrelationId + "\n"
        + (IPRange.HasValue() ? IPRange.GetValue() : "") + "\n" + protocol + "\n"
        + Storage::Details::DefaultSasVersion + "\n" + resource + "\n" + "\n" + CacheControl + "\n"
        + ContentDisposition + "\n" + ContentEncoding + "\n" + ContentLanguage + "\n" + ContentType;

    std::string signature = Azure::Core::Base64Encode(Storage::Details::HmacSha256(
        std::vector<uint8_t>(stringToSign.begin(), stringToSign.end()),
        Azure::Core::Base64Decode(userDelegationKey.Value)));

    Azure::Core::Http::Url builder;
    builder.AppendQueryParameter(
        "sv", Storage::Details::UrlEncodeQueryParameter(Storage::Details::DefaultSasVersion));
    builder.AppendQueryParameter("sr", Storage::Details::UrlEncodeQueryParameter(resource));
    if (!startsOnStr.empty())
    {
      builder.AppendQueryParameter("st", Storage::Details::UrlEncodeQueryParameter(startsOnStr));
    }
    builder.AppendQueryParameter("se", Storage::Details::UrlEncodeQueryParameter(expiresOnStr));
    builder.AppendQueryParameter("sp", Permissions);
    if (IPRange.HasValue())
    {
      builder.AppendQueryParameter(
          "sip", Storage::Details::UrlEncodeQueryParameter(IPRange.GetValue()));
    }
    builder.AppendQueryParameter("spr", Storage::Details::UrlEncodeQueryParameter(protocol));
    builder.AppendQueryParameter(
        "skoid", Storage::Details::UrlEncodeQueryParameter(userDelegationKey.SignedObjectId));
    builder.AppendQueryParameter(
        "sktid", Storage::Details::UrlEncodeQueryParameter(userDelegationKey.SignedTenantId));
    builder.AppendQueryParameter(
        "skt", Storage::Details::UrlEncodeQueryParameter(signedStartsOnStr));
    builder.AppendQueryParameter(
        "ske", Storage::Details::UrlEncodeQueryParameter(signedExpiresOnStr));
    builder.AppendQueryParameter(
        "sks", Storage::Details::UrlEncodeQueryParameter(userDelegationKey.SignedService));
    builder.AppendQueryParameter(
        "skv", Storage::Details::UrlEncodeQueryParameter(userDelegationKey.SignedVersion));
    if (!PreauthorizedAgentObjectId.empty())
    {
      builder.AppendQueryParameter(
          "saoid", Storage::Details::UrlEncodeQueryParameter(PreauthorizedAgentObjectId));
    }
    if (!AgentObjectId.empty())
    {
      builder.AppendQueryParameter(
          "suoid", Storage::Details::UrlEncodeQueryParameter(AgentObjectId));
    }
    if (!CorrelationId.empty())
    {
      builder.AppendQueryParameter(
          "scid", Storage::Details::UrlEncodeQueryParameter(CorrelationId));
    }
    if (DirectoryDepth.HasValue())
    {
      builder.AppendQueryParameter(
          "sdd",
          Storage::Details::UrlEncodeQueryParameter(std::to_string(DirectoryDepth.GetValue())));
    }
    if (!CacheControl.empty())
    {
      builder.AppendQueryParameter("rscc", Storage::Details::UrlEncodeQueryParameter(CacheControl));
    }
    if (!ContentDisposition.empty())
    {
      builder.AppendQueryParameter(
          "rscd", Storage::Details::UrlEncodeQueryParameter(ContentDisposition));
    }
    if (!ContentEncoding.empty())
    {
      builder.AppendQueryParameter(
          "rsce", Storage::Details::UrlEncodeQueryParameter(ContentEncoding));
    }
    if (!ContentLanguage.empty())
    {
      builder.AppendQueryParameter(
          "rscl", Storage::Details::UrlEncodeQueryParameter(ContentLanguage));
    }
    if (!ContentType.empty())
    {
      builder.AppendQueryParameter("rsct", Storage::Details::UrlEncodeQueryParameter(ContentType));
    }
    builder.AppendQueryParameter("sig", Storage::Details::UrlEncodeQueryParameter(signature));

    return builder.GetAbsoluteUrl();
  }

}}} // namespace Azure::Storage::Sas

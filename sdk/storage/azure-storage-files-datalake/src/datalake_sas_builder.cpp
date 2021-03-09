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
        throw std::invalid_argument("unknown DataLakeSasResource value");
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

    std::string startsOnStr = StartsOn.HasValue() ? StartsOn.GetValue().ToString(
                                  Azure::Core::DateTime::DateFormat::Rfc3339,
                                  Azure::Core::DateTime::TimeFractionFormat::Truncate)
                                                  : "";
    std::string expiresOnStr = Identifier.empty() ? ExpiresOn.ToString(
                                   Azure::Core::DateTime::DateFormat::Rfc3339,
                                   Azure::Core::DateTime::TimeFractionFormat::Truncate)
                                                  : "";

    std::string stringToSign = Permissions + "\n" + startsOnStr + "\n" + expiresOnStr + "\n"
        + canonicalName + "\n" + Identifier + "\n" + (IPRange.HasValue() ? IPRange.GetValue() : "")
        + "\n" + protocol + "\n" + Storage::_detail::DefaultSasVersion + "\n" + resource + "\n"
        + "\n" + CacheControl + "\n" + ContentDisposition + "\n" + ContentEncoding + "\n"
        + ContentLanguage + "\n" + ContentType;

    std::string signature = Azure::Core::Base64Encode(Storage::_detail::HmacSha256(
        std::vector<uint8_t>(stringToSign.begin(), stringToSign.end()),
        Azure::Core::Base64Decode(credential.GetAccountKey())));

    Azure::Core::Http::Url builder;
    builder.AppendQueryParameter(
        "sv", Storage::_detail::UrlEncodeQueryParameter(Storage::_detail::DefaultSasVersion));
    builder.AppendQueryParameter("spr", Storage::_detail::UrlEncodeQueryParameter(protocol));
    if (!startsOnStr.empty())
    {
      builder.AppendQueryParameter("st", Storage::_detail::UrlEncodeQueryParameter(startsOnStr));
    }
    if (!expiresOnStr.empty())
    {
      builder.AppendQueryParameter("se", Storage::_detail::UrlEncodeQueryParameter(expiresOnStr));
    }
    if (IPRange.HasValue())
    {
      builder.AppendQueryParameter(
          "sip", Storage::_detail::UrlEncodeQueryParameter(IPRange.GetValue()));
    }
    if (!Identifier.empty())
    {
      builder.AppendQueryParameter("si", Storage::_detail::UrlEncodeQueryParameter(Identifier));
    }
    builder.AppendQueryParameter("sr", Storage::_detail::UrlEncodeQueryParameter(resource));
    if (!Permissions.empty())
    {
      builder.AppendQueryParameter("sp", Storage::_detail::UrlEncodeQueryParameter(Permissions));
    }
    builder.AppendQueryParameter("sig", Storage::_detail::UrlEncodeQueryParameter(signature));
    if (!CacheControl.empty())
    {
      builder.AppendQueryParameter("rscc", Storage::_detail::UrlEncodeQueryParameter(CacheControl));
    }
    if (!ContentDisposition.empty())
    {
      builder.AppendQueryParameter(
          "rscd", Storage::_detail::UrlEncodeQueryParameter(ContentDisposition));
    }
    if (!ContentEncoding.empty())
    {
      builder.AppendQueryParameter(
          "rsce", Storage::_detail::UrlEncodeQueryParameter(ContentEncoding));
    }
    if (!ContentLanguage.empty())
    {
      builder.AppendQueryParameter(
          "rscl", Storage::_detail::UrlEncodeQueryParameter(ContentLanguage));
    }
    if (!ContentType.empty())
    {
      builder.AppendQueryParameter("rsct", Storage::_detail::UrlEncodeQueryParameter(ContentType));
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

    std::string startsOnStr = StartsOn.HasValue() ? StartsOn.GetValue().ToString(
                                  Azure::Core::DateTime::DateFormat::Rfc3339,
                                  Azure::Core::DateTime::TimeFractionFormat::Truncate)
                                                  : "";
    std::string expiresOnStr = ExpiresOn.ToString(
        Azure::Core::DateTime::DateFormat::Rfc3339,
        Azure::Core::DateTime::TimeFractionFormat::Truncate);
    std::string signedStartsOnStr = userDelegationKey.SignedStartsOn.ToString(
        Azure::Core::DateTime::DateFormat::Rfc3339,
        Azure::Core::DateTime::TimeFractionFormat::Truncate);
    std::string signedExpiresOnStr = userDelegationKey.SignedExpiresOn.ToString(
        Azure::Core::DateTime::DateFormat::Rfc3339,
        Azure::Core::DateTime::TimeFractionFormat::Truncate);

    std::string stringToSign = Permissions + "\n" + startsOnStr + "\n" + expiresOnStr + "\n"
        + canonicalName + "\n" + userDelegationKey.SignedObjectId + "\n"
        + userDelegationKey.SignedTenantId + "\n" + signedStartsOnStr + "\n" + signedExpiresOnStr
        + "\n" + userDelegationKey.SignedService + "\n" + userDelegationKey.SignedVersion + "\n"
        + PreauthorizedAgentObjectId + "\n" + AgentObjectId + "\n" + CorrelationId + "\n"
        + (IPRange.HasValue() ? IPRange.GetValue() : "") + "\n" + protocol + "\n"
        + Storage::_detail::DefaultSasVersion + "\n" + resource + "\n" + "\n" + CacheControl + "\n"
        + ContentDisposition + "\n" + ContentEncoding + "\n" + ContentLanguage + "\n" + ContentType;

    std::string signature = Azure::Core::Base64Encode(Storage::_detail::HmacSha256(
        std::vector<uint8_t>(stringToSign.begin(), stringToSign.end()),
        Azure::Core::Base64Decode(userDelegationKey.Value)));

    Azure::Core::Http::Url builder;
    builder.AppendQueryParameter(
        "sv", Storage::_detail::UrlEncodeQueryParameter(Storage::_detail::DefaultSasVersion));
    builder.AppendQueryParameter("sr", Storage::_detail::UrlEncodeQueryParameter(resource));
    if (!startsOnStr.empty())
    {
      builder.AppendQueryParameter("st", Storage::_detail::UrlEncodeQueryParameter(startsOnStr));
    }
    builder.AppendQueryParameter("se", Storage::_detail::UrlEncodeQueryParameter(expiresOnStr));
    builder.AppendQueryParameter("sp", Permissions);
    if (IPRange.HasValue())
    {
      builder.AppendQueryParameter(
          "sip", Storage::_detail::UrlEncodeQueryParameter(IPRange.GetValue()));
    }
    builder.AppendQueryParameter("spr", Storage::_detail::UrlEncodeQueryParameter(protocol));
    builder.AppendQueryParameter(
        "skoid", Storage::_detail::UrlEncodeQueryParameter(userDelegationKey.SignedObjectId));
    builder.AppendQueryParameter(
        "sktid", Storage::_detail::UrlEncodeQueryParameter(userDelegationKey.SignedTenantId));
    builder.AppendQueryParameter(
        "skt", Storage::_detail::UrlEncodeQueryParameter(signedStartsOnStr));
    builder.AppendQueryParameter(
        "ske", Storage::_detail::UrlEncodeQueryParameter(signedExpiresOnStr));
    builder.AppendQueryParameter(
        "sks", Storage::_detail::UrlEncodeQueryParameter(userDelegationKey.SignedService));
    builder.AppendQueryParameter(
        "skv", Storage::_detail::UrlEncodeQueryParameter(userDelegationKey.SignedVersion));
    if (!PreauthorizedAgentObjectId.empty())
    {
      builder.AppendQueryParameter(
          "saoid", Storage::_detail::UrlEncodeQueryParameter(PreauthorizedAgentObjectId));
    }
    if (!AgentObjectId.empty())
    {
      builder.AppendQueryParameter(
          "suoid", Storage::_detail::UrlEncodeQueryParameter(AgentObjectId));
    }
    if (!CorrelationId.empty())
    {
      builder.AppendQueryParameter(
          "scid", Storage::_detail::UrlEncodeQueryParameter(CorrelationId));
    }
    if (DirectoryDepth.HasValue())
    {
      builder.AppendQueryParameter(
          "sdd",
          Storage::_detail::UrlEncodeQueryParameter(std::to_string(DirectoryDepth.GetValue())));
    }
    if (!CacheControl.empty())
    {
      builder.AppendQueryParameter("rscc", Storage::_detail::UrlEncodeQueryParameter(CacheControl));
    }
    if (!ContentDisposition.empty())
    {
      builder.AppendQueryParameter(
          "rscd", Storage::_detail::UrlEncodeQueryParameter(ContentDisposition));
    }
    if (!ContentEncoding.empty())
    {
      builder.AppendQueryParameter(
          "rsce", Storage::_detail::UrlEncodeQueryParameter(ContentEncoding));
    }
    if (!ContentLanguage.empty())
    {
      builder.AppendQueryParameter(
          "rscl", Storage::_detail::UrlEncodeQueryParameter(ContentLanguage));
    }
    if (!ContentType.empty())
    {
      builder.AppendQueryParameter("rsct", Storage::_detail::UrlEncodeQueryParameter(ContentType));
    }
    builder.AppendQueryParameter("sig", Storage::_detail::UrlEncodeQueryParameter(signature));

    return builder.GetAbsoluteUrl();
  }

}}} // namespace Azure::Storage::Sas

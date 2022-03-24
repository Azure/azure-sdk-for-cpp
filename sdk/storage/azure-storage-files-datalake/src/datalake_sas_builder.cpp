// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/datalake/datalake_sas_builder.hpp"

#include <azure/core/http/http.hpp>
#include <azure/storage/common/crypt.hpp>

/* cSpell:ignore rscc, rscd, rsce, rscl, rsct, skoid, sktid, saoid, suoid, scid */

namespace Azure { namespace Storage { namespace Sas {
  namespace {
    constexpr static const char* SasVersion = "2020-02-10";

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
        throw std::invalid_argument("Unknown DataLakeSasResource value.");
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
    std::string protocol = _detail::SasProtocolToString(Protocol);
    std::string resource = DataLakeSasResourceToString(Resource);

    std::string startsOnStr = StartsOn.HasValue()
        ? StartsOn.Value().ToString(
            Azure::DateTime::DateFormat::Rfc3339, Azure::DateTime::TimeFractionFormat::Truncate)
        : "";
    std::string expiresOnStr = Identifier.empty()
        ? ExpiresOn.ToString(
            Azure::DateTime::DateFormat::Rfc3339, Azure::DateTime::TimeFractionFormat::Truncate)
        : "";

    std::string stringToSign = Permissions + "\n" + startsOnStr + "\n" + expiresOnStr + "\n"
        + canonicalName + "\n" + Identifier + "\n" + (IPRange.HasValue() ? IPRange.Value() : "")
        + "\n" + protocol + "\n" + SasVersion + "\n" + resource + "\n" + "\n" + CacheControl + "\n"
        + ContentDisposition + "\n" + ContentEncoding + "\n" + ContentLanguage + "\n" + ContentType;

    std::string signature = Azure::Core::Convert::Base64Encode(_internal::HmacSha256(
        std::vector<uint8_t>(stringToSign.begin(), stringToSign.end()),
        Azure::Core::Convert::Base64Decode(credential.GetAccountKey())));

    Azure::Core::Url builder;
    builder.AppendQueryParameter("sv", _internal::UrlEncodeQueryParameter(SasVersion));
    builder.AppendQueryParameter("spr", _internal::UrlEncodeQueryParameter(protocol));
    if (!startsOnStr.empty())
    {
      builder.AppendQueryParameter("st", _internal::UrlEncodeQueryParameter(startsOnStr));
    }
    if (!expiresOnStr.empty())
    {
      builder.AppendQueryParameter("se", _internal::UrlEncodeQueryParameter(expiresOnStr));
    }
    if (IPRange.HasValue())
    {
      builder.AppendQueryParameter("sip", _internal::UrlEncodeQueryParameter(IPRange.Value()));
    }
    if (!Identifier.empty())
    {
      builder.AppendQueryParameter("si", _internal::UrlEncodeQueryParameter(Identifier));
    }
    builder.AppendQueryParameter("sr", _internal::UrlEncodeQueryParameter(resource));
    if (!Permissions.empty())
    {
      builder.AppendQueryParameter("sp", _internal::UrlEncodeQueryParameter(Permissions));
    }
    builder.AppendQueryParameter("sig", _internal::UrlEncodeQueryParameter(signature));
    if (!CacheControl.empty())
    {
      builder.AppendQueryParameter("rscc", _internal::UrlEncodeQueryParameter(CacheControl));
    }
    if (!ContentDisposition.empty())
    {
      builder.AppendQueryParameter("rscd", _internal::UrlEncodeQueryParameter(ContentDisposition));
    }
    if (!ContentEncoding.empty())
    {
      builder.AppendQueryParameter("rsce", _internal::UrlEncodeQueryParameter(ContentEncoding));
    }
    if (!ContentLanguage.empty())
    {
      builder.AppendQueryParameter("rscl", _internal::UrlEncodeQueryParameter(ContentLanguage));
    }
    if (!ContentType.empty())
    {
      builder.AppendQueryParameter("rsct", _internal::UrlEncodeQueryParameter(ContentType));
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
    std::string protocol = _detail::SasProtocolToString(Protocol);
    std::string resource = DataLakeSasResourceToString(Resource);

    std::string startsOnStr = StartsOn.HasValue()
        ? StartsOn.Value().ToString(
            Azure::DateTime::DateFormat::Rfc3339, Azure::DateTime::TimeFractionFormat::Truncate)
        : "";
    std::string expiresOnStr = ExpiresOn.ToString(
        Azure::DateTime::DateFormat::Rfc3339, Azure::DateTime::TimeFractionFormat::Truncate);
    std::string signedStartsOnStr = userDelegationKey.SignedStartsOn.ToString(
        Azure::DateTime::DateFormat::Rfc3339, Azure::DateTime::TimeFractionFormat::Truncate);
    std::string signedExpiresOnStr = userDelegationKey.SignedExpiresOn.ToString(
        Azure::DateTime::DateFormat::Rfc3339, Azure::DateTime::TimeFractionFormat::Truncate);

    std::string stringToSign = Permissions + "\n" + startsOnStr + "\n" + expiresOnStr + "\n"
        + canonicalName + "\n" + userDelegationKey.SignedObjectId + "\n"
        + userDelegationKey.SignedTenantId + "\n" + signedStartsOnStr + "\n" + signedExpiresOnStr
        + "\n" + userDelegationKey.SignedService + "\n" + userDelegationKey.SignedVersion + "\n"
        + PreauthorizedAgentObjectId + "\n" + AgentObjectId + "\n" + CorrelationId + "\n"
        + (IPRange.HasValue() ? IPRange.Value() : "") + "\n" + protocol + "\n" + SasVersion + "\n"
        + resource + "\n" + "\n" + CacheControl + "\n" + ContentDisposition + "\n" + ContentEncoding
        + "\n" + ContentLanguage + "\n" + ContentType;

    std::string signature = Azure::Core::Convert::Base64Encode(_internal::HmacSha256(
        std::vector<uint8_t>(stringToSign.begin(), stringToSign.end()),
        Azure::Core::Convert::Base64Decode(userDelegationKey.Value)));

    Azure::Core::Url builder;
    builder.AppendQueryParameter("sv", _internal::UrlEncodeQueryParameter(SasVersion));
    builder.AppendQueryParameter("sr", _internal::UrlEncodeQueryParameter(resource));
    if (!startsOnStr.empty())
    {
      builder.AppendQueryParameter("st", _internal::UrlEncodeQueryParameter(startsOnStr));
    }
    builder.AppendQueryParameter("se", _internal::UrlEncodeQueryParameter(expiresOnStr));
    builder.AppendQueryParameter("sp", Permissions);
    if (IPRange.HasValue())
    {
      builder.AppendQueryParameter("sip", _internal::UrlEncodeQueryParameter(IPRange.Value()));
    }
    builder.AppendQueryParameter("spr", _internal::UrlEncodeQueryParameter(protocol));
    builder.AppendQueryParameter(
        "skoid", _internal::UrlEncodeQueryParameter(userDelegationKey.SignedObjectId));
    builder.AppendQueryParameter(
        "sktid", _internal::UrlEncodeQueryParameter(userDelegationKey.SignedTenantId));
    builder.AppendQueryParameter("skt", _internal::UrlEncodeQueryParameter(signedStartsOnStr));
    builder.AppendQueryParameter("ske", _internal::UrlEncodeQueryParameter(signedExpiresOnStr));
    builder.AppendQueryParameter(
        "sks", _internal::UrlEncodeQueryParameter(userDelegationKey.SignedService));
    builder.AppendQueryParameter(
        "skv", _internal::UrlEncodeQueryParameter(userDelegationKey.SignedVersion));
    if (!PreauthorizedAgentObjectId.empty())
    {
      builder.AppendQueryParameter(
          "saoid", _internal::UrlEncodeQueryParameter(PreauthorizedAgentObjectId));
    }
    if (!AgentObjectId.empty())
    {
      builder.AppendQueryParameter("suoid", _internal::UrlEncodeQueryParameter(AgentObjectId));
    }
    if (!CorrelationId.empty())
    {
      builder.AppendQueryParameter("scid", _internal::UrlEncodeQueryParameter(CorrelationId));
    }
    if (DirectoryDepth.HasValue())
    {
      builder.AppendQueryParameter(
          "sdd", _internal::UrlEncodeQueryParameter(std::to_string(DirectoryDepth.Value())));
    }
    if (!CacheControl.empty())
    {
      builder.AppendQueryParameter("rscc", _internal::UrlEncodeQueryParameter(CacheControl));
    }
    if (!ContentDisposition.empty())
    {
      builder.AppendQueryParameter("rscd", _internal::UrlEncodeQueryParameter(ContentDisposition));
    }
    if (!ContentEncoding.empty())
    {
      builder.AppendQueryParameter("rsce", _internal::UrlEncodeQueryParameter(ContentEncoding));
    }
    if (!ContentLanguage.empty())
    {
      builder.AppendQueryParameter("rscl", _internal::UrlEncodeQueryParameter(ContentLanguage));
    }
    if (!ContentType.empty())
    {
      builder.AppendQueryParameter("rsct", _internal::UrlEncodeQueryParameter(ContentType));
    }
    builder.AppendQueryParameter("sig", _internal::UrlEncodeQueryParameter(signature));

    return builder.GetAbsoluteUrl();
  }

}}} // namespace Azure::Storage::Sas

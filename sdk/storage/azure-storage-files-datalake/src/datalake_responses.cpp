// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/datalake/datalake_responses.hpp"

#include "azure/storage/files/datalake/datalake_utilities.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake {
  namespace Models {

    Acl Acl::FromString(const std::string& aclString)
    {
      std::string::const_iterator cur = aclString.begin();
      std::string str1 = _detail::GetSubstringTillDelimiter(':', aclString, cur);
      std::string str2 = _detail::GetSubstringTillDelimiter(':', aclString, cur);
      std::string str3 = _detail::GetSubstringTillDelimiter(':', aclString, cur);
      std::string str4 = _detail::GetSubstringTillDelimiter(':', aclString, cur);

      Acl acl;
      if (str4.empty())
      {
        // Scope is implicit in this case.
        acl.Type = std::move(str1);
        acl.Id = std::move(str2);
        acl.Permissions = std::move(str3);
      }
      else
      {
        // Scope is not implicit in this case.
        acl.Scope = std::move(str1);
        acl.Type = std::move(str2);
        acl.Id = std::move(str3);
        acl.Permissions = std::move(str4);
      }

      return acl;
    }

    std::string Acl::ToString(const Acl& acl)
    {
      std::string result;
      if (acl.Scope.empty())
      {
        result = acl.Type + ":" + acl.Id + ":" + acl.Permissions;
      }
      else
      {
        result = acl.Scope + ":" + acl.Type + ":" + acl.Id + ":" + acl.Permissions;
      }
      return result;
    }

    std::vector<Acl> Acl::DeserializeAcls(const std::string& aclsString)
    {
      std::vector<Acl> result;

      std::string::const_iterator cur = aclsString.begin();

      while (cur != aclsString.end())
      {
        result.emplace_back(FromString(_detail::GetSubstringTillDelimiter(',', aclsString, cur)));
      }

      return result;
    }
    std::string Acl::SerializeAcls(const std::vector<Acl>& aclArray)
    {
      std::string result;
      for (const auto& acl : aclArray)
      {
        result.append(ToString(acl) + ",");
      }
      if (!result.empty())
      {
        result.pop_back();
      }
      return result;
    }

  } // namespace Models

  void ListFileSystemsPagedResponse::CopyFromListBlobsContainersResult(
      Blobs::ListBlobContainersPagedResponse& result)
  {
    ServiceEndpoint = result.ServiceEndpoint;
    Prefix = result.Prefix;
    for (auto& item : result.Items)
    {
      Models::FileSystemItem fileSystem;
      fileSystem.Name = std::move(item.Name);
      fileSystem.Details.ETag = std::move(item.Details.ETag);
      fileSystem.Details.LastModified = std::move(item.Details.LastModified);
      fileSystem.Details.Metadata = std::move(item.Details.Metadata);
      if (item.Details.AccessType == Blobs::Models::PublicAccessType::BlobContainer)
      {
        fileSystem.Details.AccessType = Models::PublicAccessType::FileSystem;
      }
      else if (item.Details.AccessType == Blobs::Models::PublicAccessType::Blob)
      {
        fileSystem.Details.AccessType = Models::PublicAccessType::Path;
      }
      else if (item.Details.AccessType == Blobs::Models::PublicAccessType::None)
      {
        fileSystem.Details.AccessType = Models::PublicAccessType::None;
      }
      else
      {
        fileSystem.Details.AccessType
            = Models::PublicAccessType(item.Details.AccessType.ToString());
      }
      fileSystem.Details.HasImmutabilityPolicy = item.Details.HasImmutabilityPolicy;
      fileSystem.Details.HasLegalHold = item.Details.HasLegalHold;
      if (item.Details.LeaseDuration.HasValue())
      {
        fileSystem.Details.LeaseDuration
            = Models::LeaseDuration((item.Details.LeaseDuration.Value().ToString()));
      }
      fileSystem.Details.LeaseState = Models::LeaseState(item.Details.LeaseState.ToString());
      fileSystem.Details.LeaseStatus = Models::LeaseStatus(item.Details.LeaseStatus.ToString());

      Items.emplace_back(std::move(fileSystem));
    }
    CurrentPageToken = result.CurrentPageToken;
    NextPageToken = result.NextPageToken;
    RawResponse = std::move(result.RawResponse);
  }

  void ListFileSystemsPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    m_listBlobContainersPagedResponse.NextPage(context);
    CopyFromListBlobsContainersResult(m_listBlobContainersPagedResponse);
  }

}}}} // namespace Azure::Storage::Files::DataLake

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/datalake/datalake_responses.hpp"

#include "azure/storage/files/datalake/datalake_path_client.hpp"
#include "azure/storage/files/datalake/datalake_service_client.hpp"
#include "private/datalake_utilities.hpp"

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

  void ListFileSystemsPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    m_operationOptions.ContinuationToken = NextPageToken;
    *this = m_dataLakeServiceClient->ListFileSystems(m_operationOptions, context);
  }

  void ListPathsPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    *this = m_onNextPageFunc(NextPageToken.Value(), context);
  }

  void SetPathAccessControlListRecursivePagedResponse::OnNextPage(
      const Azure::Core::Context& context)
  {
    m_operationOptions.ContinuationToken = NextPageToken;
    if (m_mode == _detail::PathSetAccessControlRecursiveMode::Set)
    {
      *this = m_dataLakePathClient->SetAccessControlListRecursive(
          m_acls, m_operationOptions, context);
    }
    else if (m_mode == _detail::PathSetAccessControlRecursiveMode::Modify)
    {
      *this = m_dataLakePathClient->UpdateAccessControlListRecursive(
          m_acls, m_operationOptions, context);
    }
    else if (m_mode == _detail::PathSetAccessControlRecursiveMode::Remove)
    {
      *this = m_dataLakePathClient->RemoveAccessControlListRecursive(
          m_acls, m_operationOptions, context);
    }
    AZURE_UNREACHABLE_CODE();
  }

}}}} // namespace Azure::Storage::Files::DataLake

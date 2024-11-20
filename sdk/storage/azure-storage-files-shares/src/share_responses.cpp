// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/storage/files/shares/share_responses.hpp"

#include "azure/storage/files/shares/share_directory_client.hpp"
#include "azure/storage/files/shares/share_file_client.hpp"

#include <thread>

namespace Azure { namespace Storage { namespace Files { namespace Shares {
  namespace Models {

    namespace {

      RolePermissions ParseOctal(char c)
      {
        RolePermissions permissions = RolePermissions::None;
        int permissionValue = c - '0';
        if (permissionValue < 0 || permissionValue > 7)
        {
          throw std::invalid_argument("Permission value must be >0 and <7.");
        }
        if ((permissionValue & 4) == 4)
        {
          permissions = permissions | RolePermissions::Read;
        }
        if ((permissionValue & 2) == 2)
        {
          permissions = permissions | RolePermissions::Read;
        }
        if ((permissionValue & 2) == 1)
        {
          permissions = permissions | RolePermissions::Read;
        }
        return permissions;
      }

      char ToOctal(RolePermissions permissions)
      {
        char c = '0';
        if ((permissions & RolePermissions::Read) == RolePermissions::Read)
        {
          c += 4;
        }
        if ((permissions & RolePermissions::Write) == RolePermissions::Write)
        {
          c += 2;
        }
        if ((permissions & RolePermissions::Execute) == RolePermissions::Execute)
        {
          c += 1;
        }
        return c;
      }

      RolePermissions ParseSymbolic(std::string s, bool& setSticky)
      {
        if (s.length() != 3)
        {
          throw std::invalid_argument("Symbolic role permission format is invalid.");
        }

        RolePermissions permissions = RolePermissions::None;
        setSticky = false;
        // Read character
        if (s[0] == 'r')
        {
          permissions = permissions | RolePermissions ::Read;
        }
        else if (s[0] != '-')
        {
          throw std::invalid_argument("Invalid character in symbolic role permission: " + s[0]);
        }

        // Write character
        if (s[1] == 'w')
        {
          permissions = permissions | RolePermissions ::Write;
        }
        else if (s[1] != '-')
        {
          throw std::invalid_argument("Invalid character in symbolic role permission: " + s[1]);
        }

        // Execute character
        if (s[2] == 'x' || s[2] == 's' || s[2] == 't')
        {
          permissions = permissions | RolePermissions ::Execute;
          if (s[2] == 's' || s[2] == 't')
          {
            setSticky = true;
          }
        }
        if (s[2] == 'S' || s[2] == 'T')
        {
          setSticky = true;
        }

        if (s[2] != 'x' && s[2] != 's' && s[2] != 'S' && s[2] != 't' && s[2] != 'T' && s[2] != '-')
        {
          throw std::invalid_argument("Invalid character in symbolic role permission: " + s[2]);
        }

        return permissions;
      }

      std::string ToSymbolic(RolePermissions permissions)
      {
        std::string s;
        s.push_back((permissions & RolePermissions::Read) == RolePermissions::Read ? 'r' : '-');
        s.push_back((permissions & RolePermissions::Write) == RolePermissions::Write ? 'w' : '-');
        s.push_back(
            (permissions & RolePermissions::Execute) == RolePermissions::Execute ? 'x' : '-');
        return s;
      }
    } // namespace

    std::string NfsFileMode::ToOctalFileMode() const
    {
      int higherOrderDigit = 0;
      if (EffectiveUserIdentity)
      {
        higherOrderDigit |= 4;
      }
      if (EffectiveGroupIdentity)
      {
        higherOrderDigit |= 2;
      }
      if (StickyBit)
      {
        higherOrderDigit |= 1;
      }

      std::string modeString = "";
      modeString.push_back((char)(higherOrderDigit + '0'));
      modeString.push_back(ToOctal(Owner));
      modeString.push_back(ToOctal(Group));
      modeString.push_back(ToOctal(Other));
      return modeString;
    }

    std::string NfsFileMode::ToSymbolicFileMode() const
    {
      std::string modeString = "";
      modeString.append(ToSymbolic(Owner));
      modeString.append(ToSymbolic(Group));
      modeString.append(ToSymbolic(Other));
      if (EffectiveUserIdentity)
      {
        if (modeString[2] == 'x')
        {
          modeString[2] = 's';
        }
        else
        {
          modeString[2] = 'S';
        }
      }

      if (EffectiveGroupIdentity)
      {
        if (modeString[5] == 'x')
        {
          modeString[5] = 's';
        }
        else
        {
          modeString[5] = 'S';
        }
      }

      if (StickyBit)
      {
        if (modeString[8] == 'x')
        {
          modeString[8] = 't';
        }
        else
        {
          modeString[8] = 'T';
        }
      }
      return modeString;
    }

    NfsFileMode NfsFileMode::ParseOctalFileMode(const std::string& modeString)
    {
      if (modeString.length() != 4)
      {
        throw std::invalid_argument("modeString must be a 4-digit octal number.");
      }
      NfsFileMode mode;
      mode.Owner = ParseOctal(modeString[1]);
      mode.Group = ParseOctal(modeString[2]);
      mode.Other = ParseOctal(modeString[3]);

      int value = modeString[0] - '0';
      if ((value & 4) == 4)
      {
        mode.EffectiveUserIdentity = true;
      }

      if ((value & 2) == 2)
      {
        mode.EffectiveGroupIdentity = true;
      }

      if ((value & 1) == 1)
      {
        mode.StickyBit = true;
      }
      return mode;
    }

    NfsFileMode NfsFileMode::ParseSymbolicFileMode(const std::string& modeString)
    {
      if (modeString.length() != 9)
      {
        throw std::invalid_argument("modeString must be a 9-character octal number.");
      }
      NfsFileMode mode;
      mode.Owner = ParseSymbolic(modeString.substr(0, 3), mode.EffectiveUserIdentity);
      mode.Group = ParseSymbolic(modeString.substr(3, 3), mode.EffectiveGroupIdentity);
      mode.Other = ParseSymbolic(modeString.substr(6, 3), mode.StickyBit);
      return mode;
    }

    ShareFileHandleAccessRights::ShareFileHandleAccessRights(const std::string& value)
    {
      if (!value.empty())
      {
        m_value.insert(value);
      }
    }
    ShareFileHandleAccessRights ShareFileHandleAccessRights::operator|(
        const ShareFileHandleAccessRights& other) const
    {
      ShareFileHandleAccessRights ret;
      std::set_union(
          m_value.begin(),
          m_value.end(),
          other.m_value.begin(),
          other.m_value.end(),
          std::inserter(ret.m_value, ret.m_value.begin()));
      return ret;
    }
    ShareFileHandleAccessRights ShareFileHandleAccessRights::operator&(
        const ShareFileHandleAccessRights& other) const
    {
      ShareFileHandleAccessRights ret;
      std::set_intersection(
          m_value.begin(),
          m_value.end(),
          other.m_value.begin(),
          other.m_value.end(),
          std::inserter(ret.m_value, ret.m_value.begin()));
      return ret;
    }
    ShareFileHandleAccessRights ShareFileHandleAccessRights::operator^(
        const ShareFileHandleAccessRights& other) const
    {
      ShareFileHandleAccessRights ret;
      std::set_symmetric_difference(
          m_value.begin(),
          m_value.end(),
          other.m_value.begin(),
          other.m_value.end(),
          std::inserter(ret.m_value, ret.m_value.begin()));
      return ret;
    }
    const ShareFileHandleAccessRights ShareFileHandleAccessRights::Read("Read");
    const ShareFileHandleAccessRights ShareFileHandleAccessRights::Write("Write");
    const ShareFileHandleAccessRights ShareFileHandleAccessRights::Delete("Delete");
  } // namespace Models

  std::unique_ptr<Azure::Core::Http::RawResponse> StartFileCopyOperation::PollInternal(
      const Azure::Core::Context&)
  {

    auto response = m_fileClient->GetProperties();
    if (!response.Value.CopyStatus.HasValue())
    {
      m_status = Azure::Core::OperationStatus::Failed;
    }
    else if (response.Value.CopyStatus.Value() == Models::CopyStatus::Pending)
    {
      m_status = Azure::Core::OperationStatus::Running;
    }
    else if (response.Value.CopyStatus.Value() == Models::CopyStatus::Success)
    {
      m_status = Azure::Core::OperationStatus::Succeeded;
    }
    else
    {
      m_status = Azure::Core::OperationStatus::Failed;
    }
    m_pollResult = response.Value;
    return std::move(response.RawResponse);
  }

  Azure::Response<Models::FileProperties> StartFileCopyOperation::PollUntilDoneInternal(
      std::chrono::milliseconds period,
      Azure::Core::Context& context)
  {
    while (true)
    {
      auto rawResponse = Poll(context);

      if (m_status == Azure::Core::OperationStatus::Succeeded)
      {
        return Azure::Response<Models::FileProperties>(
            m_pollResult, std::make_unique<Azure::Core::Http::RawResponse>(rawResponse));
      }
      else if (m_status == Azure::Core::OperationStatus::Failed)
      {
        throw Azure::Core::RequestFailedException("Operation failed.");
      }
      else if (m_status == Azure::Core::OperationStatus::Cancelled)
      {
        throw Azure::Core::RequestFailedException("Operation was cancelled.");
      }

      std::this_thread::sleep_for(period);
    }
  }

  void ListSharesPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    m_operationOptions.ContinuationToken = NextPageToken;
    *this = m_shareServiceClient->ListShares(m_operationOptions, context);
  }

  void ListFilesAndDirectoriesPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    m_operationOptions.ContinuationToken = NextPageToken;
    *this = m_shareDirectoryClient->ListFilesAndDirectories(m_operationOptions, context);
  }

  void ListFileHandlesPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    m_operationOptions.ContinuationToken = NextPageToken;
    *this = m_shareFileClient->ListHandles(m_operationOptions, context);
  }

  void ForceCloseAllFileHandlesPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    m_operationOptions.ContinuationToken = NextPageToken;
    *this = m_shareFileClient->ForceCloseAllHandles(m_operationOptions, context);
  }

  void ListDirectoryHandlesPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    m_operationOptions.ContinuationToken = NextPageToken;
    *this = m_shareDirectoryClient->ListHandles(m_operationOptions, context);
  }

  void ForceCloseAllDirectoryHandlesPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    m_operationOptions.ContinuationToken = NextPageToken;
    *this = m_shareDirectoryClient->ForceCloseAllHandles(m_operationOptions, context);
  }

}}}} // namespace Azure::Storage::Files::Shares

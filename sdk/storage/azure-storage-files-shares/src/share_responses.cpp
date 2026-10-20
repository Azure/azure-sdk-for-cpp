// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/storage/files/shares/share_responses.hpp"

#include "azure/storage/files/shares/share_directory_client.hpp"
#include "azure/storage/files/shares/share_file_client.hpp"

#include <algorithm>
#include <iterator>
#include <thread>

namespace Azure { namespace Storage { namespace Files { namespace Shares {
  namespace Models {
    const ShareTokenIntent ShareTokenIntent::Backup("backup");
    const AccessTier AccessTier::TransactionOptimized("TransactionOptimized");
    const AccessTier AccessTier::Hot("Hot");
    const AccessTier AccessTier::Cool("Cool");
    const AccessTier AccessTier::Premium("Premium");
    const LeaseStatus LeaseStatus::Locked("locked");
    const LeaseStatus LeaseStatus::Unlocked("unlocked");
    const LeaseState LeaseState::Available("available");
    const LeaseState LeaseState::Leased("leased");
    const LeaseState LeaseState::Expired("expired");
    const LeaseState LeaseState::Breaking("breaking");
    const LeaseState LeaseState::Broken("broken");
    const LeaseDurationType LeaseDurationType::Infinite("infinite");
    const LeaseDurationType LeaseDurationType::Fixed("fixed");
    const ShareProtocols ShareProtocols::Smb("SMB");
    const ShareProtocols ShareProtocols::Nfs("NFS");
    const ShareRootSquash ShareRootSquash::NoRootSquash("NoRootSquash");
    const ShareRootSquash ShareRootSquash::RootSquash("RootSquash");
    const ShareRootSquash ShareRootSquash::AllSquash("AllSquash");
    const DeleteSnapshotsOption DeleteSnapshotsOption::Include("include");
    const FilePermissionFormat FilePermissionFormat::Sddl("sddl");
    const FilePermissionFormat FilePermissionFormat::Binary("binary");
    FileAttributes::FileAttributes(const std::string& value)
    {
      const std::string delimiter = " | ";
      size_t cur = 0;
      while (cur != std::string::npos)
      {
        auto delimiter_pos = value.find(delimiter, cur);
        std::string v;
        if (delimiter_pos == std::string::npos)
        {
          v = value.substr(cur);
          cur = delimiter_pos;
        }
        else
        {
          v = value.substr(cur, delimiter_pos - cur);
          cur = delimiter_pos + delimiter.size();
        }
        if (!v.empty())
        {
          m_value.push_back(std::move(v));
        }
      }
      std::sort(m_value.begin(), m_value.end());
    }
    FileAttributes FileAttributes::operator|(const FileAttributes& other) const
    {
      FileAttributes ret;
      std::set_union(
          m_value.begin(),
          m_value.end(),
          other.m_value.begin(),
          other.m_value.end(),
          std::back_inserter(ret.m_value));
      return ret;
    }
    FileAttributes FileAttributes::operator&(const FileAttributes& other) const
    {
      FileAttributes ret;
      std::set_intersection(
          m_value.begin(),
          m_value.end(),
          other.m_value.begin(),
          other.m_value.end(),
          std::back_inserter(ret.m_value));
      return ret;
    }
    FileAttributes FileAttributes::operator^(const FileAttributes& other) const
    {
      FileAttributes ret;
      std::set_symmetric_difference(
          m_value.begin(),
          m_value.end(),
          other.m_value.begin(),
          other.m_value.end(),
          std::back_inserter(ret.m_value));
      return ret;
    }
    std::string FileAttributes::ToString() const
    {
      std::string ret;
      for (const auto& v : m_value)
      {
        ret += (ret.empty() ? "" : " | ") + v;
      }
      return ret;
    }
    const FileAttributes FileAttributes::ReadOnly("ReadOnly");
    const FileAttributes FileAttributes::Hidden("Hidden");
    const FileAttributes FileAttributes::System("System");
    const FileAttributes FileAttributes::None("None");
    const FileAttributes FileAttributes::Directory("Directory");
    const FileAttributes FileAttributes::Archive("Archive");
    const FileAttributes FileAttributes::Temporary("Temporary");
    const FileAttributes FileAttributes::Offline("Offline");
    const FileAttributes FileAttributes::NotContentIndexed("NotContentIndexed");
    const FileAttributes FileAttributes::NoScrubData("NoScrubData");
    const FilePropertySemantics FilePropertySemantics::New("New");
    const FilePropertySemantics FilePropertySemantics::Restore("Restore");
    const NfsFileType NfsFileType::Regular("Regular");
    const NfsFileType NfsFileType::Directory("Directory");
    const NfsFileType NfsFileType::SymLink("SymLink");
    namespace _detail {
      const AccessRight AccessRight::Read("Read");
      const AccessRight AccessRight::Write("Write");
      const AccessRight AccessRight::Delete("Delete");
    } // namespace _detail
    const CopyStatus CopyStatus::Pending("pending");
    const CopyStatus CopyStatus::Success("success");
    const CopyStatus CopyStatus::Aborted("aborted");
    const CopyStatus CopyStatus::Failed("failed");
    const FileLastWrittenMode FileLastWrittenMode::Now("now");
    const FileLastWrittenMode FileLastWrittenMode::Preserve("preserve");
    const PermissionCopyMode PermissionCopyMode::Source("source");
    const PermissionCopyMode PermissionCopyMode::Override("override");
    const ModeCopyMode ModeCopyMode::Source("source");
    const ModeCopyMode ModeCopyMode::Override("override");
    const OwnerCopyMode OwnerCopyMode::Source("source");
    const OwnerCopyMode OwnerCopyMode::Override("override");

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

  void GetFileRangeListPagedResponse::OnNextPage(const Azure::Core::Context& context)
  {
    m_operationOptions.ContinuationToken = NextPageToken;
    if (m_previousShareSnapshot.HasValue())
    {
      *this = m_shareFileClient->GetAllRangeListDiff(
          m_previousShareSnapshot.Value(), m_operationOptions, context);
    }
    else
    {
      *this = m_shareFileClient->GetAllRangeList(m_operationOptions, context);
    }
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

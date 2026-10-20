// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <azure/storage/common/internal/storage_pipeline.hpp>
#include <azure/storage/files/datalake/rest_client.hpp>

namespace Azure { namespace Storage { namespace Files { namespace DataLake { namespace _detail {

  namespace PublicModels = Azure::Storage::Files::DataLake::Models;

  class DataLakeProtocolClient final {
  private:
    Core::Url m_url;
    std::shared_ptr<Core::Http::_internal::HttpPipeline> m_pipeline;

  public:
    DataLakeProtocolClient(
        Core::Url url,
        std::shared_ptr<Core::Http::_internal::HttpPipeline> pipeline)
        : m_url(std::move(url)), m_pipeline(std::move(pipeline))
    {
    }

    Response<PublicModels::_detail::PathList> ListPaths(
        FileSystemClient::ListFileSystemPathsOptions const& options,
        Core::Context const& context) const;
    Response<PublicModels::CreatePathResult> Create(
        PathClient::CreatePathOptions const& options,
        Core::Context const& context) const;
    Response<PublicModels::DeletePathResult> Delete(
        PathClient::DeletePathOptions const& options,
        Core::Context const& context) const;
    Response<PublicModels::SetPathAccessControlListResult> SetAccessControlList(
        PathClient::SetPathAccessControlListOptions const& options,
        Core::Context const& context) const;
    Response<PublicModels::_detail::SetAccessControlListRecursiveResult>
    SetAccessControlListRecursive(
        PathClient::SetPathAccessControlListRecursiveOptions const& options,
        Core::Context const& context) const;
    Response<PublicModels::_detail::UndeletePathResult> Undelete(
        PathClient::UndeletePathOptions const& options,
        Core::Context const& context) const;
    Response<PublicModels::_detail::GetPathAccessControlListResult> GetAccessControlList(
        PathClient::GetPathAccessControlListOptions const& options,
        Core::Context const& context) const;
    Response<PublicModels::PathSystemProperties> GetSystemProperties(
        PathClient::GetPathSystemPropertiesOptions const& options,
        Core::Context const& context) const;
    Response<PublicModels::FlushFileResult> Flush(
        FileClient::FlushFileOptions const& options,
        Core::Context const& context) const;
    Response<PublicModels::AppendFileResult> Append(
        Core::IO::BodyStream& content,
        FileClient::AppendFileOptions const& options,
        Core::Context const& context) const;
  };

  class DataLakeProtocolPolicy final : public Core::Http::Policies::HttpPolicy {
  public:
    std::unique_ptr<Core::Http::Policies::HttpPolicy> Clone() const override
    {
      return std::make_unique<DataLakeProtocolPolicy>();
    }

    std::unique_ptr<Core::Http::RawResponse> Send(
        Core::Http::Request& request,
        Core::Http::Policies::NextHttpPolicy nextPolicy,
        Core::Context const& context) const override
    {
      // Preserve legacy wire headers before user policies run.
      request.RemoveHeader("Accept");
      auto const contentType = request.GetHeader("Content-Type");
      if (contentType.HasValue() && contentType.Value() == "application/octet-stream")
      {
        request.RemoveHeader("Content-Type");
      }
      return nextPolicy.Send(request, context);
    }
  };

  inline std::vector<std::unique_ptr<Core::Http::Policies::HttpPolicy>>
  BuildDataLakePipelinePolicies(
      Core::_internal::ClientOptions const& options,
      _internal::BuildStoragePipelineOptions pipelineOptions)
  {
    auto policies = _internal::BuildHttpPipelinePolicies(options, std::move(pipelineOptions));
    policies.insert(policies.begin(), std::make_unique<DataLakeProtocolPolicy>());
    return policies;
  }

}}}}} // namespace Azure::Storage::Files::DataLake::_detail

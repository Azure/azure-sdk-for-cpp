// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <string>

#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/response.hpp>
#include <azure/storage/common/storage_credential.hpp>

#include "azure/storage/files/shares/share_options.hpp"
#include "azure/storage/files/shares/share_responses.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  class ShareClient;

  class ShareServiceClient final {
  public:
    /**
     * @brief Create A ShareServiceClient from connection string to manage the service related
     * attributes.
     * @param connectionString Azure Storage connection string.
     * @param options Optional parameters used to initialize the client.
     * @return ShareServiceClient
     */
    static ShareServiceClient CreateFromConnectionString(
        const std::string& connectionString,
        const ShareClientOptions& options = ShareClientOptions());

    /**
     * @brief Initialize a new instance of ShareServiceClient using shared key authentication.
     * @param serviceUrl The service URL this client's request targets.
     * @param credential The shared key credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit ShareServiceClient(
        const std::string& serviceUrl,
        std::shared_ptr<StorageSharedKeyCredential> credential,
        const ShareClientOptions& options = ShareClientOptions());

    /**
     * @brief Initialize a new instance of ShareServiceClient using shared key authentication.
     * @param serviceUrl The service URL this client's request targets.
     * @param credential The token credential used to sign requests.
     * @param options Optional parameters used to initialize the client.
     */
    explicit ShareServiceClient(
        const std::string& serviceUrl,
        std::shared_ptr<Core::Credentials::TokenCredential> credential,
        const ShareClientOptions& options = ShareClientOptions());

    /**
     * @brief Initialize a new instance of ShareServiceClient using anonymous access or shared
     * access signature.
     * @param serviceUrl The service URL this client's request targets.
     * @param options Optional parameters used to initialize the client.
     */
    explicit ShareServiceClient(
        const std::string& serviceUrl,
        const ShareClientOptions& options = ShareClientOptions());

    /**
     * @brief Create a ShareClient from current ShareServiceClient
     * @param shareName The name of the file share.
     * @return ShareClient A share client that can be used to manage a share resource.
     */
    ShareClient GetShareClient(const std::string& shareName) const;

    /**
     * @brief Gets the file share service's primary URL endpoint.
     *
     * @return The file share service's primary URL endpoint.
     */
    std::string GetUrl() const { return m_serviceUrl.GetAbsoluteUrl(); }

    /**
     * @brief Returns a paginated collection of the shares in the storage account. Enumerating the
     * shares may make multiple requests to the service while fetching all the values.
     * @param options Optional parameters to list the shares.
     * @param context Context for cancelling long running operations.
     * @return ListSharesPagedResponse describing the shares in this storage account.
     */
    ListSharesPagedResponse ListShares(
        const ListSharesOptions& options = ListSharesOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Set the service's properties.
     * @param properties The properties of the service that is to be set.
     * @param options Optional parameters to set the properties of the service.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::SetServicePropertiesResult> The information returned
     * when setting the service properties.
     */
    Azure::Response<Models::SetServicePropertiesResult> SetProperties(
        Models::ShareServiceProperties properties,
        const SetServicePropertiesOptions& options = SetServicePropertiesOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Get the service's properties.
     * @param options Optional parameters to get the properties of the service.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::FileServiceProperties> The properties of the service.
     */
    Azure::Response<Models::ShareServiceProperties> GetProperties(
        const GetServicePropertiesOptions& options = GetServicePropertiesOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

  private:
    Azure::Core::Url m_serviceUrl;
    std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> m_pipeline;
    Nullable<bool> m_allowTrailingDot;
    Nullable<bool> m_allowSourceTrailingDot;
    Nullable<Models::ShareTokenIntent> m_shareTokenIntent;
  };
}}}} // namespace Azure::Storage::Files::Shares

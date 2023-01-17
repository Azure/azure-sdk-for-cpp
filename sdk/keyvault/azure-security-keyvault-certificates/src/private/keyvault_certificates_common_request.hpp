// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Provides a wrapper class for the Azure Core Pipeline for all Key Vault services where
 * common functionality is set up.
 *
 */

#pragma once

#include <azure/core/context.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_serializable.hpp>
#include <azure/core/response.hpp>

#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Security { namespace KeyVault { namespace _detail {

  constexpr static const char ContentHeaderName[] = "content-type";
  constexpr static const char ApplicationJsonValue[] = "application/json";
  constexpr static const char ApiVersionQueryParamName[] = "api-version";

  struct KeyVaultCertificatesCommonRequest final
  {
    static Azure::Core::Http::Request CreateRequest(
        Azure::Core::Url url,
        std::string const& apiVersion,
        Azure::Core::Http::HttpMethod method,
        std::vector<std::string> const& path,
        Azure::Core::IO::BodyStream* content);

    static std::unique_ptr<Azure::Core::Http::RawResponse> SendRequest(
        Azure::Core::Http::_internal::HttpPipeline const& pipeline,
        Azure::Core::Http::Request& request,
        Azure::Core::Context const& context);
  };

}}}} // namespace Azure::Security::KeyVault::_detail

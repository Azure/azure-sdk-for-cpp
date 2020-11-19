// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/http/http.hpp>

#include "azure/core/test/playback_http_client.hpp"

#include <string>

std::unique_ptr<Azure::Core::Http::RawResponse> Azure::Core::Test::PlaybackClient::Send(
    Azure::Core::Context const& context,
    Azure::Core::Http::Request& request)
{
  (void)(context);
  (void)(request);
  (void)(m_recordedData);

  throw;
}

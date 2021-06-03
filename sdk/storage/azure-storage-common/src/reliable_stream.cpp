// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/common/internal/reliable_stream.hpp"

#include <azure/core/http/http.hpp>

using Azure::Core::Context;
using Azure::Core::IO::BodyStream;

namespace Azure { namespace Storage { namespace _internal {

  size_t ReliableStream::OnRead(uint8_t* buffer, size_t count, Context const& context)
  {
    (void)context;
    for (int64_t intent = 1;; intent++)
    {
      // check if we need to get inner stream
      if (this->m_inner == nullptr)
      {
        // Get a bodyStream that starts from last known offset
        // if this fails, throw bubbles up
        // As m_inner is unique_pr, it will be destructed on reassignment, cleaning up network
        // session.
        this->m_inner = this->m_streamReconnector(this->m_retryOffset, context);
      }
      try
      {
        auto const readBytes = this->m_inner->Read(buffer, count, context);
        // update offset
        this->m_retryOffset += readBytes;
        return readBytes;
      }
      catch (std::runtime_error const& e)
      {
        // forget about the inner stream. We will need to request a new one
        // As m_inner is unique_pr, it will be destructed on reassignment (cleaning up network
        // session).
        this->m_inner.reset();
        (void)e; // todo: maybe log the exception in the future?
        if (intent == this->m_options.MaxRetryRequests)
        {
          // max retry. End loop. Rethrow
          throw;
        }
      }
    }
  }
}}} // namespace Azure::Storage::_internal

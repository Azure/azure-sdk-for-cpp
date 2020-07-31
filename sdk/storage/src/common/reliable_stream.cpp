// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "common/reliable_stream.hpp"
#include "http/http.hpp"

using Azure::Core::Context;
using Azure::Core::Http::BodyStream;

namespace Azure { namespace Storage {

  int64_t ReliableStream::Read(Context& context, uint8_t* buffer, int64_t count)
  {
    if (this->m_options.DoInjectError)
    {
      throw std::runtime_error("Injected error");
    }

    for (int64_t intent = 1;; intent++)
    {
      // check if we need to get inner stream
      if (this->m_inner == nullptr)
      {
        // Get a a bodyStream that starts from last known offset
        // if this fails, throw bubbles up
        // As m_inner is unique_pr, it will be destructed on reassignment, cleaning up network
        // session.
        this->m_inner = this->m_httpGetter(context, this->m_retryInfo);
      }
      try
      {
        auto const readBytes = this->m_inner->Read(context, buffer, count);
        // update offset
        this->m_retryInfo.Offset += readBytes;
        return readBytes;
      }
      catch (std::runtime_error const& e)
      {
        // forget about the inner stream. We will need to request a new one
        // As m_inner is unique_pr, it will be destructed on reassignment (cleaning up network
        // session).
        this->m_inner.release();
        if (intent == this->m_options.MaxRetryRequests)
        {
          // max retry. End loop
          throw e;
        }
      }
    }
  }
}} // namespace Azure::Storage

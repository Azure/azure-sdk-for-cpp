// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "common/retriable_stream.hpp"

using Azure::Core::Context;
using Azure::Core::Http::BodyStream;

namespace Azure { namespace Storage {

  int64_t RetriableStream::Read(Context& context, uint8_t* buffer, int64_t count)
  {
    if (this->m_options.DoInjectError)
    {
      throw std::runtime_error("do we say something here for testing?");
    }

    char const* error;
    for (int64_t intent = 0; intent < this->m_options.MaxRetryRequests; intent++)
    {
      // check if we need to get inner stream
      if (this->m_inner == nullptr)
      {
        // Get a new inner bodyStream that start from last offset and size m_count (remaining)
        // if this fails, throw should bubble up
        this->m_inner = this->m_retryFn(context, this->m_retryInfo);
      }
      try
      {
        auto const readBytes = this->m_inner->Read(context, buffer, count);
        // update offset and content
        this->m_retryInfo.Offset += readBytes;
        this->m_retryInfo.Count -= readBytes;
        return readBytes;
      }
      catch (std::exception const& e)
      {
        error = e.what();
        // forget about the inner stream. We will need to request a new one
        this->m_inner.release();
      }
    }
    // retries exhausted, throw?
    throw std::runtime_error(error);
  }
}} // namespace Azure::Storage
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "context.hpp"
#include "http/body_stream.hpp"

namespace Azure { namespace Storage {
  typedef std::unique_ptr<Azure::Core::Http::BodyStream> (
      *retryFunction)(Azure::Core::Context&, int64_t, int64_t);

  class RetriableStream : public Azure::Core::Http::BodyStream {
  private:
    std::unique_ptr<Azure::Core::Http::BodyStream> m_inner;
    int64_t m_maxRetries = 0;
    retryFunction m_retryFn;
    int64_t m_offset = 0;
    int64_t m_count = 0;

  public:
    // TODO: Do we need to keep original length forever?
    RetriableStream(
        std::unique_ptr<Azure::Core::Http::BodyStream> inner,
        int64_t maxRetries,
        retryFunction retryFn)
        : m_inner(std::move(inner)), m_maxRetries(maxRetries), m_retryFn(retryFn),
          m_count(inner->Length())
    {
    }

    int64_t Length() const override { return this->m_inner->Length(); }
    void Rewind() override
    {
      this->m_inner->Rewind();
      this->m_offset = 0;
    }
    int64_t Read(Azure::Core::Context& context, uint8_t* buffer, int64_t count) override;
  };

}} // namespace Azure::Storage
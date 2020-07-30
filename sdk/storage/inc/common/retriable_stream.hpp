// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "context.hpp"
#include "http/body_stream.hpp"

namespace Azure { namespace Storage {

  // options used by the callback that will get a new bodyStream starting from last offset
  struct RetryInfo
  {
    int64_t Offset;
    int64_t Count;
    std::string ETag;
  };

  struct RetriableStreamOptions
  {
    // configures the maximun retries to be done.
    int64_t MaxRetryRequests;
    // Use for testing purposes only.
    bool DoInjectError;
  };

  // expected function signature for getting a new bodyStream upon failure.
  // RetryInfo will be const& to avoid changes on the callback (read-only)
  typedef std::unique_ptr<Azure::Core::Http::BodyStream> (
      *retryFunction)(Azure::Core::Context&, RetryInfo const&);

  class RetriableStream : public Azure::Core::Http::BodyStream {
  private:
    // initial bodyStream
    std::unique_ptr<Azure::Core::Http::BodyStream> m_inner;
    // Configuration for the re-triable stream
    RetriableStreamOptions const m_options;
    // callback to get a new bodyStream in case Reading fails
    retryFunction m_retryFn;
    // Options to use when getting a new bodyStream
    RetryInfo m_retryInfo;

  public:
    // TODO: Do we need to keep original length forever?
    RetriableStream(
        std::unique_ptr<Azure::Core::Http::BodyStream> inner,
        RetriableStreamOptions const options,
        retryFunction retryFn,
        RetryInfo retryInfo)
        : m_inner(std::move(inner)), m_options(std::move(options)), m_retryFn(retryFn),
          m_retryInfo(std::move(retryInfo))
    {
    }

    int64_t Length() const override { return this->m_inner->Length(); }
    void Rewind() override
    {
      this->m_inner->Rewind();
      this->m_retryInfo.Offset = 0;
    }
    int64_t Read(Azure::Core::Context& context, uint8_t* buffer, int64_t count) override;
  };

}} // namespace Azure::Storage
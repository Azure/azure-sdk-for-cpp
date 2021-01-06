// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <algorithm>
#include <functional>
#include <memory>

#include <azure/core/context.hpp>
#include <azure/core/http/body_stream.hpp>

namespace Azure { namespace Storage {

  // options used by the fm callback that will get a bodyStream starting from last offset
  struct HttpGetterInfo
  {
    int64_t Offset = 0;
  };

  // Defines a fn signature to be use to get a bodyStream from a specific offset.
  typedef std::function<std::unique_ptr<Azure::Core::Http::BodyStream>(
      Azure::Core::Context const&,
      HttpGetterInfo const&)>
      HTTPGetter;

  // Options used by reliable stream
  struct ReliableStreamOptions
  {
    // configures the maximun retries to be done.
    int64_t MaxRetryRequests;
  };

  /**
   * @brief Decorates a body stream by providing reliability while readind from it.
   * ReliableStream uses an HTTPGetter callback (provided on constructor) to get a bodyStream
   * starting on last known offset to resume a fail Read() operation.
   *
   * @remark An HTTPGetter callback is expected to verify the initial `eTag` from first http request
   * to ensure read operation will continue on the same content.
   *
   * @remark An HTTPGetter callback is expected to calculate and set the range header based on the
   * offset provided by the ReliableStream.
   *
   */
  class ReliableStream : public Azure::Core::Http::BodyStream {
  private:
    // initial bodyStream.
    std::unique_ptr<Azure::Core::Http::BodyStream> m_inner;
    // Configuration for the re-triable stream
    ReliableStreamOptions const m_options;
    // callback to get a bodyStream in case Read operation fails
    HTTPGetter m_httpGetter;
    // Options to use when getting a new bodyStream like current offset
    HttpGetterInfo m_retryInfo;

    int64_t OnRead(Azure::Core::Context const& context, uint8_t* buffer, int64_t count) override;

  public:
    explicit ReliableStream(
        std::unique_ptr<Azure::Core::Http::BodyStream> inner,
        ReliableStreamOptions const options,
        HTTPGetter httpGetter)
        : m_inner(std::move(inner)), m_options(options), m_httpGetter(std::move(httpGetter))
    {
    }

    int64_t Length() const override { return this->m_inner->Length(); }
    void Rewind() override
    {
      // Rewind directly from a transportAdapter body stream (like libcurl) would throw
      this->m_inner->Rewind();
      this->m_retryInfo.Offset = 0;
    }
  };

}} // namespace Azure::Storage

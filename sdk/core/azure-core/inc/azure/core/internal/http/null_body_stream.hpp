// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief A null body stream for Http requests without a payload.
 */

#pragma once

#include "azure/core/http/body_stream.hpp"

namespace Azure { namespace Core { namespace Internal { namespace Http {

  /**
   * @brief Empty #Azure::Core::Http::BodyStream.
   * @remark Used for requests with no body.
   */
  class NullBodyStream : public Azure::Core::Http::BodyStream {
  private:
    int64_t OnRead(Azure::Core::Context const& context, uint8_t* buffer, int64_t count) override
    {
      (void)context;
      (void)buffer;
      (void)count;
      return 0;
    };

  public:
    /// Constructor.
    explicit NullBodyStream() {}

    int64_t Length() const override { return 0; }

    void Rewind() override {}

    /**
     * @brief Gets a singleton instance of a #Azure::Core::Http::NullBodyStream.
     */
    static NullBodyStream* GetNullBodyStream()
    {
      static NullBodyStream nullBodyStream;
      return &nullBodyStream;
    }
  };

}}}} // namespace Azure::Core::Internal::Http

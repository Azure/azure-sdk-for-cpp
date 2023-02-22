// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Defines the network models for recording HTTP requests from the network.
 *
 */

#pragma once

namespace Azure { namespace Core { namespace Test {

  /**
   * @brief The mode how the tests cases wil behave.
   *
   */
  enum class TestMode
  {
    PLAYBACK,
    RECORD,
    LIVE,
  };

}}} // namespace Azure::Core::Test

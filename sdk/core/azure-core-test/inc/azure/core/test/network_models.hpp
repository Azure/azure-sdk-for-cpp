// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Defines the network models for recording HTTP requests from the network.
 *
 */

#pragma once

#include <list>
#include <map>
#include <string>

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

  /**
   * @brief Keeps track of network call records from each unit test session.
   *
   */
  struct NetworkCallRecord
  {
    std::string Method;
    std::string Url;
    std::map<std::string, std::string> Headers;
    std::map<std::string, std::string> Response;
  };

  /**
   * @brief Keeps track of the network calls and variable names that were made in a test
   * session.
   *
   */
  class RecordedData {
  public:
    std::list<NetworkCallRecord> NetworkCallRecords;
    std::list<std::string> Variables;
  };

}}} // namespace Azure::Core::Test

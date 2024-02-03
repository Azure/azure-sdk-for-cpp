// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Test the JSON performance.
 *
 */

#pragma once

#include "../../../core/perf/inc/azure/perf.hpp"

#include <azure/core.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_optional.hpp>

#include <string>

using namespace Azure::Core::Json::_internal;

#include <memory>

namespace Azure { namespace Core { namespace Test {

  struct JsonTestObject
  {
    // stack
    bool Boolean;
    int8_t Int8;
    int16_t Int16;
    int32_t Int32;
    int64_t Int64;
    uint8_t Uint8;
    uint16_t Uint16;
    uint32_t Uint32;
    uint64_t Uint64;
    float Float;
    double Double;
    std::string String;

    // nullables
    Azure::Nullable<bool> NullableBoolean;
    Azure::Nullable<int8_t> NullableInt8;
    Azure::Nullable<int16_t> NullableInt16;
    Azure::Nullable<int32_t> NullableInt32;
    Azure::Nullable<int64_t> NullableInt64;
    Azure::Nullable<uint8_t> NullableUint8;
    Azure::Nullable<uint16_t> NullableUint16;
    Azure::Nullable<uint32_t> NullableUint32;
    Azure::Nullable<uint64_t> NullableUint64;
    Azure::Nullable<float> NullableFloat;
    Azure::Nullable<double> NullableDouble;
    Azure::Nullable<std::string> NullableString;

    // vectors
    std::vector<bool> Booleans;
    std::vector<int8_t> Int8s;
    std::vector<int16_t> Int16s;
    std::vector<int32_t> Int32s;
    std::vector<int64_t> Int64s;
    std::vector<uint8_t> Uint8s;
    std::vector<uint16_t> Uint16s;
    std::vector<uint32_t> Uint32s;
    std::vector<uint64_t> Uint64s;
    std::vector<float> Floats;
    std::vector<double> Doubles;
    std::vector<std::string> Strings;

    // Map
    std::map<std::string, std::string> Map;

    bool operator==(const JsonTestObject& other) const
    {
      return Boolean == other.Boolean && Int8 == other.Int8 && Int16 == other.Int16
          && Int32 == other.Int32 && Int64 == other.Int64 && Uint8 == other.Uint8
          && Uint16 == other.Uint16 && Uint32 == other.Uint32 && Uint64 == other.Uint64
          && Float == other.Float && Double == other.Double && String == other.String
          && Booleans == other.Booleans && Int8s == other.Int8s && Int16s == other.Int16s
          && Int32s == other.Int32s && Int64s == other.Int64s && Uint8s == other.Uint8s
          && Uint16s == other.Uint16s && Uint32s == other.Uint32s && Uint64s == other.Uint64s
          && Floats == other.Floats && Doubles == other.Doubles && Strings == other.Strings
          && Map == other.Map;
    }

    std::string Serialize() const
    {
      Azure::Core::Json::_internal::json j;

      j["Boolean"] = Boolean;
      j["Int8"] = Int8;
      j["Int16"] = Int16;
      j["Int32"] = Int32;
      j["Int64"] = Int64;
      j["Uint8"] = Uint8;
      j["Uint16"] = Uint16;
      j["Uint32"] = Uint32;
      j["Uint64"] = Uint64;
      j["Float"] = Float;
      j["Double"] = Double;
      j["String"] = String;

      JsonOptional::SetFromNullable<bool>(NullableBoolean, j, "NullableBoolean");
      JsonOptional::SetFromNullable<int8_t>(NullableInt8, j, "NullableInt8");
      JsonOptional::SetFromNullable<int16_t>(NullableInt16, j, "NullableInt16");
      JsonOptional::SetFromNullable<int32_t>(NullableInt32, j, "NullableInt32");
      JsonOptional::SetFromNullable<int64_t>(NullableInt64, j, "NullableInt64");
      JsonOptional::SetFromNullable<uint8_t>(NullableUint8, j, "NullableUint8");
      JsonOptional::SetFromNullable<uint16_t>(NullableUint16, j, "NullableUint16");
      JsonOptional::SetFromNullable<uint32_t>(NullableUint32, j, "NullableUint32");
      JsonOptional::SetFromNullable<uint64_t>(NullableUint64, j, "NullableUint64");
      JsonOptional::SetFromNullable<float>(NullableFloat, j, "NullableFloat");
      JsonOptional::SetFromNullable<double>(NullableDouble, j, "NullableDouble");
      JsonOptional::SetFromNullable<std::string>(NullableString, j, "NullableString");

      j["Booleans"] = Booleans;
      j["Int8s"] = Int8s;
      j["Int16s"] = Int16s;
      j["Int32s"] = Int32s;
      j["Int64s"] = Int64s;
      j["Uint8s"] = Uint8s;
      j["Uint16s"] = Uint16s;
      j["Uint32s"] = Uint32s;
      j["Uint64s"] = Uint64s;
      j["Floats"] = Floats;
      j["Doubles"] = Doubles;
      j["Strings"] = Strings;

      for (auto const& pair : Map)
      {
        j["Map"][pair.first] = pair.second;
      };

      return j.dump();
    }

    void Deserialize(std::string const& json)
    {
      Azure::Core::Json::_internal::json j = Azure::Core::Json::_internal::json::parse(json);

      Boolean = j["Boolean"].get<bool>();
      Int8 = j["Int8"].get<int8_t>();
      Int16 = j["Int16"].get<int16_t>();
      Int32 = j["Int32"].get<int32_t>();
      Int64 = j["Int64"].get<int64_t>();
      Uint8 = j["Uint8"].get<uint8_t>();
      Uint16 = j["Uint16"].get<uint16_t>();
      Uint32 = j["Uint32"].get<uint32_t>();
      Uint64 = j["Uint64"].get<uint64_t>();
      Float = j["Float"].get<float>();
      Double = j["Double"].get<double>();
      String = j["String"].get<std::string>();

      JsonOptional::SetIfExists<bool>(NullableBoolean, j, "NullableBoolean");
      JsonOptional::SetIfExists<int8_t>(NullableInt8, j, "NullableInt8");
      JsonOptional::SetIfExists<int16_t>(NullableInt16, j, "NullableInt16");
      JsonOptional::SetIfExists<int32_t>(NullableInt32, j, "NullableInt32");
      JsonOptional::SetIfExists<int64_t>(NullableInt64, j, "NullableInt64");
      JsonOptional::SetIfExists<uint8_t>(NullableUint8, j, "NullableUint8");
      JsonOptional::SetIfExists<uint16_t>(NullableUint16, j, "NullableUint16");
      JsonOptional::SetIfExists<uint32_t>(NullableUint32, j, "NullableUint32");
      JsonOptional::SetIfExists<uint64_t>(NullableUint64, j, "NullableUint64");
      JsonOptional::SetIfExists<float>(NullableFloat, j, "NullableFloat");
      JsonOptional::SetIfExists<double>(NullableDouble, j, "NullableDouble");
      JsonOptional::SetIfExists<std::string>(NullableString, j, "NullableString");

      Booleans = j["Booleans"].get<std::vector<bool>>();
      Int8s = j["Int8s"].get<std::vector<int8_t>>();
      Int16s = j["Int16s"].get<std::vector<int16_t>>();
      Int32s = j["Int32s"].get<std::vector<int32_t>>();
      Int64s = j["Int64s"].get<std::vector<int64_t>>();
      Uint8s = j["Uint8s"].get<std::vector<uint8_t>>();
      Uint16s = j["Uint16s"].get<std::vector<uint16_t>>();
      Uint32s = j["Uint32s"].get<std::vector<uint32_t>>();
      Uint64s = j["Uint64s"].get<std::vector<uint64_t>>();
      Floats = j["Floats"].get<std::vector<float>>();
      Doubles = j["Doubles"].get<std::vector<double>>();
      Strings = j["Strings"].get<std::vector<std::string>>();

      for (auto const& pair : j["Map"].items())
      {
        Map[pair.key()] = pair.value().get<std::string>();
      }
    }

    JsonTestObject() = default;
    JsonTestObject(size_t const& vectorSize)
    {
      Boolean = true;
      Int8 = (int8_t)1;
      Int16 = (int16_t)2;
      Int32 = (int32_t)3;
      Int64 = (int64_t)4;
      Uint8 = (uint8_t)5;
      Uint16 = (uint16_t)6;
      Uint32 = (uint32_t)7;
      Uint64 = (uint64_t)8;
      Float = (float)9.0;
      Double = (double)10.0;
      String = "string";

      NullableBoolean = true;
      NullableInt8 = (int8_t)1;
      NullableInt16 = (int16_t)2;
      NullableInt32 = (int32_t)3;
      NullableInt64 = (int64_t)4;
      NullableUint8 = (uint8_t)5;
      NullableUint16 = (uint16_t)6;
      NullableUint32 = (uint32_t)7;
      NullableUint64 = (uint64_t)8;
      NullableFloat = (float)9.0;
      NullableDouble = (double)10.0;
      NullableString = "string";

      Booleans = std::vector<bool>(vectorSize, true);
      Int8s = std::vector<int8_t>(vectorSize, (int8_t)1);
      Int16s = std::vector<int16_t>(vectorSize, (int16_t)2);
      Int32s = std::vector<int32_t>(vectorSize, (int32_t)3);
      Int64s = std::vector<int64_t>(vectorSize, (int64_t)4);
      Uint8s = std::vector<uint8_t>(vectorSize, (uint8_t)5);
      Uint16s = std::vector<uint16_t>(vectorSize, (uint16_t)6);
      Uint32s = std::vector<uint32_t>(vectorSize, (uint32_t)7);
      Uint64s = std::vector<uint64_t>(vectorSize, (uint64_t)8);
      Floats = std::vector<float>(vectorSize, (float)9.0);
      Doubles = std::vector<double>(vectorSize, (double)10.0);
      Strings = std::vector<std::string>(vectorSize, "string");

      Map = std::map<std::string, std::string>();
      for (size_t i = 0; i < vectorSize; i++)
      {
        Map["key" + std::to_string(i)] = "value" + std::to_string(i);
      };
    }
  };

  /**
   * @brief Measure the HTTP transport performance.
   */
  class JsonTest : public Azure::Perf::PerfTest {
    enum class Action
    {
      Serialize,
      Deserialize
    };

    Action m_action;
    size_t m_vectorSize;
    JsonTestObject m_testObject;
    std::string m_jsonBody;

  public:
    /**
     * @brief Construct a new JsonTest test.
     *
     * @param options The test options.
     */
    JsonTest(Azure::Perf::TestOptions options) : PerfTest(options) {}

    void Setup() override
    {
      m_action = m_options.GetOptionOrDefault<std::string>("Action", "serialize") == "serialize"
          ? Action::Serialize
          : Action::Deserialize;

      m_vectorSize = m_options.GetOptionOrDefault<size_t>("Size", 1000);
      m_testObject = JsonTestObject(m_vectorSize);

      if (m_action == Action::Deserialize)
      {
        m_jsonBody = m_testObject.Serialize();
      }
    }

    /**
     * @brief Perform Json test.
     *
     */
    void Run(Azure::Core::Context const&) override
    {
      try
      {
        switch (m_action)
        {
          case Action::Serialize: {
            m_testObject.Serialize();
            break;
          }
          case Action::Deserialize: {
            m_testObject.Deserialize(m_jsonBody);
            break;
          }
        }
      }
      catch (std::exception const&)
      {
        // don't print exceptions, they are happening at each request, this is the point of the test
      }
    }

    /**
     * @brief Define the test options for the test.
     *
     * @return The list of test options.
     */
    std::vector<Azure::Perf::TestOption> GetTestOptions() override
    {
      return {
          {"Action", {"--action"}, "Serialize/deserialize, default Serialize", 1, false},
          {"Size", {"--size"}, "The vector size, default 1000", 1, false}};
    }

    /**
     * @brief Get the static Test Metadata for the test.
     *
     * @return Azure::Perf::TestMetadata describing the test.
     */
    static Azure::Perf::TestMetadata GetTestMetadata()
    {
      return {
          "JsonTest",
          "Measures Json serialize/deserialize performance",
          [](Azure::Perf::TestOptions options) {
            return std::make_unique<Azure::Core::Test::JsonTest>(options);
          }};
    }
  };

}}} // namespace Azure::Core::Test

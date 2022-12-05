/*
    __ _____ _____ _____
 __|  |   __|     |   | |  JSON for Modern C++ (test suite)
|  |  |__   |  |  | | | |  version 3.8.0
|_____|_____|_____|_|___|  https://github.com/nlohmann/json

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2013-2019 Niels Lohmann <http://nlohmann.me>.

Permission is hereby  granted, free of charge, to any  person obtaining a copy
of this software and associated  documentation files (the "Software"), to deal
in the Software  without restriction, including without  limitation the rights
to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "doctest_compatibility.h"

#include <azure/core/internal/json/json.hpp>
using Azure::Core::Json::_internal::json;

#include <array>
#include <map>
#include <memory>
#include <string>

namespace udt {
enum class country
{
  china,
  france,
  russia
};

struct age
{
  int m_val;
  age(int rhs = 0) : m_val(rhs) {}
};

struct name
{
  std::string m_val;
  name(const std::string rhs = "") : m_val(rhs) {}
};

struct address
{
  std::string m_val;
  address(const std::string rhs = "") : m_val(rhs) {}
};

struct person
{
  age m_age;
  name m_name;
  country m_country;
  person() : m_age(), m_name(), m_country() {}
  person(const age& a, const name& n, const country& c) : m_age(a), m_name(n), m_country(c) {}
};

struct contact
{
  person m_person;
  address m_address;
  contact() : m_person(), m_address() {}
  contact(const person& p, const address& a) : m_person(p), m_address(a) {}
};

struct contact_book
{
  name m_book_name;
  std::vector<contact> m_contacts;
  contact_book() : m_book_name(), m_contacts() {}
  contact_book(const name& n, const std::vector<contact>& c) : m_book_name(n), m_contacts(c) {}
};
} // namespace udt

// to_json methods
namespace udt {
// templates because of the custom_json tests (see below)
template <typename BasicJsonType> static void to_json(BasicJsonType& j, age a) { j = a.m_val; }

template <typename BasicJsonType> static void to_json(BasicJsonType& j, const name& n)
{
  j = n.m_val;
}

template <typename BasicJsonType> static void to_json(BasicJsonType& j, country c)
{
  switch (c)
  {
    case country::china:
      j = u8"中华人民共和国";
      return;
    case country::france:
      j = "France";
      return;
    case country::russia:
      j = u8"Российская Федерация";
      return;
    default:
      break;
  }
}

template <typename BasicJsonType> static void to_json(BasicJsonType& j, const person& p)
{
  j = BasicJsonType{{"age", p.m_age}, {"name", p.m_name}, {"country", p.m_country}};
}

static void to_json(Azure::Core::Json::_internal::json& j, const address& a) { j = a.m_val; }

static void to_json(Azure::Core::Json::_internal::json& j, const contact& c)
{
  j = json{{"person", c.m_person}, {"address", c.m_address}};
}

static void to_json(Azure::Core::Json::_internal::json& j, const contact_book& cb)
{
  j = json{{"name", cb.m_book_name}, {"contacts", cb.m_contacts}};
}

// operators
static bool operator==(age lhs, age rhs) { return lhs.m_val == rhs.m_val; }

static bool operator==(const address& lhs, const address& rhs) { return lhs.m_val == rhs.m_val; }

static bool operator==(const name& lhs, const name& rhs) { return lhs.m_val == rhs.m_val; }

static bool operator==(const person& lhs, const person& rhs)
{
  return std::tie(lhs.m_name, lhs.m_age) == std::tie(rhs.m_name, rhs.m_age);
}

static bool operator==(const contact& lhs, const contact& rhs)
{
  return std::tie(lhs.m_person, lhs.m_address) == std::tie(rhs.m_person, rhs.m_address);
}

static bool operator==(const contact_book& lhs, const contact_book& rhs)
{
  return std::tie(lhs.m_book_name, lhs.m_contacts) == std::tie(rhs.m_book_name, rhs.m_contacts);
}
} // namespace udt

// from_json methods
namespace udt {
template <typename BasicJsonType> static void from_json(const BasicJsonType& j, age& a)
{
  a.m_val = j.template get<int>();
}

template <typename BasicJsonType> static void from_json(const BasicJsonType& j, name& n)
{
  n.m_val = j.template get<std::string>();
}

template <typename BasicJsonType> static void from_json(const BasicJsonType& j, country& c)
{
  const auto str = j.template get<std::string>();
  static const std::map<std::string, country> m
      = {{u8"中华人民共和国", country::china},
         {"France", country::france},
         {u8"Российская Федерация", country::russia}};

  const auto it = m.find(str);
  // TODO test exceptions
  c = it->second;
}

template <typename BasicJsonType> static void from_json(const BasicJsonType& j, person& p)
{
  p.m_age = j["age"].template get<age>();
  p.m_name = j["name"].template get<name>();
  p.m_country = j["country"].template get<country>();
}

static void from_json(const Azure::Core::Json::_internal::json& j, address& a)
{
  a.m_val = j.get<std::string>();
}

static void from_json(const Azure::Core::Json::_internal::json& j, contact& c)
{
  c.m_person = j["person"].get<person>();
  c.m_address = j["address"].get<address>();
}

static void from_json(const Azure::Core::Json::_internal::json& j, contact_book& cb)
{
  cb.m_book_name = j["name"].get<name>();
  cb.m_contacts = j["contacts"].get<std::vector<contact>>();
}
} // namespace udt

TEST_CASE("basic usage" * doctest::test_suite("udt"))
{

  // a bit narcissistic maybe :) ?
  const udt::age a{23};
  const udt::name n{"theo"};
  const udt::country c{udt::country::france};
  const udt::person sfinae_addict{a, n, c};
  const udt::person senior_programmer{{42}, {u8"王芳"}, udt::country::china};
  const udt::address addr{"Paris"};
  const udt::contact cpp_programmer{sfinae_addict, addr};
  const udt::contact_book book{{"C++"}, {cpp_programmer, {senior_programmer, addr}}};
}

namespace udt {
struct legacy_type
{
  std::string number;
  legacy_type() : number() {}
  legacy_type(const std::string& n) : number(n) {}
};
} // namespace udt

namespace Azure { namespace Core { namespace Json { namespace _internal {
  template <typename T> struct adl_serializer<std::shared_ptr<T>>
  {
    static void to_json(json& j, const std::shared_ptr<T>& opt)
    {
      if (opt)
      {
        j = *opt;
      }
      else
      {
        j = nullptr;
      }
    }

    static void from_json(const json& j, std::shared_ptr<T>& opt)
    {
      if (j.is_null())
      {
        opt = nullptr;
      }
      else
      {
        opt.reset(new T(j.get<T>()));
      }
    }
  };

  template <> struct adl_serializer<udt::legacy_type>
  {
    static void to_json(json& j, const udt::legacy_type& l) { j = std::stoi(l.number); }

    static void from_json(const json& j, udt::legacy_type& l)
    {
      l.number = std::to_string(j.get<int>());
    }
  };
}}}} // namespace Azure::Core::Json::_internal

TEST_CASE("adl_serializer specialization" * doctest::test_suite("udt"))
{
  SECTION("partial specialization")
  {
    SECTION("to_json")
    {
      std::shared_ptr<udt::person> optPerson;

      json j = optPerson;
      CHECK(j.is_null());

      optPerson.reset(new udt::person{{42}, {"John Doe"}, udt::country::russia});
      j = optPerson;
      CHECK_FALSE(j.is_null());

      CHECK(j.get<udt::person>() == *optPerson);
    }

    SECTION("from_json")
    {
      auto person = udt::person{{42}, {"John Doe"}, udt::country::russia};
      json j = person;

      auto optPerson = j.get<std::shared_ptr<udt::person>>();
      REQUIRE(optPerson);
      CHECK(*optPerson == person);

      j = nullptr;
      optPerson = j.get<std::shared_ptr<udt::person>>();
      CHECK(!optPerson);
    }
  }

  SECTION("total specialization")
  {
    SECTION("to_json")
    {
      udt::legacy_type lt{"4242"};

      json j = lt;
      CHECK(j.get<int>() == 4242);
    }

    SECTION("from_json")
    {
      json j = 4242;
      auto lt = j.get<udt::legacy_type>();
      CHECK(lt.number == "4242");
    }
  }
}

namespace Azure { namespace Core { namespace Json { namespace _internal {
  template <> struct adl_serializer<std::vector<float>>
  {
    using type = std::vector<float>;
    static void to_json(json& j, const type&) { j = "hijacked!"; }

    static void from_json(const json&, type& opt) { opt = {42.0, 42.0, 42.0}; }

    // preferred version
    static type from_json(const json&) { return {4.0, 5.0, 6.0}; }
  };
}}}} // namespace Azure::Core::Json::_internal

TEST_CASE("even supported types can be specialized" * doctest::test_suite("udt"))
{
  json j = std::vector<float>{1.0, 2.0, 3.0};
  CHECK(j.dump() == R"("hijacked!")");
  auto f = j.get<std::vector<float>>();
  // the single argument from_json method is preferred
  CHECK((f == std::vector<float>{4.0, 5.0, 6.0}));
}

namespace Azure { namespace Core { namespace Json { namespace _internal {
  template <typename T> struct adl_serializer<std::unique_ptr<T>>
  {
    static void to_json(json& j, const std::unique_ptr<T>& opt)
    {
      if (opt)
      {
        j = *opt;
      }
      else
      {
        j = nullptr;
      }
    }

    // this is the overload needed for non-copyable types,
    static std::unique_ptr<T> from_json(const json& j)
    {
      if (j.is_null())
      {
        return nullptr;
      }
      else
      {
        return std::unique_ptr<T>(new T(j.get<T>()));
      }
    }
  };
}}}} // namespace Azure::Core::Json::_internal

TEST_CASE("Non-copyable types" * doctest::test_suite("udt"))
{
  SECTION("to_json")
  {
    std::unique_ptr<udt::person> optPerson;

    json j = optPerson;
    CHECK(j.is_null());

    optPerson.reset(new udt::person{{42}, {"John Doe"}, udt::country::russia});
    j = optPerson;
    CHECK_FALSE(j.is_null());

    CHECK(j.get<udt::person>() == *optPerson);
  }

  SECTION("from_json")
  {
    auto person = udt::person{{42}, {"John Doe"}, udt::country::russia};
    json j = person;

    auto optPerson = j.get<std::unique_ptr<udt::person>>();
    REQUIRE(optPerson);
    CHECK(*optPerson == person);

    j = nullptr;
    optPerson = j.get<std::unique_ptr<udt::person>>();
    CHECK(!optPerson);
  }
}

// custom serializer - advanced usage
// pack structs that are pod-types (but not scalar types)
// relies on adl for any other type
template <typename T, typename = void> struct pod_serializer
{
  // use adl for non-pods, or scalar types
  template <
      typename BasicJsonType,
      typename U = T,
      typename std::enable_if<not(std::is_pod<U>::value and std::is_class<U>::value), int>::type
      = 0>
  static void from_json(const BasicJsonType& j, U& t)
  {
    using Azure::Core::Json::_internal::from_json;
    from_json(j, t);
  }

  // special behaviour for pods
  template <
      typename BasicJsonType,
      typename U = T,
      typename std::enable_if<std::is_pod<U>::value and std::is_class<U>::value, int>::type = 0>
  static void from_json(const BasicJsonType& j, U& t)
  {
    std::uint64_t value;
    // TODO The following block is no longer relevant in this serializer, make another one that
    // shows the issue the problem arises only when one from_json method is defined without any
    // constraint
    //
    // Why cannot we simply use: j.get<std::uint64_t>() ?
    // Well, with the current experiment, the get method looks for a from_json
    // function, which we are currently defining!
    // This would end up in a stack overflow. Calling Azure::Core::Json::_internal::from_json is a
    // workaround (is it?).
    // I shall find a good way to avoid this once all constructors are converted
    // to free methods
    //
    // In short, constructing a json by constructor calls to_json
    // calling get calls from_json, for now, we cannot do this in custom
    // serializers
    Azure::Core::Json::_internal::from_json(j, value);
    auto bytes = static_cast<char*>(static_cast<void*>(&value));
    std::memcpy(&t, bytes, sizeof(value));
  }

  template <
      typename BasicJsonType,
      typename U = T,
      typename std::enable_if<not(std::is_pod<U>::value and std::is_class<U>::value), int>::type
      = 0>
  static void to_json(BasicJsonType& j, const T& t)
  {
    using Azure::Core::Json::_internal::to_json;
    to_json(j, t);
  }

  template <
      typename BasicJsonType,
      typename U = T,
      typename std::enable_if<std::is_pod<U>::value and std::is_class<U>::value, int>::type = 0>
  static void to_json(BasicJsonType& j, const T& t) noexcept
  {
    auto bytes = static_cast<const unsigned char*>(static_cast<const void*>(&t));
    std::uint64_t value;
    std::memcpy(&value, bytes, sizeof(value));
    Azure::Core::Json::_internal::to_json(j, value);
  }
};

namespace udt {
struct small_pod
{
  int begin;
  char middle;
  short end;
};

struct non_pod
{
  std::string s;
  non_pod() : s() {}
  non_pod(const std::string& S) : s(S) {}
};

template <typename BasicJsonType> static void to_json(BasicJsonType& j, const non_pod& np)
{
  j = np.s;
}

template <typename BasicJsonType> static void from_json(const BasicJsonType& j, non_pod& np)
{
  np.s = j.template get<std::string>();
}

static bool operator==(small_pod lhs, small_pod rhs) noexcept
{
  return std::tie(lhs.begin, lhs.middle, lhs.end) == std::tie(rhs.begin, rhs.middle, rhs.end);
}

static bool operator==(const non_pod& lhs, const non_pod& rhs) noexcept { return lhs.s == rhs.s; }

static std::ostream& operator<<(std::ostream& os, small_pod l)
{
  return os << "begin: " << l.begin << ", middle: " << l.middle << ", end: " << l.end;
}
} // namespace udt

TEST_CASE("custom serializer for pods" * doctest::test_suite("udt"))
{
  using custom_json = Azure::Core::Json::_internal::basic_json<
      std::map,
      std::vector,
      std::string,
      bool,
      std::int64_t,
      std::uint64_t,
      double,
      std::allocator,
      pod_serializer>;

  auto p = udt::small_pod{42, '/', 42};
  custom_json j = p;

  auto p2 = j.get<udt::small_pod>();

  CHECK(p == p2);

  auto np = udt::non_pod{{"non-pod"}};
  custom_json j2 = np;
  auto np2 = j2.get<udt::non_pod>();
  CHECK(np == np2);
}

template <typename T, typename> struct another_adl_serializer;

using custom_json = Azure::Core::Json::_internal::basic_json<
    std::map,
    std::vector,
    std::string,
    bool,
    std::int64_t,
    std::uint64_t,
    double,
    std::allocator,
    another_adl_serializer>;

template <typename T, typename> struct another_adl_serializer
{
  static void from_json(const custom_json& j, T& t)
  {
    using Azure::Core::Json::_internal::from_json;
    from_json(j, t);
  }

  static void to_json(custom_json& j, const T& t)
  {
    using Azure::Core::Json::_internal::to_json;
    to_json(j, t);
  }
};

TEST_CASE("custom serializer that does adl by default" * doctest::test_suite("udt"))
{
  auto me = udt::person{{23}, {"theo"}, udt::country::france};

  json j = me;
  custom_json cj = me;

  CHECK(j.dump() == cj.dump());

  CHECK(me == j.get<udt::person>());
  CHECK(me == cj.get<udt::person>());
}

TEST_CASE("different basic_json types conversions")
{
  SECTION("null")
  {
    json j;
    custom_json cj = j;
    CHECK(cj == nullptr);
  }

  SECTION("boolean")
  {
    json j = true;
    custom_json cj = j;
    CHECK(cj == true);
  }

  SECTION("discarded")
  {
    json j(json::value_t::discarded);
    custom_json cj;
    CHECK_NOTHROW(cj = j);
    CHECK(cj.type() == custom_json::value_t::discarded);
  }

  SECTION("array")
  {
    json j = {1, 2, 3};
    custom_json cj = j;
    CHECK((cj == std::vector<int>{1, 2, 3}));
  }

  SECTION("integer")
  {
    json j = 42;
    custom_json cj = j;
    CHECK(cj == 42);
  }

  SECTION("float")
  {
    json j = 42.0;
    custom_json cj = j;
    CHECK(cj == 42.0);
  }

  SECTION("unsigned")
  {
    json j = 42u;
    custom_json cj = j;
    CHECK(cj == 42u);
  }

  SECTION("string")
  {
    json j = "forty-two";
    custom_json cj = j;
    CHECK(cj == "forty-two");
  }

  SECTION("binary")
  {
    json j = json::binary({1, 2, 3}, 42);
    custom_json cj = j;
    CHECK(cj.get_binary().subtype() == 42);
    std::vector<std::uint8_t> cv = cj.get_binary();
    std::vector<std::uint8_t> v = j.get_binary();
    CHECK(cv == v);
  }

  SECTION("object")
  {
    json j = {{"forty", "two"}};
    custom_json cj = j;
    auto m = j.get<std::map<std::string, std::string>>();
    CHECK(cj == m);
  }

  SECTION("get<custom_json>")
  {
    json j = 42;
    custom_json cj = j.get<custom_json>();
    CHECK(cj == 42);
  }
}

namespace {
struct incomplete;

// std::is_constructible is broken on macOS' libc++
// use the cppreference implementation

template <typename T, typename = void> struct is_constructible_patched : std::false_type
{
};

template <typename T>
struct is_constructible_patched<T, decltype(void(json(std::declval<T>())))> : std::true_type
{
};
} // namespace

TEST_CASE(
    "an incomplete type does not trigger a compiler error in non-evaluated context"
    * doctest::test_suite("udt"))
{
  static_assert(not is_constructible_patched<json, incomplete>::value, "");
}

namespace {
class Evil {
public:
  Evil() = default;
  template <typename T> Evil(T t) : m_i(sizeof(t)) {}

  int m_i = 0;
};

void from_json(const json&, Evil&) {}
} // namespace

TEST_CASE("Issue #924")
{
  // Prevent get<std::vector<Evil>>() to throw
  auto j = json::array();

  CHECK_NOTHROW(j.get<Evil>());
  CHECK_NOTHROW(j.get<std::vector<Evil>>());

  // silence Wunused-template warnings
  Evil e(1);
  CHECK(e.m_i >= 0);
}

TEST_CASE("Issue #1237")
{
  struct non_convertible_type
  {
  };
  static_assert(not std::is_convertible<json, non_convertible_type>::value, "");
}
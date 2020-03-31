

#include "context.hpp"

using namespace Azure::Core;

struct SomethingUnexpected : ValueBase
{
  int a, b, c;
  std::string d;
};


int main()
{
  using namespace std::chrono;
  using namespace std::chrono_literals;
  Azure::Core::Context ctx;
  auto now = system_clock::now();
  auto ten_minutes_from_now = now + 10min;
  auto ten_seconds_from_now = now + 10s;
  auto in_ten_minutes = ctx.WithDeadline(ten_minutes_from_now);
  auto in_ten_seconds = in_ten_minutes.WithDeadline(ten_seconds_from_now);
  assert(in_ten_minutes.CancelWhen() == ten_minutes_from_now);
  assert(in_ten_seconds.CancelWhen() == ten_seconds_from_now);

  auto backwards = ctx.WithDeadline(ten_seconds_from_now).WithDeadline(ten_minutes_from_now);
  assert(backwards.CancelWhen() == ten_seconds_from_now);

  auto imbued = ctx.WithValue("example key", true);
  assert(imbued["example key"].get<bool>() == true);
  const auto& the_thing = imbued["some other nonexistent key"];
  if (the_thing.alternative() == not_found)
  {
    assert(true);
  }

  {
    Context with_complex_thing;
    {
      std::unique_ptr<SomethingUnexpected> stuff{new SomethingUnexpected};
      stuff->a = 42;
      stuff->b = 1729;
      stuff->c = 0xFFFF;
      stuff->d = "some string content";

      with_complex_thing = ctx.WithValue("key", std::move(stuff));
      assert(stuff.get() == nullptr);
    }
  } // the delete happens here

  puts("pass");
}

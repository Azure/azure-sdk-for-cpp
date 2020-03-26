#include "context.hpp"


struct something_you_didnt_see_coming : value_base
{
  int a, b, c;
  std::string d;
};


int main()
{
  using namespace std::chrono;
  using namespace std::chrono_literals;
  Context ctx;
  auto now = system_clock::now();
  auto ten_minutes_from_now = now + 10min;
  auto ten_seconds_from_now = now + 10s;
  auto in_ten_minutes = ctx.with_deadline(ten_minutes_from_now);
  auto in_ten_seconds = in_ten_minutes.with_deadline(ten_seconds_from_now);
  assert(in_ten_minutes.cancel_when() == ten_minutes_from_now);
  assert(in_ten_seconds.cancel_when() == ten_seconds_from_now);

  auto backwards = ctx.with_deadline(ten_seconds_from_now).with_deadline(ten_minutes_from_now);
  assert(backwards.cancel_when() == ten_seconds_from_now);

  auto imbued = ctx.with_value("example key", true);
  assert(imbued["example key"].get<bool>() == true);
  const auto& the_thing = imbued["some other nonexistent key"];
  if (the_thing.alternative() == not_found)
  {
    assert(true);
  }

  {
    Context with_complex_thing;
    {
      std::unique_ptr<something_you_didnt_see_coming> stuff{new something_you_didnt_see_coming};
      stuff->a = 42;
      stuff->b = 1729;
      stuff->c = 0xFFFF;
      stuff->d = "some string content";

      with_complex_thing = ctx.with_value("key", std::move(stuff));
      assert(stuff.get() == nullptr);
    }
  } // the delete happens here

  puts("pass");
}
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <atomic>
#include <cstdlib>
#include <thread>
#include <vector>

extern "C" int CountedBranch(int value)
{
  if (value % 2 == 0)
  {
    return 1;
  }
  return 0;
}

int main()
{
  constexpr int ThreadCount = 16;
  constexpr int Iterations = 100000;
  std::atomic<int> ready{0};
  std::atomic<bool> start{false};
  std::vector<std::thread> threads;
  for (int index = 0; index < ThreadCount; ++index)
  {
    threads.emplace_back([&]() {
      ++ready;
      while (!start.load())
      {
        std::this_thread::yield();
      }
      int count = 0;
      for (int iteration = 0; iteration < Iterations; ++iteration)
      {
        count += CountedBranch(iteration);
      }
      if (count != Iterations / 2)
      {
        std::abort();
      }
    });
  }
  while (ready.load() != ThreadCount)
  {
    std::this_thread::yield();
  }
  start = true;
  for (auto& thread : threads)
  {
    thread.join();
  }
}

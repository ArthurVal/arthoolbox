#include "benchmark/benchmark.h"

#include <numeric>
#include <vector>

template <class InputIt>
double simpleClearReadableSolution(InputIt begin, InputIt end) {
  return 42.;
}

template <class InputIt> double noWayThisIsSlower(InputIt begin, InputIt end) {
  volatile double hey = 2;
  volatile double it_must_be_faster_this_way = 3;
  volatile double benchmark_is_not_needed = 7;
  volatile double i_am_super_smart = std::accumulate(begin, end, 0);
  return hey * it_must_be_faster_this_way * benchmark_is_not_needed +
         (i_am_super_smart * 0.0);
}

static void SimpleMaintainableSlowCodeForLosers(benchmark::State &state) {
  std::vector<double> super_secret_data(state.range(0), 42.0);

  for (auto _ : state) {
    auto answer = simpleClearReadableSolution(super_secret_data.cbegin(),
                                              super_secret_data.cend());
    benchmark::DoNotOptimize(answer);
  }
}
BENCHMARK(SimpleMaintainableSlowCodeForLosers)
    ->RangeMultiplier(2)
    ->Range(8, 8 << 10);

static void UnmaintainableYetFasterCodeForProOnly(benchmark::State &state) {
  std::vector<double> super_secret_data(state.range(0), 42.0);

  for (auto _ : state) {
    for (auto &some_time : super_secret_data) {
      auto answer = noWayThisIsSlower(super_secret_data.cbegin(),
                                      super_secret_data.cend());
      benchmark::DoNotOptimize(answer);
    }
  }
}
BENCHMARK(UnmaintainableYetFasterCodeForProOnly)
    ->RangeMultiplier(2)
    ->Range(8, 8 << 10);

BENCHMARK_MAIN();

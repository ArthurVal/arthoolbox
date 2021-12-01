#include <benchmark/benchmark.h>

#include "arthoolbox/math/statistics.hpp"

#include <iterator>
#include <random>
#include <vector>

namespace arthoolbox {
namespace stats {

std::vector<double> generateRandomNormalDistribution(double mean, double stddev,
                                                     std::size_t n) noexcept {
  std::random_device seed_generator;
  std::mt19937 random_generator(seed_generator());
  std::normal_distribution<double> distribution(mean, stddev);

  std::vector<double> out;
  out.reserve(n);

  std::generate_n(std::back_inserter(out), n,
                  [&distribution, &random_generator]() {
                    return distribution(random_generator);
                  });

  return out;
}

const std::vector<double> &getRandomData() {
  static auto out = generateRandomNormalDistribution(42, 5e-3, 500);
  return out;
}

static void BM_ComputeRecurringMean(benchmark::State &state) {
  const auto& data = getRandomData();
  for (auto _ : state) {
    double mean = 0.0;
    for(auto i = 0; i < data.size(); i++) {
      const auto &sample = data[i];
      const auto n = i + 1;

      mean = update_recurring_mean(sample, mean, i + 1);
    }
    benchmark::DoNotOptimize(mean);
  }
}
BENCHMARK(BM_ComputeRecurringMean);

static void BM_ComputeRecurringMeanVariance(benchmark::State &state) {
  const auto& data = getRandomData();
  for (auto _ : state) {
    double mean = 0.0;
    double variance = 0.0;

    for(auto i = 0; i < data.size(); i++) {
      const auto &sample = data[i];
      const auto n = i + 1;
      const auto old_mean = mean;

      mean = update_recurring_mean(sample, mean, n);
      variance = update_recurring_variance(sample, variance, mean, old_mean, n);
    }

    benchmark::DoNotOptimize(mean);
    benchmark::DoNotOptimize(variance);
  }
}
BENCHMARK(BM_ComputeRecurringMeanVariance);

static void BM_ComputeRecurringStatisticsCtor(benchmark::State &state) {
  for (auto _ : state) {
    RecurrentStatistics<double, double> stats;
    benchmark::DoNotOptimize(stats);
  }
}
BENCHMARK(BM_ComputeRecurringStatisticsCtor);

static void BM_ComputeRecurringStatistics(benchmark::State &state) {
  const auto& data = getRandomData();

  for (auto _ : state) {
    RecurrentStatistics<double, double> stats;

    for(const auto &sample : data)
      stats.updateWith(sample);

    benchmark::DoNotOptimize(stats);
  }
}
BENCHMARK(BM_ComputeRecurringStatistics);

} // namespace stats
} // namespace arthoolbox

BENCHMARK_MAIN();

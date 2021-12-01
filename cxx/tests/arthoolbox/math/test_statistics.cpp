#include <cmath>
#include <gtest/gtest.h>

#include "arthoolbox/math/statistics.hpp"

#include <algorithm>
#include <cstdlib>
#include <iterator>
#include <memory>
#include <numeric>
#include <random>

namespace arthoolbox {
namespace stats {
namespace {

// TODO WE MUST TEST DIFFERENT TYPES ETC...
struct Statistics : public ::testing::Test {
  using data_type = double;

  struct DataHolder {
    data_type mean;
    data_type variance;
    std::vector<data_type> samples;
  };

  static void SetUpTestSuite() {
    constexpr data_type N = 5e3;

    std::random_device seed_generator;
    std::mt19937 random_generator(seed_generator());
    std::normal_distribution<data_type> distribution(50, 5e-3);

    data.samples.reserve(N);

    std::generate_n(std::back_inserter(data.samples), N,
                    [&distribution, &random_generator]() {
                      return distribution(random_generator);
                    });

    data.mean = 0;
    data.mean =
        std::accumulate(data.samples.cbegin(), data.samples.cend(), data.mean);
    data.mean /= data.samples.size();

    data.variance = 0;
    for (const auto &sample : data.samples)
      data.variance += std::pow((sample - data.mean), 2);
    data.variance /= data.samples.size();
  }

  static DataHolder data;
};

Statistics::DataHolder Statistics::data;


TEST_F(Statistics, RecurringMeanComputation) {
  data_type mean_computed_online = 0;

  for (std::size_t i = 0; i < data.samples.size(); i++) {
    const data_type &sample = data.samples[i];
    mean_computed_online =
        update_recurring_mean(sample, mean_computed_online, i + 1);
  }

  ASSERT_NEAR(mean_computed_online, data.mean, 1e-6);
}

TEST_F(Statistics, RecurringVarianceComputation) {
  data_type mean_computed_online = 0;
  data_type variance_computed_online = 0;

  for (std::size_t i = 0; i < data.samples.size(); i++) {
    const data_type &sample = data.samples[i];
    const auto old_mean = mean_computed_online;
    const auto n = i+1;

    mean_computed_online =
        update_recurring_mean(sample, mean_computed_online, n);

    variance_computed_online = update_recurring_variance(
        sample, variance_computed_online, mean_computed_online, old_mean, n);
  }

  ASSERT_NEAR(variance_computed_online, data.variance, 1e-6);
}

} // namespace
} // namespace stats
} // namespace arthoolbox

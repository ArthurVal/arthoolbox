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
protected:
  using data_type = double;

  struct DataHolder {
    data_type mean;
    data_type variance;
    std::vector<data_type> raw;
  };

  void SetUp() override {
    constexpr data_type N = 5e3;

    std::random_device seed_generator;
    std::mt19937 random_generator(seed_generator());
    std::normal_distribution<data_type> distribution(50, 5e-3);

    data_.raw.reserve(N);

    std::generate_n(std::back_inserter(data_.raw), N,
                    [&distribution, &random_generator]() {
                      return distribution(random_generator);
                    });

    data_.mean = 0;
    data_.mean =
        std::accumulate(data_.raw.cbegin(), data_.raw.cend(), data_.mean);
    data_.mean /= data_.raw.size();

    data_.variance = 0;
    for (const auto &data : data_.raw)
      data_.variance += std::pow((data - data_.mean), 2);
    data_.variance /= data_.raw.size();
  }

  void TearDown() override {}

  DataHolder data_;
};

TEST_F(Statistics, RecurringMeanComputation) {
  data_type mean_computed_online = 0;

  for (std::size_t i = 0; i < data_.raw.size(); i++) {
    const data_type &data = data_.raw[i];
    mean_computed_online =
        update_recurring_mean(data, mean_computed_online, i + 1);
  }

  ASSERT_NEAR(mean_computed_online, data_.mean, 1e-6);
}

TEST_F(Statistics, RecurringVarianceComputation) {
  data_type mean_computed_online = 0;
  data_type variance_computed_online = 0;

  for (std::size_t i = 0; i < data_.raw.size(); i++) {
    const data_type &data = data_.raw[i];
    const auto old_mean = mean_computed_online;
    const auto n = i+1;

    mean_computed_online =
        update_recurring_mean(data, mean_computed_online, n);

    variance_computed_online = update_recurring_variance(
        data, variance_computed_online, mean_computed_online, old_mean, n);
  }

  ASSERT_NEAR(variance_computed_online, data_.variance, 1e-6);
}

} // namespace
} // namespace stats
} // namespace arthoolbox

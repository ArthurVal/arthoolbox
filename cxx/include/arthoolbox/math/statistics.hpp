#pragma once
/**
 *   \file statistics.hpp
 *   \brief Contains usefulll tools to perform staticstics computations
 */

#include <cassert> // assert
#include <cstdlib> // std::size_t
#include <sstream> // stringstream

namespace arthoolbox {
namespace stats {

/**
 *  \brief Update the statistical mean using recurrent equation
 *
 *  From the previously mean computed using the n-1 measurments (M_n-1), we
 *  compute the new mean (M_n) adding a new measurement (X_n) with the formula:
 *  M_n = M_n-1 + (X_n - M_n-1)/n
 *
 * See: https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
 *
 *  \tparam T The measurment type
 *  \tparam U The mean type
 *
 *  \param[in] new_sample The new measurment Xn
 *  \param[in] old_mean The previously computed mean Mn-1
 *  \param[in] data_number The number of data n used to compute Mn
 *  \return T The new mean computed
 */
template <class T, class U = T>
constexpr U update_recurring_mean(const T &new_sample, const U &old_mean,
                                  const std::size_t data_number) noexcept {
  return (old_mean + (new_sample - old_mean) / data_number);
}

/**
 *  \brief Update the statistical variance using recurrent equation
 *
 *  From the previously variance computed using the n and n-1 means (Mn, M_n-1)
 *  and the new measurment Xn, we compute the new variance (V_n) using:
 *  V_n = V_n-1 + (((X_n - M_n)*(X_n - M_n-1) - V_n-1) / n)
 *
 * See: https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
 *
 *  \note This formula may be unstable due to floating point calculation issues.
 *
 *  \tparam T The measurment type
 *  \tparam U The mean type
 *  \tparam V The variance type
 *
 *  \param[in] new_sample The new measurment Xn
 *  \param[in] old_variance The previously computed variance Vn-1
 *  \param[in] new_mean The new mean Mn
 *  \param[in] old_mean The previously computed mean Mn-1
 *  \param[in] data_number The number of data n used to compute Mn/Vn
 *  \return T The new variance computed
 */
template <class T, class U = T, class V = T>
constexpr T update_recurring_variance(const T &new_sample,
                                      const V &old_variance, const U &new_mean,
                                      const U &old_mean,
                                      const std::size_t data_number) noexcept {
  return (old_variance +
          ((new_sample - new_mean) * (new_sample - old_mean) - old_variance) /
              data_number);
}

/**
 *  \brief Update the statistical sum of squares using recurrent equation
 *
 *  From the previously sum computed (SUM_n-1), using the n and n-1 means (Mn,
 *  M_n-1) and the new measurment Xn, we compute the new sum of squares (SUM_n)
 *  using:
 *  SUM_n = SUM_n-1 + (X_n - M_n)*(X_n - M_n-1)
 *
 *  This SUM can be use to compute the variance and sample variance:
 *  Vn = SUM_n/n
 *  Sn = SUM_n/(n+1)
 *
 *  See: https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
 *
 *  \note This formula make the variance computation suffer less from floating
 *  point computation instabilities
 *
 *  \tparam T The measurment type
 *  \tparam U The mean type
 *  \tparam S The sum squares type
 *
 *  \param[in] new_sample The new measurment Xn
 *  \param[in] old_sum_square The previously computed variance Vn-1
 *  \param[in] new_mean The new mean Mn
 *  \param[in] old_mean The previously computed mean Mn-1
 *  \return T The new variance computed
 */
template <class T, class U = T, class S = T>
constexpr T
update_recurring_sum_square(const T &new_sample, const S &old_sum_square,
                            const U &new_mean, const U &old_mean) noexcept {
  return (old_sum_square + (new_sample - new_mean) * (new_sample - old_mean));
}

/**
 *  \brief Enables online recurrent statistics computation of a given sample
 *
 *  \notes This class is not thread safe !
 *
 *  \tparam T The measurement sample type
 *  \tparam U The mean type
 *  \tparam S The variance type
 */
template <class T, class U, class S = U> class RecurrentStatistics {
public:
  //! Default constructor
  RecurrentStatistics() : number_of_measurements_(0), mean_(), sum_square_(){};

  //! Constructor with non default parameters
  RecurrentStatistics(const U &init_mean, const S &init_sum_square)
      : number_of_measurements_(0), mean_(init_mean),
        sum_square_(init_sum_square){};

  /**
   *  \brief Reset the statistics (N set to 0)
   *  \param[in] mean The new default value used as mean
   *  \param[in] sum_square The new default value used as sum_square
   */
  void reset(const U &mean, const S &sum_square) noexcept {
    number_of_measurements_ = 0;
    mean_ = mean;
    sum_square_ = sum_square;
  };

  /**
   *  \brief Get the number of measurement N
   *  \return std::size_t N the number of measurment
   */
  std::size_t getNumberOfMeasurements() const noexcept {
    return number_of_measurements_;
  }

  /**
   *  \brief Get the currently computed mean
   *  \return U The mean computed
   */
  U getMean() const noexcept { return mean_; }

  /**
   *  \brief Get the currently computed variance
      \notes Number of measurement must be != 0
   *  \return U the variance computed
   */
  S getVariance() const {
    assert(number_of_measurements_ != 0 && "Need at least 1 measurement to compute the variance");
    return sum_square_ / number_of_measurements_;
  }

  /**
   *  \brief Get the currently computed sampled variance
      \notes Number of measurement must be > 1
   *  \return U the sampled variance computed
   */
  S getSampledVariance() const {
    assert(number_of_measurements_ > 1 &&
           "Need at least 2 measurement to compute the sample variance");
    return sum_square_ / (number_of_measurements_ - 1);
  }

  /**
   *  \brief Update the computed stats with the new measurement
   *  \param[in] new_data The new measurement use to update the data
   */
  void updateWith(const T &new_data) {
    number_of_measurements_++;
    const auto new_mean =
        update_recurring_mean(new_data, mean_, number_of_measurements_);
    sum_square_ =
        update_recurring_sum_square(new_data, sum_square_, new_mean, mean_);
    mean_ = new_mean;
  };

private:
  std::size_t
      number_of_measurements_; /*!< Hold the current number of measurment N */
  U mean_;                     /*!< Hold the currently computed mean */
  S sum_square_;               /*!< Hold the sum square */
};

/**
 *  \brief Format a RecurrentStatistics to a string
 *  \param[in] stats The RecurrentStatistics object to format
 *  \return std::string The string format of the RecurrentStatistics
 */
template <class T, class U, class S>
std::string format(const RecurrentStatistics<T, U, S> &stats) {
  constexpr const char *not_enough_samples = " -> Not enough samples yet";

  std::stringstream output;
  const auto n = stats.getNumberOfMeasurements();
  output << "Stats [N = " << n;
  output << "]\nMean: " << stats.getMean();

  switch (n) {
  case 0:
    output << "\nVar : " << not_enough_samples;
    output << "\nSVar: " << not_enough_samples;
    break;

  case 1:
    output << "\nVar : " << stats.getVariance();
    output << "\nSVar: " << not_enough_samples;
    break;

  default:
    output << "\nVar : " << stats.getVariance();
    output << "\nSVar: " << stats.getSampledVariance();
  }

  return output.str();
}

} // namespace stats
} // namespace arthoolbox

#pragma once

#include <cassert>
#include <string>

namespace arthoolbox {

/**
 * @brief Data structure for versions number (major.minor.patch)
 */
struct VersionNumber {
  /**
   * @brief Construct a VersionNumber from a string
   *
   * The parsing doesn't enforce the expected format by any means and it is
   * expected to have the MAJOR/SEPARATOR_1/MINOR/SEPARATOR_2/PATCH format.
   *
   * @param[in] version The version number to stringify
   * @param[in] sep The separator used (default: ".")
   * @return std::string of the version number major + sep + minor + sep + patch
   */
  static inline VersionNumber from_string(const std::string &str,
                                          const std::string &sep = ".") {
    const auto first_sep = str.find(sep, 0);
    assert(first_sep != std::string::npos);
    const auto minor_begin = first_sep + 1;

    const auto second_sep = str.find(sep, minor_begin);
    assert(second_sep != std::string::npos);
    const auto patch_begin = second_sep + 1;

    return VersionNumber{
        .major = static_cast<unsigned>(std::stoul(str.substr(0, first_sep))),
        .minor = static_cast<unsigned>(
            std::stoul(str.substr(minor_begin, second_sep - minor_begin))),
        .patch = static_cast<unsigned>(std::stoul(str.substr(patch_begin)))};
  }

  unsigned major, /*!< Major version number */
      minor,      /*!< Minor version number */
      patch;      /*!< Patch version number */
};

/**
 * @brief Stringify a VersionNumber
 * @param[in] version The version number to stringify
 * @param[in] sep The separator used (default: ".")
 * @return std::string of the version number major + sep + minor + sep + patch
 */
inline std::string to_string(const VersionNumber &version,
                             const std::string &sep = ".") {
  std::string out = std::to_string(version.major);
  out += sep + std::to_string(version.minor);
  out += sep + std::to_string(version.patch);
  return out;
}

constexpr bool operator<(const VersionNumber& lhs, const VersionNumber& rhs) {
  if (lhs.major != rhs.major) return lhs.major < rhs.major;
  else if (lhs.minor != rhs.minor) return lhs.minor < rhs.minor;
  else return lhs.patch < rhs.patch;
}

constexpr bool operator>(const VersionNumber& lhs, const VersionNumber& rhs) {
  return rhs < lhs;
}

constexpr bool operator<=(const VersionNumber& lhs, const VersionNumber& rhs) {
  return not(lhs > rhs);
}

constexpr bool operator>=(const VersionNumber& lhs, const VersionNumber& rhs) {
  return not(lhs < rhs);
}

constexpr bool operator==(const VersionNumber& lhs, const VersionNumber& rhs) {
  return ((lhs.major == rhs.major) && (lhs.minor == rhs.minor) &&
          (lhs.patch == rhs.patch));
}

constexpr bool operator!=(const VersionNumber& lhs, const VersionNumber& rhs) {
  return not(lhs == rhs);
}

}  // namespace arthoolbox

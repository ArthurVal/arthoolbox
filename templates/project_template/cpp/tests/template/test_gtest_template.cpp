#include <gtest/gtest.h>

#include <string>

// https://google.github.io/googletest/

namespace {
  constexpr const char* theBestViennoiserie() {
    // Obviously !
    return "Chocolatine";
  }

  TEST(UselessTests, ThisObviouslyWork) {
    ASSERT_STREQ(theBestViennoiserie(), "Chocolatine");
    ASSERT_STRNE(theBestViennoiserie(), "Pain au Chocolat");
  }

  TEST(UselessTests, ThisWillNeverWork) {
    ASSERT_STREQ(theBestViennoiserie(), "Pain au Chocolat");
  }
}

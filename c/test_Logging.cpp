#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <cstdlib>

#define ATB_LOG_LEVEL ATB_LOG_LEVEL_DEBUG

extern "C" {
#include "Logging.h"
}

namespace {
TEST(CoreLogging, Debug) {
  testing::internal::CaptureStdout();
  testing::internal::CaptureStderr();

  ATB_LOG(DEBUG, "Test");

  EXPECT_THAT(testing::internal::GetCapturedStdout(),
              testing::ContainsRegex("\\[[0-9]+\\]\\[DEBUG  \\] Test\n"));
  EXPECT_EQ(testing::internal::GetCapturedStderr(), "");

  testing::internal::CaptureStdout();
  testing::internal::CaptureStderr();

  ATB_LOG(DEBUG, "Test %d %s", 42, "Coucou");

  EXPECT_THAT(
      testing::internal::GetCapturedStdout(),
      testing::ContainsRegex("\\[[0-9]+\\]\\[DEBUG  \\] Test 42 Coucou\n"));
  EXPECT_EQ(testing::internal::GetCapturedStderr(), "");
}

TEST(CoreLogging, Info) {
  testing::internal::CaptureStdout();
  testing::internal::CaptureStderr();

  ATB_LOG(INFO, "Test");

  EXPECT_THAT(testing::internal::GetCapturedStdout(),
              testing::ContainsRegex("\\[[0-9]+\\]\\[INFO   \\] Test\n"));
  EXPECT_EQ(testing::internal::GetCapturedStderr(), "");

  testing::internal::CaptureStdout();
  testing::internal::CaptureStderr();

  ATB_LOG(INFO, "Test %d %s", 42, "Coucou");

  EXPECT_THAT(
      testing::internal::GetCapturedStdout(),
      testing::ContainsRegex("\\[[0-9]+\\]\\[INFO   \\] Test 42 Coucou\n"));
  EXPECT_EQ(testing::internal::GetCapturedStderr(), "");
}

TEST(CoreLogging, Warning) {
  testing::internal::CaptureStdout();
  testing::internal::CaptureStderr();

  ATB_LOG(WARNING, "Test");

  EXPECT_EQ(testing::internal::GetCapturedStdout(), "");
  EXPECT_THAT(testing::internal::GetCapturedStderr(),
              testing::ContainsRegex("\\[[0-9]+\\]\\[WARNING\\] Test\n"));

  testing::internal::CaptureStdout();
  testing::internal::CaptureStderr();

  ATB_LOG(WARNING, "Test %d %s", 42, "Coucou");

  EXPECT_EQ(testing::internal::GetCapturedStdout(), "");
  EXPECT_THAT(
      testing::internal::GetCapturedStderr(),
      testing::ContainsRegex("\\[[0-9]+\\]\\[WARNING\\] Test 42 Coucou\n"));
}

TEST(CoreLogging, Error) {
  testing::internal::CaptureStdout();
  testing::internal::CaptureStderr();

  ATB_LOG(ERROR, "Test");

  EXPECT_EQ(testing::internal::GetCapturedStdout(), "");
  EXPECT_THAT(testing::internal::GetCapturedStderr(),
              testing::ContainsRegex("\\[[0-9]+\\]\\[ERROR  \\] Test\n"));

  testing::internal::CaptureStdout();
  testing::internal::CaptureStderr();

  ATB_LOG(ERROR, "Test %d %s", 42, "Coucou");

  EXPECT_EQ(testing::internal::GetCapturedStdout(), "");
  EXPECT_THAT(
      testing::internal::GetCapturedStderr(),
      testing::ContainsRegex("\\[[0-9]+\\]\\[ERROR  \\] Test 42 Coucou\n"));
}

TEST(CoreLogging, Fatal) {
  EXPECT_EXIT(ATB_LOG(FATAL, "Test"), ::testing::ExitedWithCode(EXIT_FAILURE),
              testing::ContainsRegex("\\[[0-9]+\\]\\[FATAL  \\] Test\n"));

  EXPECT_EXIT(
      ATB_LOG(FATAL, "Test %d %s", 42, "Coucou"),
      ::testing::ExitedWithCode(EXIT_FAILURE),
      testing::ContainsRegex("\\[[0-9]+\\]\\[FATAL  \\] Test 42 Coucou\n"));
}

} // namespace

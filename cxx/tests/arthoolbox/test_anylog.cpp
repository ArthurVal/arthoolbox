#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <type_traits>

#include "arthoolbox/anylogger.hpp"

namespace arthoolbox::log {

namespace {
struct StaticFunction {
  static constexpr void debug(std::string_view) {}
  static constexpr void info(std::string_view) {}
  static constexpr void warn(std::string_view) {}
  static constexpr void error(std::string_view) {}
  static constexpr void critical(std::string_view) {}
};

struct MixedFunction {
  constexpr void debug(std::string_view) const {}
  static constexpr void info(std::string_view) {}
  constexpr void warn(std::string_view) const {}
  static constexpr void error(std::string_view) {}
  constexpr void critical(std::string_view) const {}
};

struct PolicyMock {
  inline PolicyMock() { std::puts("DEFAULT CTOR"); }
  inline PolicyMock(const PolicyMock &) { std::puts("COPY CTOR"); }
  inline PolicyMock(PolicyMock &&) { std::puts("MOVE CTOR"); }
  inline PolicyMock &operator=(const PolicyMock &) {
    std::puts("COPY ASSIGN");
    return *this;
  }
  inline PolicyMock &operator=(PolicyMock &&) {
    std::puts("MOVE ASSIGN");
    return *this;
  }

  inline virtual ~PolicyMock() noexcept { std::puts("DTOR"); };

  MOCK_METHOD(void, debug, (std::string_view msg_received), (const override));
  MOCK_METHOD(void, info, (std::string_view msg_received), (const override));
  MOCK_METHOD(void, warn, (std::string_view msg_received), (const override));
  MOCK_METHOD(void, error, (std::string_view msg_received), (const override));
  MOCK_METHOD(void, critical, (std::string_view msg_received),
              (const override));
};

template <class Policy> class TestAnyPolicy : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}

  Policy policy_;
};

template <class LogPolicy>
class TestAnyLoggerForPolicy : public TestAnyPolicy<LogPolicy> {
protected:
  TestAnyLoggerForPolicy() : TestAnyPolicy<LogPolicy>(), log_(this->policy_) {}

  void SetUp() override {}
  void TearDown() override {}
  AnyLogger log_;
};

using ValidPolicyTypes =
    ::testing::Types<StaticFunction, MixedFunction, PolicyMock>;
TYPED_TEST_SUITE(TestAnyPolicy, ValidPolicyTypes);
TYPED_TEST(TestAnyPolicy, AnyLogCtorCopyAndMove) {
  ASSERT_NO_THROW({
    AnyLogger log(this->policy_);
    AnyLogger copy_log(log);
    AnyLogger move_log(std::move(log));
    AnyLogger copy_assigned_log = move_log;
    AnyLogger move_assigned_log = std::move(move_log);
  });

  ASSERT_NO_THROW({
    AnyLogger shared_ptr_log(std::make_shared<TypeParam>(TypeParam{}));
    AnyLogger ref_wrap_log(std::ref(this->policy_));
    AnyLogger cref_wrap_log(std::cref(this->policy_));
  });
}

constexpr std::string_view simple_msg = "Salut !";
using TestAnyLogger = TestAnyLoggerForPolicy<::testing::StrictMock<PolicyMock>>;
TEST_F(TestAnyLogger, SimpleCall) {
  {
    ::testing::InSequence s;
    EXPECT_CALL(this->policy_, debug(simple_msg))
        .Times(1)
        .WillOnce(::testing::Return());

    EXPECT_CALL(this->policy_, info(simple_msg))
        .Times(1)
        .WillOnce(::testing::Return());

    EXPECT_CALL(this->policy_, warn(simple_msg))
        .Times(1)
        .WillOnce(::testing::Return());

    EXPECT_CALL(this->policy_, error(simple_msg))
        .Times(1)
        .WillOnce(::testing::Return());

    EXPECT_CALL(this->policy_, critical(simple_msg))
        .Times(1)
        .WillOnce(::testing::Return());
  }

  ASSERT_NO_THROW({
    this->log_.debug(simple_msg);
    this->log_.info(simple_msg);
    this->log_.warn(simple_msg);
    this->log_.error(simple_msg);
    this->log_.critical(simple_msg);
  });
}

#define ASSERT_THROW_AND_TEST_EXCEPTION(statement, expected_exception,         \
                                        catch_statement)                       \
  do {                                                                         \
    try {                                                                      \
      statement;                                                               \
      FAIL() << "Didn't throw any exception";                                  \
    } catch (const expected_exception &received_exception) {                   \
      catch_statement                                                          \
    } catch (...) {                                                            \
      FAIL() << "Didn't throw the expected exception type";                    \
    }                                                                          \
  } while (0)

TEST_F(TestAnyLogger, ForwardException) {
  const std::runtime_error err_thrown("HELP !");
  {
    ::testing::InSequence s;
    EXPECT_CALL(this->policy_, debug(simple_msg))
        .Times(1)
        .WillOnce(::testing::Throw(err_thrown));

    EXPECT_CALL(this->policy_, info(simple_msg))
        .Times(1)
        .WillOnce(::testing::Throw(err_thrown));

    EXPECT_CALL(this->policy_, warn(simple_msg))
        .Times(1)
        .WillOnce(::testing::Throw(err_thrown));

    EXPECT_CALL(this->policy_, error(simple_msg))
        .Times(1)
        .WillOnce(::testing::Throw(err_thrown));

    EXPECT_CALL(this->policy_, critical(simple_msg))
        .Times(1)
        .WillOnce(::testing::Throw(err_thrown));
  }

  ASSERT_THROW_AND_TEST_EXCEPTION(
      this->log_.debug(simple_msg), std::runtime_error,
      ASSERT_STREQ(received_exception.what(), err_thrown.what()););

  ASSERT_THROW_AND_TEST_EXCEPTION(
      this->log_.info(simple_msg), std::runtime_error,
      ASSERT_STREQ(received_exception.what(), err_thrown.what()););

  ASSERT_THROW_AND_TEST_EXCEPTION(
      this->log_.warn(simple_msg), std::runtime_error,
      ASSERT_STREQ(received_exception.what(), err_thrown.what()););

  ASSERT_THROW_AND_TEST_EXCEPTION(
      this->log_.error(simple_msg), std::runtime_error,
      ASSERT_STREQ(received_exception.what(), err_thrown.what()););

  ASSERT_THROW_AND_TEST_EXCEPTION(
      this->log_.critical(simple_msg), std::runtime_error,
      ASSERT_STREQ(received_exception.what(), err_thrown.what()););
}

} // namespace
} // namespace arthoolbox::log

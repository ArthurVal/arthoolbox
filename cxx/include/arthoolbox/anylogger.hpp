#pragma once

#include <memory>
#include <string_view>

namespace arthoolbox {
namespace log {

struct AnyLogger {
private:
  struct LogConcept {
    virtual ~LogConcept() noexcept = default;

    virtual constexpr std::unique_ptr<LogConcept> clone() const = 0;

    virtual constexpr void debug(std::string_view) const = 0;
    virtual constexpr void info(std::string_view) const = 0;
    virtual constexpr void warn(std::string_view) const = 0;
    virtual constexpr void error(std::string_view) const = 0;
    virtual constexpr void fatal(std::string_view) const = 0;
  };

  template <class LogPolicy> struct LogWrapper final : LogConcept {
    template <class U>
    constexpr LogWrapper(U &&arg) : m_policy(std::forward<U>(arg)) {}

    constexpr std::unique_ptr<LogConcept> clone() const override {
      return std::make_unique<LogWrapper>(*this);
    }

    constexpr void debug(std::string_view msg) const override {
      m_policy.debug(msg);
    }

    constexpr void info(std::string_view msg) const override {
      m_policy.info(msg);
    }

    constexpr void warn(std::string_view msg) const override {
      m_policy.warn(msg);
    }

    constexpr void error(std::string_view msg) const override {
      m_policy.error(msg);
    }

    constexpr void fatal(std::string_view msg) const override {
      m_policy.fatal(msg);
    }

    LogPolicy m_policy;
  };

  std::unique_ptr<LogConcept> m_wrapper_ptr_;

public:
  template <class LogPolicy>
  constexpr AnyLogger(LogPolicy &&log_policy)
      : m_wrapper_ptr_{
            new LogWrapper<LogPolicy>(std::forward<LogPolicy>(log_policy))} {}

  inline void debug(std::string_view msg) const { m_wrapper_ptr_->debug(msg); }
  inline void info(std::string_view msg) const { m_wrapper_ptr_->info(msg); }
  inline void warn(std::string_view msg) const { m_wrapper_ptr_->warn(msg); }
  inline void error(std::string_view msg) const { m_wrapper_ptr_->error(msg); }
  inline void fatal(std::string_view msg) const { m_wrapper_ptr_->fatal(msg); }

  AnyLogger() = delete;
  explicit AnyLogger(const AnyLogger &other)
      : m_wrapper_ptr_(other.m_wrapper_ptr_->clone()) {}

  AnyLogger &operator=(const AnyLogger &other) {
    m_wrapper_ptr_ = other.m_wrapper_ptr_->clone();
    return *this;
  };

  explicit AnyLogger(AnyLogger &&other)
      : m_wrapper_ptr_(std::move(other.m_wrapper_ptr_)) {}

  AnyLogger &operator=(AnyLogger &&other) {
    m_wrapper_ptr_ = std::move(other.m_wrapper_ptr_);
    return *this;
  };

  virtual ~AnyLogger() noexcept = default;
};

// EXAMPLE:
// struct NoLogs {
//     static constexpr void debug(std::string_view) {}
//     static constexpr void info(std::string_view) {}
//     static constexpr void warn(std::string_view) {}
//     static constexpr void error(std::string_view) {}
//     static constexpr void fatal(std::string_view) {}
// };

// struct FmtLogs {
//     std::string_view fmt_str;

//     void debug(std::string_view msg) const {
//         fmt::print(fmt_str, fmt::arg("msg", msg), fmt::arg("level",
//         "DEBUG"));
//     }
//     void info(std::string_view msg) const {
//         fmt::print(fmt_str, fmt::arg("msg", msg), fmt::arg("level", "INFO"));
//     }
//     void warn(std::string_view msg) const {
//         fmt::print(fmt_str, fmt::arg("msg", msg), fmt::arg("level", "WARN"));
//     }
//     void error(std::string_view msg) const {
//         fmt::print(fmt_str, fmt::arg("msg", msg), fmt::arg("level",
//         "ERROR"));
//     }
//     void fatal(std::string_view msg) const {
//         fmt::print(fmt_str, fmt::arg("msg", msg), fmt::arg("level",
//         "FATAL"));
//     }
// };

// struct CoutLogs {
//     static void debug(std::string_view msg) {
//         std::cout << "Youhou: DEBUG: " << msg << '\n';
//     }
//     static void info(std::string_view msg) {
//         std::cout << "Youhou: INFO: " << msg << '\n';
//     }
//     static void warn(std::string_view msg) {
//         std::cout << "Youhou: WARN: " << msg << '\n';
//     }
//     static void error(std::string_view msg) {
//         std::cout << "Youhou: ERROR: " << msg << '\n';
//     }
//     static void fatal(std::string_view msg) {
//         std::cout << "Youhou: FATAL: " << msg << '\n';
//     }
// };

// struct OstreamLogs {
//     std::ostream &os;

//     void debug(std::string_view msg) const {
//         os << "Stream: DEBUG: " << msg << '\n';
//     }
//     void info(std::string_view msg) const {
//         os << "Stream: INFO: " << msg << '\n';
//     }
//     void warn(std::string_view msg) const {
//         os << "Stream: WARN: " << msg << '\n';
//     }
//     void error(std::string_view msg) const {
//         os << "Stream: ERROR: " << msg << '\n';
//     }
//     void fatal(std::string_view msg) const {
//         os << "Stream: FATAL: " << msg << '\n';
//     }
// };

// constexpr std::string_view bar = "===============================";

// int main(int argc, char *argv[]) {
//     fmt::print("{}\n", bar);

//     if constexpr (false) {
//         AnyLogger log(CoutLogs{});
//         log.debug("CHO");
//         log.info("CO");
//         log.warn("LA");
//         log.error("TI");
//         log.fatal("NE");

//     } else {
//         std::vector<AnyLogger> all_logs;

//         all_logs.emplace_back(FmtLogs{"[{level}]: {msg}\n"});
//         all_logs.emplace_back(FmtLogs{"<{level}>: \"{msg}\"\n"});
//         all_logs.emplace_back(CoutLogs{});
//         all_logs.emplace_back(NoLogs{});
//         all_logs.emplace_back(OstreamLogs{std::cout});
//         all_logs.emplace_back(OstreamLogs{std::cerr});

//         std::stringstream strstream;
//         all_logs.emplace_back(OstreamLogs{strstream});

//         for (const auto &log : all_logs) {
//             log.debug("CHO");
//             log.info("CO");
//             log.warn("LA");
//             log.error("TI");
//             log.fatal("NE");
//             fmt::print("{}\n", bar);
//         }

//         fmt::print("Inside strstream:\n{}", strstream.str());
//     }

//     fmt::print("{}\n", bar);
//     return 0;
// }

} // namespace log
} // namespace arthoolbox

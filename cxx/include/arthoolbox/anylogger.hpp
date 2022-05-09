#pragma once

#include <memory>               // unique_ptr
#include <functional>           // std::invoke, std::is_invocable
#include <string_view>

namespace arthoolbox {
namespace log {

#define CREATE_LOG_TRAITS_FOR_FUNCTION(FN_NAME)                           \
    template <class T, class = void>                                      \
    struct FN_NAME##_traits_impl {                                        \
        static constexpr bool fn_is_defined = false;                      \
        using fn_type = void;                                             \
    };                                                                    \
                                                                          \
    template <class T>                                                    \
    struct FN_NAME##_traits_impl<T, std::void_t<decltype(&T::FN_NAME)>> { \
        static constexpr bool fn_is_defined = true;                       \
        using fn_type = decltype(&T::FN_NAME);                            \
    }

#define CREATE_LOG_WITH_LEVEL(FN_NAME)                                         \
   private:                                                                    \
    CREATE_LOG_TRAITS_FOR_FUNCTION(FN_NAME);                                   \
                                                                               \
    using FN_NAME##_traits = FN_NAME##_traits_impl<raw_policy_t>;              \
                                                                               \
    static_assert(FN_NAME##_traits::fn_is_defined,                             \
                  "The given LogPolicy doesn't define the function: " #FN_NAME \
                  "(std::string_view).");                                      \
                                                                               \
   public:                                                                     \
    void FN_NAME(std::string_view msg) const override {                        \
        using fn_type = typename FN_NAME##_traits::fn_type;                    \
        if constexpr (std::is_invocable_v<fn_type, LogPolicy,                  \
                                          decltype(msg)>) {                    \
            std::invoke(&raw_policy_t::FN_NAME, m_policy, msg);                \
        } else {                                                               \
            std::invoke(&raw_policy_t::FN_NAME, msg);                          \
        }                                                                      \
    }

namespace _meta {

template <class, template <class...> class...>
struct remove_outer_type_if_in;

template <class T, template <class...> class U>
struct remove_outer_type_if_in<T, U> {
    using type = T;
};

template <template <class, class...> class T, class U>
struct remove_outer_type_if_in<T<U>, T> {
    using type = U;
};

template <class T, template <class...> class U, template <class...> class... V>
struct remove_outer_type_if_in<T, U, V...> {
    using type = typename remove_outer_type_if_in<T, V...>::type;
};

template <template <class, class...> class T, class U,
          template <class, class...> class... V>
struct remove_outer_type_if_in<T<U>, T, V...> {
    using type = U;
};
}  // namespace _meta

template <class T, template <class...> class... U>
using remove_outer_type_if_in_t =
    typename _meta::remove_outer_type_if_in<T, U...>::type;

// Type Erasure implement of ANYLOGGER
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

    template <class LogPolicy>
    struct LogWrapper final : LogConcept {
        template <class U>
        constexpr LogWrapper(U &&arg) : m_policy(std::forward<U>(arg)) {}

        std::unique_ptr<LogConcept> clone() const override {
            return std::make_unique<LogWrapper>(*this);
        }

        using raw_policy_t = remove_outer_type_if_in_t<
            std::remove_pointer_t<std::remove_reference_t<LogPolicy>>,
            std::reference_wrapper, std::unique_ptr, std::shared_ptr>;

        CREATE_LOG_WITH_LEVEL(debug)
        CREATE_LOG_WITH_LEVEL(info)
        CREATE_LOG_WITH_LEVEL(warn)
        CREATE_LOG_WITH_LEVEL(error)
        CREATE_LOG_WITH_LEVEL(fatal)

        LogPolicy m_policy;
    };

    std::unique_ptr<LogConcept> m_wrapper_ptr_;

   public:
    template <class LogPolicy>
    constexpr AnyLogger(LogPolicy &&log_policy)
        : m_wrapper_ptr_{
              new LogWrapper<LogPolicy>(std::forward<LogPolicy>(log_policy))} {}

    inline void debug(std::string_view msg) const {
        m_wrapper_ptr_->debug(msg);
    }
    inline void info(std::string_view msg) const { m_wrapper_ptr_->info(msg); }
    inline void warn(std::string_view msg) const { m_wrapper_ptr_->warn(msg); }
    inline void error(std::string_view msg) const {
        m_wrapper_ptr_->error(msg);
    }
    inline void fatal(std::string_view msg) const {
        m_wrapper_ptr_->fatal(msg);
    }

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

#undef CREATE_LOG_WITH_LEVEL
#undef CREATE_LOG_TRAITS_FOR_FUNCTION

// EXAMPLES
// struct NoLogs {
//     static constexpr void debug(std::string_view) {}
//     static constexpr void info(std::string_view) {}
//     static constexpr void warn(std::string_view) {}
//     static constexpr void error(std::string_view) {}
//     static constexpr void fatal(std::string_view) {}
// };
// 
// struct FmtLogs {
//     std::string_view fmt_str;

//     void debug(std::string_view msg) const {
//         fmt::print(fmt_str, fmt::arg("msg", msg), fmt::arg("level", "DEBUG"));
//     }
//     void info(std::string_view msg) const {
//         fmt::print(fmt_str, fmt::arg("msg", msg), fmt::arg("level", "INFO"));
//     }
//     void warn(std::string_view msg) const {
//         fmt::print(fmt_str, fmt::arg("msg", msg), fmt::arg("level", "WARN"));
//     }
//     void error(std::string_view msg) const {
//         fmt::print(fmt_str, fmt::arg("msg", msg), fmt::arg("level", "ERROR"));
//     }
//     void fatal(std::string_view msg) const {
//         fmt::print(fmt_str, fmt::arg("msg", msg), fmt::arg("level", "FATAL"));
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

// struct Toto {};

// int main(int argc, char *argv[]) {

//     constexpr std::string_view bar = "===============================";

//     fmt::print("{}\n", bar);

//     std::vector<AnyLogger> all_logs;

//     auto log_feature = FmtLogs{"<{level}>: \"{msg}\"\n"};
//     all_logs.emplace_back(FmtLogs{"[{level}]: {msg}\n"});
//     all_logs.emplace_back(log_feature);
//     all_logs.emplace_back(std::ref(log_feature));
//     all_logs.emplace_back(std::cref(log_feature));
//     all_logs.emplace_back(&log_feature);
//     all_logs.emplace_back(std::make_shared<FmtLogs>(log_feature));
//     all_logs.emplace_back(CoutLogs{});
//     all_logs.emplace_back(NoLogs{});
//     all_logs.emplace_back(OstreamLogs{std::cout});
//     all_logs.emplace_back(OstreamLogs{std::cerr});

//     std::stringstream strstream;
//     all_logs.emplace_back(OstreamLogs{strstream});

//     for (const auto &log : all_logs) {
//         log.debug("CHO");
//         log.info("CO");
//         log.warn("LA");
//         log.error("TI");
//         log.fatal("NE");
//         fmt::print("{}\n", bar);
//     }

//     fmt::print("Inside strstream:\n{}", strstream.str());

//     fmt::print("{}\n", bar);
//     return 0;
// }

} // namespace log
} // namespace arthoolbox

#pragma once

#include <functional> // std::invoke, std::is_invocable
#include <memory>     // unique_ptr
#include <string_view>

namespace arthoolbox {
namespace log {

// Metaprogrammation nightmare, please close your eyes //////////////////////
// Sanity may decrease abnormally the longuer you try to understand/work on this
// s***. I don't even know why I keep doing this... I must be a masochist.

/** /brief Contains metaprogrammation functions */
namespace _meta {

// Goal: Given a templated type T<U> and a list of templated type Vn (V1, V2,
// V3,...). If T is included inside Vn, we set ::value to  true. Otherwise
// ::value is false.
// Example:
// - is_one_of<int,
//             std::shared_ptr, std::vector>::value == false;
// - is_one_of<std::vector<int>,
//             std::shared_ptr, std::vector>::value == true;
// - is_one_of<std::vector<int>,
//             std::shared_ptr, std::unique_ptr>::value == false;
template <class, template <class...> class...>
struct is_one_of : std::false_type {};

template <template <class...> class T, class... U,
          template <class...> class... Other>
struct is_one_of<T<U...>, T, Other...> : std::true_type {};

template <template <class...> class T, class... U,
          template <class...> class Different,
          template <class...> class... Other>
struct is_one_of<T<U...>, Different, Other...> : is_one_of<T<U...>, Other...> {
};

// Declare a trait `template<class T> NAME_funtion_traits;` use to check if the
// functions required by the log policy exists and extract compile time
// informations on it.
//
// This traits contains:
// - ::value (bool): Indicates if T::NAME expression exists and it's a function
//                   invocable as of T::NAME(std::string_view);
//
// If `value` is set to true, it also define the following:
// - ::function_type (type): decltype(&T::NAME), the function type;
// - ::is_member_function (bool): true if function_type is a non static member
//   function, false otherwise
//
// Example:
//
// struct Toto {
//     void debug(std::string_view) const;
// };
//
// struct Titi {
//     static void debug(std::string_view);
// };
//
// struct Tata {
//     static void debug(int);
// };
//
// struct Tete {};
//
// static_assert(not debug_function_traits<Tete>::value);
// static_assert(not debug_function_traits<Tata>::value);
//
// static_assert(debug_function_traits<Toto>::value);
// static_assert(debug_function_traits<Toto>::is_member_function);
// static_assert(std::is_same_v<debug_function_traits<Toto>::function_type,
//                              decltype(&Toto::debug)>);
//
// static_assert(debug_function_traits<Titi>::value);
// static_assert(not debug_function_traits<Titi>::is_member_function);
#define ANYLOG_CREATE_FUNCTION_TRAITS(NAME)                                    \
  template <class T, class = void>                                             \
  struct NAME##_function_traits : std::false_type {};                          \
                                                                               \
  template <class T>                                                           \
  struct NAME##_function_traits<                                               \
      T, std::void_t<decltype(std::declval<T>().NAME(std::string_view{}))>>    \
      : std::true_type {                                                       \
    using function_type = decltype(&T::NAME);                                  \
    static constexpr bool is_member_function =                                 \
        std::is_member_function_pointer_v<function_type>;                      \
  }

ANYLOG_CREATE_FUNCTION_TRAITS(debug);
ANYLOG_CREATE_FUNCTION_TRAITS(info);
ANYLOG_CREATE_FUNCTION_TRAITS(warn);
ANYLOG_CREATE_FUNCTION_TRAITS(error);
ANYLOG_CREATE_FUNCTION_TRAITS(critical);

#undef ANYLOG_CREATE_FUNCTION_TRAITS

} // namespace _meta

/**
 *  \brief Traits of a given LogPolicy
 *
 *  This trait returns the following information:
 * - ::policy_type (type): The LogPolicy type removed from CV, reference and
 *                         pointer qualifier;
 * - ::debug_fn (custom traits): Trait that indicate if the
 * debug(std::string_view) is defined and, if it's the case, addictional
 * informations on the function type and if it's a static member function or
 * not;
 * - ::info_fn: Same as of debug_fn but for the info() function;
 * - ::warn_fn: Same as of debug_fn but for the warn() function;
 * - ::error_fn: Same as of debug_fn but for the error() function;
 * - ::critical_fn: Same as of debug_fn but for the critical() function;
 *
 *  \tparam T The LogPolicy
 */
template <class T, class = void> struct log_policy_traits {
  using policy_type =
      std::remove_pointer_t<std::remove_reference_t<std::remove_cv_t<T>>>;
  using debug_fn = typename _meta::debug_function_traits<policy_type>;
  using info_fn = typename _meta::info_function_traits<policy_type>;
  using warn_fn = typename _meta::warn_function_traits<policy_type>;
  using error_fn = typename _meta::error_function_traits<policy_type>;
  using critical_fn = typename _meta::critical_function_traits<policy_type>;
};

/**
 *  \brief Specialization of log_policy_traits for std::reference_wrapper,
 * std::unique_ptr and std::shared_ptr types
 *
 *  \tparam T The LogPolicy
 */
template <template <class, class... V> class T, class U, class... V>
struct log_policy_traits<
    T<U, V...>,
    std::enable_if_t<_meta::is_one_of<T<U, V...>, std::reference_wrapper,
                                      std::shared_ptr>::value>>
    : log_policy_traits<U> {};

/**
 *  \brief The NoLogs Policy, for people that prefer the silence
 */
struct NoLogs {
  inline static constexpr void debug(std::string_view) {}
  inline static constexpr void info(std::string_view) {}
  inline static constexpr void warn(std::string_view) {}
  inline static constexpr void error(std::string_view) {}
  inline static constexpr void critical(std::string_view) {}
};

/**
 *  \brief Adapter that wrapper ANY struct that define ALL logging functions
 *
 *  This class implements generic wrapper (using Type Erasure ) of a logging
 *  facility.
 *
 *  It accepts ANY struct T that declares the following interfaces :
 *  - &T::debug(string_view)
 *  - &T::info(string_view)
 *  - &T::warn(string_view)
 *  - &T::error(string_view)
 *  - &T::critical(string_view)
 *  And any call to the debug/info/warn/error/critical member function of the
 *  AnyLogger will be forwarded to &T::FUNCTION accordingly.
 *
 * It currently accepts shared_ptr, unique_ptr and reference_wrapper of T.
 */
struct AnyLogger {
private:
  /**
   *  \brief Internal Concept declaring the INTERFACE of all wrapped loggers
   */
  struct LogConcept {
    virtual ~LogConcept() noexcept = default;

    virtual std::unique_ptr<LogConcept> clone() const = 0;
    virtual void debug(std::string_view) const = 0;
    virtual void info(std::string_view) const = 0;
    virtual void warn(std::string_view) const = 0;
    virtual void error(std::string_view) const = 0;
    virtual void critical(std::string_view) const = 0;
  };

  /**
   *  \brief Wrapper of ANY logger LogPolicy
   *
   *  This wrapper can take ANY LogPolicy and apply it when calling the
   *  debug/info/warn/error/critical functions.

   *  The LogPolicy needs the following to be accepted (FN_NAME =
   *  debug/info/warn/error/critical):
   *  - decltype(std::declval<LogPolicy>().FN_NAME(std::string_view{})) MUST BE
   a valid expression;
   *
   *  \tparam LogPolicy The logging policy used by this wrapper
   */
  template <class LogPolicy> struct LogWrapper final : LogConcept {
    using policy_traits = log_policy_traits<LogPolicy>;
    /**
     *  \brief Construct a LogWrapper given any LogPolicy rvalue
     */
    template <class U>
    constexpr LogWrapper(U &&arg) : m_policy(std::forward<U>(arg)) {}

    /**
     *  \brief Clone any LogWrapper by copying itself
     *  \return std::unique_ptr<LogConcept>(*this)
     */
    std::unique_ptr<LogConcept> clone() const override {
      return std::make_unique<LogWrapper>(*this);
    }

// Declare and implement the FN_NAME(std::string_view msg), inside LogWrapper,
// which will call the LogPolicy::FN_NAME(msg), based on if it's a static
// functions of class functions
#define ANYLOG_CREATE_LOG_WITH_LEVEL(NAME)                                     \
  static_assert(policy_traits::NAME##_fn::value,                               \
                "The given LogPolicy doesn't define the function: " #NAME      \
                "(std::string_view).");                                        \
  void NAME(std::string_view msg) const override {                             \
    using policy_t = typename policy_traits::policy_type;                      \
    if constexpr (policy_traits::NAME##_fn::is_member_function) {              \
      std::invoke(&policy_t::NAME, m_policy, msg);                             \
    } else {                                                                   \
      std::invoke(&policy_t::NAME, msg);                                       \
    }                                                                          \
  }

    ANYLOG_CREATE_LOG_WITH_LEVEL(debug)
    ANYLOG_CREATE_LOG_WITH_LEVEL(info)
    ANYLOG_CREATE_LOG_WITH_LEVEL(warn)
    ANYLOG_CREATE_LOG_WITH_LEVEL(error)
    ANYLOG_CREATE_LOG_WITH_LEVEL(critical)

#undef ANYLOG_CREATE_LOG_WITH_LEVEL

    LogPolicy m_policy; /*!< The logger/logpolicy being wrapped */
  };

  std::unique_ptr<LogConcept>
      m_wrapper_ptr_; /*!< Unique PTR of the log wrapped by the AnyLogger */

public:
  /**
   *  \brief AnyLogger ctor factory.
   *
   *  \tparam LogPolicy The Logging policy wrapped
   *  \param[in] log_policy rvalue forwarded to the LogWrapper
   */
  template <class LogPolicy>
  inline constexpr AnyLogger(LogPolicy &&log_policy)
      : m_wrapper_ptr_{
            new LogWrapper<LogPolicy>(std::forward<LogPolicy>(log_policy))} {}

  inline void debug(std::string_view msg) const { m_wrapper_ptr_->debug(msg); }
  inline void info(std::string_view msg) const { m_wrapper_ptr_->info(msg); }
  inline void warn(std::string_view msg) const { m_wrapper_ptr_->warn(msg); }
  inline void error(std::string_view msg) const { m_wrapper_ptr_->error(msg); }
  inline void critical(std::string_view msg) const {
    m_wrapper_ptr_->critical(msg);
  }

  inline AnyLogger() : AnyLogger(NoLogs{}){};

  inline explicit AnyLogger(const AnyLogger &other)
      : m_wrapper_ptr_(other.m_wrapper_ptr_->clone()) {}

  inline AnyLogger &operator=(const AnyLogger &other) {
    m_wrapper_ptr_ = other.m_wrapper_ptr_->clone();
    return *this;
  };

  inline explicit AnyLogger(AnyLogger &&other)
      : m_wrapper_ptr_(std::move(other.m_wrapper_ptr_)) {}

  inline AnyLogger &operator=(AnyLogger &&other) {
    m_wrapper_ptr_ = std::move(other.m_wrapper_ptr_);
    return *this;
  };

  virtual ~AnyLogger() noexcept = default;
};

/////////////////////////////////////////////////////////////////////////////
//                                 EXAMPLE                                 //
/////////////////////////////////////////////////////////////////////////////
/*
LIVE DEMO HERE :

https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QXACx8BBAKoBnTAAUAHpwAMvAFYTStJg1DIApACYAQuYukl9ZATwDKjdAGFUtAK4sGIM1ykrgAyeAyYAHI%2BAEaYxBLSAA6oCoRODB7evv6ByamOAqHhUSyx8VK2mPYFDEIETMQEmT5%2BAZXV6XUNBEWRMXEJtvWNzdltCsM9YX2lA1IAlLaoXsTI7BzmAMxhyN5YANQmm25UXgwO6WJH2CYaAIJbO3uYh8dsLCQAnte3D2bb52erzcE2IYWAAH0AG54TAAdx%2B91%2BAHpkfsALKYeqJYioYDEVgsJg1fYMPDABAEInETCkfaJehMJT7XYpF6fZbEfaYT6YBT7VGCoXCkWolFooSGQiffZEmVYZA0pkvJjRBgkIm0WgyggIF60ATALxxfYcrz7AjEHWofZnLDECaGdDIuEkADW%2BwEFoQeAU4v2CgAVMGAHT7ACS%2B3QAjAHAI3KhjH2bvVcP2cIQMsjbswmESUdQ4O9vpDpYjsq8E32sX2TFlTNQyB9ExDvxRwYF0TBmCo%2Bw8giYYX5bGxuPxhOJ6X2p3ONX5geRv2YbAUiSYa32EJHdZMAHYrEj7qj9gBxVBiECnvCJhi1i2YFgM4mYdAWz6JF4AFSObjk11rDCvnWtC%2BvGqC9gQD5PpBr4EO%2BLwAGq3hACGBPsCFmKQ/oIZspCliG8xhuGvafvsvpkYCXhYK%2BQ54AcSF0nCLxKPGIAgFCYjGhaNoWsQxphgA8rqcRwr6mD%2BmxHHeC85FUGISitkeaLYKorAMuw/oALRkQoEICJgelUD%2BYQEFhSn7BZllWZZEzoGxCgIA0L4Qoklp0rZkmYA4JDXJJnEvEcAAiQXTvJ4mbAedzHtpvp6eEhk/h57FeUQxDGYI1xmVFaLWblAYEHZIAOU56AuW5%2BWFYm3lpZs2B%2BdJrzBZsgW8caRyRdFOlxQZ4GJQVnnVelBCZf6eXWUlxU0qVrlkBVbFnHgACOxplTVdXsf5jUhXJtBKO1vyQY%2BBiQcCuxMgodKHdBAXHGdCgKPh/53Q9paIncoJeA4XX6YZ%2ByXklO1KBCcEfoc%2B57k1kUHVBx03W4V2w6dBj3Y9tUssj/KfnSz34fschZWNFkI8%2BSPnaj2Do2TZZCXqa2/B9X2xT9vXHN%2BxxyOTdJY/sNNxOTf1zSAlorSDAXg7ukNtvcxMnT%2BMtwzjr1o89%2Bzc4rYb478hP3kdJM/urT0Y/sgV4FQVBxIwpla4T8ukyjSsU%2BrPPCXT9wM/GTPxSzbhs7%2BnPG6b5s0oIdK88Q/OXp7PVGazP4cw7ocu/ze5QxL%2B2HtlxteQYNJ3pag7xiYABsGjy/rRtsxTER3Oi2AQjONTAwShB%2BhFxcaLazJECyerIB6pveuJ5kzhcAj8jSy14FN1Y6nq%2BwGsA9KeHgyAypgqigfyTrcqo%2BdfWgj50C8jhsP6YT8MQRJzp6t6EIp2X%2Bp%2Bza8QX/JoAOQ4gFpf0bQ1EDRKgTw8xLzhkAivZ8/IB6fjYtXWuO8cR8lSF6dem8AI0QILGLe04zijwYKNbWeUwhQkbKqegtZ%2BTgVVjAmu2AIATUtOCaEsI4TzHTg/cyxFDglykm1EuOkAxYm4q1WkZF4zyRtFgGg4RB7Tk8AaUSRgv7mW0mxEejdRb7AgKLYBUZs6iwgOYIu0CQCwOwIsGRaipyizYZ1NisV3hlHrjgkk/9AG0B0cLGSvZLECGBvBfhdZ1S3kdI4ZAsoHxlHwT4hgdJAYvFQC7USe0lL%2BhUmpcQ4p/Tu1Vgkm0Kd8EWWIXRXR0QvDADof1IqDCjBMPhPMFkY9hpt3MhDGxi5zLZM/GkMGHUco2XqKE/YRTXxYFKeU%2BhYIakwjqW0sGksUkdOFl9T8xIdzg3MlZEJK8hmFhGZgMZEATKsOaZnVpJyxSLL4ssrEYsrBp3Oe0zOWzkAQnOnEAgEB1TxlGWUpxs50hN1fj%2BT8NzfK/2NMc3p%2BVJwvLeY0T5CSSm/OiYCluwLVlgp4ZgSFmTFkwtefdd5EAfmQhRfnNFrNcmYv8jivFoSCVKHhSSv5uDUUEFbj7KltU7G6QcXEFlNRaVPIGSvBlRKkqxQUKwAyUIfzMrJc3dlwKuXrTJfBLKx4CFatygqWg%2BjDGflyWxElxyzFtKySK2FhKmX7ORc4gF5KlWszSNS6SQrjzPLFfCr5SLSX2t8Y6jlXTHBgvsRE/l0ShVbEkdMfYdwIgAE1ggCRPBCNwAAlbAdxPx1wAGJyAiD7cMAkIgQk/Omu44ZPxCAgKY%2Bp2qG1jRMAAVjcNbW25dzqq2xkbEKwz/yNsHdqltbb7gWWyaYrYWwBUOsVfyf6lS4l%2BNBvk%2B5Fgh2ExHdbDdO7d27q3WOnW107aYwHXu89DaD13HHUs%2BMk7/jToVUC44F7X1XtytzJKwzgZyr0fBCphVdVSWBdcCAhFTEAfstUyE0yETi0CvMU1Z7Ditu3ZZBdhVPHLtua%2B3DuV32WUrEWNVK7mq6N2PqswRjqG10hXhptqHD03phQ0hgEx144mrG4rqfLiAzq9CFej1kCO5Qlby8NfGUXJBMvy2VxwSPhRuBFc9BHWkZ3jUmlNabM3ZrzQWotJay0VqrTWk1bCNPJtTRmrNOaIT5sLZ%2BYtpby2VurYchg/AhUWa09Z3Tdn9OOcMy5kzEA4QNAYF5xNlntM2b0w5pzRnXM1riLiYgkXNNWZ07Z%2BzBnnPGbc4qNIoh3FsLbP8O0PY41RZ81luLuXEsmalg8CWAo0TLj5GuDcW4sRMCa8iYMWtAwWRHV2WEJE52el7HWYA14kzBDxMoZeq9BuDYsk/cijr9g0gICsNjFi5GoAUYvc%2BGpJzOFWyokAyQQKr2w5o7Rl4n4vHm8ARbN2dT%2BJpB8RMr4qC4hYH2BCdIaRB0YBuJ0q3X3ScECaZaYhTawhqpFfYQ3LvyuQsgSsRAAeBp0Z%2BRV3piQUXwKIE6A9hKrZJZBqpkyYPMPqeRGN4QgKATpAPQgmCZGiCUHSJg6ASc1CuGOobJ3L5nb216YS2D/mS/8dvdnGCOBYOeeE2YXJomejSsL0kCS2Eo/2Jdk79c/D7ElGwchk3fXG%2BrF4eMF8ZEnbA9L3BevUc/zC8QBgxvLxm5VBQ3s6Obd25IDIj3DAnfRNdwbn%2BKWSDe9N9Ki3lDA%2BlOD1yKXsfiAR/9Xgk5%2BvLuFdCWIePvuk8B9taS28qfZHp7noX8BtBs8y9z8jwMkOR0EDXASAHpEnv7Be29le3xhePI7bdCuPau19t2f%2BbJC8XJLc%2BGy/k%2BTD1EaMEvd7d2gpoZvYVL7qBEwL5k3xppIJKkH6PyDi25wDJn6SpfgyyAoQ/tjrVH47%2B8%2BdyLIHkKot2ubjbjGoV78Ze6Bo/jXZD7YYf7I7f4b5G5UC3h/7wQAHdb1B2IeaoCgHL4QGL7QGf6wHr6Lxh7W7IEfioFAEgAkFPoUpuCQG3bWIEHWxEHcjECpakFkb/6J5oFMBsSZ7YHgHHD0FL6MFKaEGpAb717FYcEtRcHm48FsRSHF40FOp0F4GiGlZrp9YDba4oZuCjaVZCAfjIB4Dw4ABe4ulu8%2Bwhy%2BNegs1%2BwcawEIcIBIiQH4ZAq2SUC0y0BkM0aCgsk0zkfhosfowu7eraneDQrAqsqsc8A%2Bi%2Bg2o%2BMMesxwY%2Bbgz0k%2B9sYYCEhsXaasRs%2BMlMWR6Eb0c%2BeIC%2BW%2Bgho616lkvshRCEnMaGSUjApCBkpsr%2BbgChIAUcCUb%2Bv4dIDRCc9hPYN%2BThLhTAbhcQBMQmhME0jkU0q0rqbUn%2BNwTGAs1h6hc6cc/4q68yDwR4Oh16Q2ehBhJEc8EQqAL2/Ig%2Bq8sSIeH4qA6khO8YCCQcMiqQ9At%2BiR9Mt6%2BwFxVxPS1sYQIE0iKu787GqgnGwyvq1OoIjCsG9SuxQJDAIJzElqrGEJUJuyFE/AsJ0GtSLCgJach6wJsaYJjSHGXI0JYeeJtOBJiJ8GyJqJ0Kgy4JkEkJVJ2JmetJ8J9ORJwUJJKJZJ6JbJlJOyxSShjeEyvJdS/JbYWhGc/WbeuhI23YvYdw6AExkEteRO4xkxXIGm%2BUVy8YuoROTOLwdwwQwQ88eIM2G%2B0SoRRxkO62b8RseAR0D4ls/IwAjAcQ2yep7hmiLBn4/i2ABICgKwLw9SlCwEtp4IkOckJhIEcE98KOkO4YYiyAawrk/IhpnSzx5GOcfI%2B2Woh2RYJ%2BiZxZSiRxFk2kBqwBBycJUy9OkOdZVGxijuTZdOMyuhbZ1GVB4WFSdJCJrZhw7ZfBbBJAQ5MpLCo59ZKAYIReUp%2BJI5uhdwgEAEMoxWtAQiUuJKyIJ2Lo4WyImeyIkpquZQzuJIlCFOa5DAnwL2PpXIokWo1YLwF8Hu1EQi85OWgWEQtYWZJA%2BARg2oqZypNZGZLIKwwcBA2oAF2Z7KAY8xQR5U3hK0fh28DhYOBkAZJolCn4YFjy2S65D5tpJoq%2BdwOI14z41ZFkSp26xxqpY2EYMOnuYgfYAgCFhZDQRYUu4YEQOa6auadwbgFMMZr5uFr4C8T5jplkC41s2SL2/YXFFFVkMIjQXg7FAAfkpZxXmB8vUuqOvCpWRpIkwF4HqqVmsepTtuxV4WSD4YsccLpbfq5IbPpE7myY1PsBoHrpZDZZpTudCVTtKc2bKV5SFL5V/oUlPLZUFdiZ2ZUl2fSRiYXGRlFbATFRpexdSYOaFd2YSRFelX5VlXFeKa%2BNyflSlUVS1BlWhgFTldiZKTyWFYVY0t5XVYemclDIevRUxoxa2qcfsAAOquGBmUKGnSUmjxHvYMUMVrbPy4VciiC3j1A5hVYJr94LaL7%2BETEMgyiEDph6i3jbm8V6jzW%2BoHmYFHme4nmTnEBnmLkN5XnpAPRNZyULXPbbXvaki5joD8hS78CllHZCI1jrgIUviaK5qlqmJbT9UWT7mHlh53WpaPVFZiDAIXXaS6r6JJRAZXDOXfVD6gaETQ0QgQZVWwa7GIYYhyBCCkQWCrE1F3hSTFKUmILpAlXgUfV6GRHd5bWvY7V97SVFjCGdyQ3RCzzkRLUMWPJEzJGyzj5dozXE1oyKV4ijV7UmhSLsWXguUqXrLM1wGLw2GBreUbGVFbGE2C2zVMFrF9VG364nFqkcVsa/F1gvaa36n7AzY3ibkC03EyjEBYq77yVrFpEqx/jv5rGikckC1e3uEQByBjlFyGINDAD1KXgsAVFD7U4fkNDoDbG1QQDp001IkZxWQO3WQDX6Eu1uAGjSKGAyie1jUmiS0NKJCfDlnspVBGRrFO0jrba7aCxoW%2BGWg/j636WgaBi6i%2BisL91h1G32VLToXj3W3KVT3KwN2YCeXtWH4pZ0S3K75bZYjD1JREg5gQij0T0a2t1rQQAz3NhCpWRqb7GZyBTZxOT%2BHunqRsCCAyJk0U1JUrnMKygKAZ1s5saH3x130apogZgrwIDph0A7nbkyIq2rxsSAM0IQAsDgPmLRDKivhegK6c51jPL%2BgOmW4qwOllabDmkbXRa%2Ba2bRbDVVoAASEIwQ2ACE2AwQtaNC9aMx%2BGjGNRnqcKHyptc6NGNwD6/w3u4KIiwjImw6ZgZgfevtc2RNq8BYfIDAsY3yPYsagNOel45gZgY5mwsNwmojg65jLVBVhE5jdGwjm6tj5VfxODlNoDeDGdqVnoiYbB0DKcMxKjLBpt3lchLwUjr8bEpt8E7UKm7jFkA8sdnGEAMTLcMjU68jiBPK3WauoBDJ66w6yT/ShURCqAOYBiVGptMjdI2dwhDT%2BDiTQmKjEM3Iu0OGrjPTlkKje%2BGBxC1ThidTJiNCzTGdrTvTL9ZTHT0z8zMz1Rw2xJb9Vk3mmWsWXDWmbDn4nD3DvD/DZmax6zMWfmrDHDXDPDfD7mnmaGJzzDdc5zuzlzBzoW4W89Rt9ztWWzqaOzezVz/DmeHzaz1WGzZz2zFz%2Bz1zkpHzdDFW6poLpzLDELzzULwQ71lkGDMojTCRym/WYAYAP4sR%2BoZFD1C8YtsQRYklKOctcyVlYjlSo9Tlbgk9bl0djtsozhd9q0EIiT%2BLhLL6cgDlXEyg5aluUuC86Yrdr47dUuJFj5Joi9vwiQXg0QN2tFAohxPNTFlW8rpLLIqUoU1UnwqZFk3NZrw2ERXe0RWLxLAtdpJtO1klF1I61rLALaFgYQLaLUFtQ%2BW2WKNen5kN3cUuLdWtWuzNi98tusit6RRsWLb0KTQp0iaTBp95CrWetrhihivrq8wL1kWdXL4bPLqlBC4QaYYb%2BpN9NtqttCAMJAn51bgdoGubnwiGHT5dqzFEzJwVFeDjBJYDfjEV%2B4nLS1PLmk1wVOvjdGr9SbPbCVmB/bsGg79Sw7662dY7M0EIE7tUju07iTs73bsauVnuS7Pj%2BD/jITG73LW7O72ANJ%2B7ymh7pJ0i0JlVwDw557Q77VV7Rb%2Bp471w3Jj766z7ybLw0JzV3j8IK7l7htVk17xbt71wzVwH1sr9TJsaerwAT5Tul4WHOH/xeIfo8GrCjJJyGH0iHGN2h1%2BHcQEAXltHXIhiCStM%2BbVkhbm7loEIEALHfMCH/7SHtUrI4QYGxT6HgpzJjHKdjxcQxIPkzU9H7VUnzHLsYnax/H7hPL3lvH4cGn/Kgn2AwnO9z9lkQ9nuKOs9rcsB3VGLL7LwVHK8NH6bpLEAynVGKnrHx9HHN7XH1O32O9OnIYenfGW7ZdjJFdR70ibnRcMnBIqUQUrnzn2H5F7nVGOnanRtwXWnIU59%2B9PHScWXoXJVJ9O25nj9voflNnEXDVO5WljHTuRlqgJlLUZlFlTSqcexdDTwVEcMTgoImArAb0jwlEBwiU/Xg37Lw3uwPXwIVU8Xk3SI/w3Xo3ajVALABAZ5JAmAIYCA5jTW2Sua63AJpbAzNOM50463EIoI9Lal2JIVH75307sHmV1ka3BAcTkyHyb3V35Ub3bE6dNTZgvj5jEz5if3IAAP5j9AiYtAIPY5ZggU2AFgcgJ4zjJnyzApRt0JiVhUyVy7T3a7x9Fk4PVFgg1Al3oIsS63/3xA5S5jwPajoPVP73EPtPgP0PVQcP5j/FuaAkaPxXh7hS2JNJUHaYBPP7cHeUJPn35PBAP3s04PkPajDPmEK7zPNPdPajHPsPjP8Pw1dw6aEQ/P0VdL1lXJ91Z70H4vbtgJ/dlk0vJksv8v6vrPmvQP4DcP07LvSvZg2vXPaj2A6a6aAk6axvL3gvHjkHD3rVMHhPdvxP1PV2Mv33lPF3LPPvKvTPafGv7PmAMP/vZgGaVa4YbglpYfaG6HCpb92SHgtux3kvzGgyvbjZ0fBVMHJ340lSaAtuwIRL5jtfPQRHl4iPyPJ4ZjajvfL6vjk/bg%2BwsYI6%2BjGwJvEfKu2Pi7ov7fDfnfhU3faVbaL6/fywg/wACgICEQvP4/FjP4RL0/1/L68/rai/AvKzmy6JJ74eG/T3Hfr/O/R/M/8PA/K4peH16G9L%2B//W/scCJYP83AT/Zfi/36QsY32FvT/he2/7lNFCf/O/rP0P518h%2B%2BwQPsH3TRgCsBg7f/tANgHh94BjfbZBByerFZLeYvVAVvx/4YCe%2BJAnAcf1P59h00xfUvsEGIGQCp%2B4DMgRsEf7z84BmPKrtX1%2BICRxuLAevi9ySgpBLQA3AHMxys4YshexSe7rjxAZW8L2cfDlhZBSD/9zGsglQawCAHGwkeKPAQfv1n4QD7Bc/UQTAPEGUDMet3YpDjygyft9B37G3mgOMEr5BB2AtRuYKVDyC8BPPASHYJv7CCSB5AtwRXyoEeMRerfAdtbyrCBDPQwQpwWYLkFWCQBEQWIUIMXgJCXBFA5IR4P8rm9UsDA2PhLxe5WQTB7AsIQULwEECQ%2BJQhwfEJCHOCF%2BSQtYhH1oHo1lyug3wYwP8FZDmBlkFoX0PyEWDIhJ/S8EX0cx8DuhpA8oQMKX7uD5SnXN2L8UNTdwkS5He4CZHrBhB3MYiWnsgGxjzEUc6dWVM2isDNoEMtvI2qmwCJ6C0whDJjmRj27NQd8gI4EU1FBFAiwRII8EVCMhEwiIRcI6EfCMBFqMbu9vRPqTw%2BR7dGSj/OHr8KjRrEv0KUeTqXyS5Pl/wYgWgBCAXgaCIudFNEOSMpFEcQwyRJwoQz7gQAjhqAamiiNpH%2Bt/IaGekVSKZG6wWR64N0BAEO6cD8kyI54SnD95pxXhZjfcMD3lEwDMRCGbka1nngh01i5lbuPPnNjEhIy3lCUVcSlFmBr%2B%2B4OUU1Dqh6FMRFgZUcFFQzSjVRyIqvlZAFGMjmRBkVkWKP1EDdSu2KDUceFoABssKt%2BP9u4RAzst3RWoBkSfyFFPgRRbIh/D2AgB%2BjDRNIdtqcKNrBjQxIxRwmMTvr6x2qlcfkbGMFFeiIQPo6nIqFTHpiAxWYnqjmLRAEg0w0ObUmWIpEVjhR3o0UTUyLj1jIyeI5sUhRKjMtSxOo8sZ6J7FVi%2Bx59JgJfUCKF1jgJooji23KIGiGxw4yumiDDjJgwgxDXsBg1hCyVLIHo%2BMZWOrGAC1xJHYrueIeiXi%2BxhHE/lyJN73iExBgJMWKPCGqDTR%2B4JKLvwhjo9awU4i8TOOrE/jLBN4iwABJSxASNR%2BVOQUaTkF3jQJD48CX2MglLDiOMEy0ChIljbj7eIeRTjb11F5IqMkrS8PeIy5jQF4IYKnP33YZ881GwEqyHRMdz99mJZgViZZDok0lzGwQP4NxOK5sS8QTIi3uY0czOMRJvEsSc1XMYRBZGwkk3lZAd5k87RrSbEbr1xHP9JBaxNSRiLUagJUgBwUEHIK/jYjGSuvMyYsJDCghROGogyYDyRJaTVeOkk3mZ1vCdVmsgUDgIsFoCcBm0vAPwBwC0CkBUAnANtJYGsABhOQG4LYDwFIDvdQpfkxYG6BACSBJAIYDQJIDMCSAi4AATmbR5SuAAADkkAaBm0%2BgTgJIGCmaBeAEUjgLwFP4aAkp9UxYHAFgAwBEAKAVAIfHoBkAKAinfqQMGABcA2gNAPVHEFP7/x6ppAaIGEAaCfBOAiUg%2BH/QIACQUSy0lKaQCwBEgQK6wMKfgBpAXBEwp/HaevC8i25DpvAGTAFJ2kgQuwS0jwFgDmkMIWAK0vyXwAMAn8EIzCASOQU%2BkyBBAIgMQOwAqD8BBAigFQOoB2m6BAgBgRRKYGimWB9AeAaIKf1gDtYQAb3UgIEyFh8QGAboRDOFNcivVOAmkASBY00i2QgoyMqwJYC4C7hmpVQFKOkBcCARRgfgQICEGmAlAygegPIGkAEBczBZKQYWQwF6D8y5g7QNmQIC6AjBPALQPQHYDlm1BJgUs/oOUCGDdBRZgQR0I0E1lq4JAiwBQHFPBlJSaQ6wHgP5MCl1SdpjU1QKVKLiaQi4kgH2lmX2DjSQwXATRFFIZk2B8B%2BAQ1glLpAeARpTHf4FwHmC8BkpWgEmXqD5wDAwMpAdKc2k2AhgCpkgLgFwA0BcACpmwZtM2gKlFwi4hc6qRwFqmkAPpkgXcCGEkClTdw5UzKRoHKlmAm5u4UgCFLCmNTmpIAVqXHK%2BmdSepl0zHKlHICUB06CgZQIYCqBCAEAh2EKatL6mJA6A4uGeeEFoDzzF5c0g%2BKvIGkgAxpbQPeUfGIACRbc28uEN3N4Cjy7gtPc6TfNUBeQ6gwFTgLwEhnCBis4M6QB/OhlqA5p8M/QIYGMDWBrAaMjGfAE%2BTSocZ63PGQMGFhEySZjxOcBTKpkG5aZzUemdYCZnNTzZKsgqGEA3lzyF5V8oGSoOtm8A9Sn022RwCCldy5pjUlSFdMNZOyXZbsj2WEm9m%2ByIA/ssBUHMIAh4EpMctqSlITkDd7QlAGhVXI%2BnFSQwzacaVwCLnFzFFu4AIIEGvnhS35tgfuSIvjlpSQARczOc2l3DpycpJi0qYooKmlSK5mwe2T3K0WDySZ90swHYoakOL2piwQJkgj8CSAgAA

#include <iostream>
#include <sstream>
#include <vector>

#include "fmt/core.h"

struct FmtLogs {
    std::string_view fmt_str;

    void debug(std::string_view msg) const {
        fmt::print(fmt_str, fmt::arg("msg", msg), fmt::arg("level", "DEBUG"));
    }
    void info(std::string_view msg) const {
        fmt::print(fmt_str, fmt::arg("msg", msg), fmt::arg("level", "INFO"));
    }
    void warn(std::string_view msg) const {
        fmt::print(fmt_str, fmt::arg("msg", msg), fmt::arg("level", "WARN"));
    }
    void error(std::string_view msg) const {
        fmt::print(fmt_str, fmt::arg("msg", msg), fmt::arg("level", "ERROR"));
    }
    void critical(std::string_view msg) const {
        fmt::print(fmt_str, fmt::arg("msg", msg), fmt::arg("level",
"CRITICAL"));
    }
};

struct CoutLogs {
    static void debug(std::string_view msg) {
        std::cout << "CoutLogs: DEBUG: " << msg << '\n';
    }
    static void info(std::string_view msg) {
        std::cout << "CoutLogs: INFO: " << msg << '\n';
    }
    static void warn(std::string_view msg) {
        std::cout << "CoutLogs: WARN: " << msg << '\n';
    }
    static void error(std::string_view msg) {
        std::cout << "CoutLogs: ERROR: " << msg << '\n';
    }
    static void critical(std::string_view msg) {
        std::cout << "CoutLogs: CRITICAL: " << msg << '\n';
    }
};

struct OstreamLogs {
    std::ostream &os;

    void debug(std::string_view msg) const {
        os << "OstreamLogs: DEBUG: " << msg << '\n';
    }
    void info(std::string_view msg) const {
        os << "OstreamLogs: INFO: " << msg << '\n';
    }
    void warn(std::string_view msg) const {
        os << "OstreamLogs: WARN: " << msg << '\n';
    }
    void error(std::string_view msg) const {
        os << "OstreamLogs: ERROR: " << msg << '\n';
    }
    void critical(std::string_view msg) const {
        os << "OstreamLogs: CRITICAL: " << msg << '\n';
    }
};

struct Toto {};

int main(int argc, char *argv[]) {
    constexpr std::string_view bar = "===============================";

    fmt::print("{}\n", bar);

    std::vector<AnyLogger> all_logs;

    // all_logs.emplace_back(Toto{});

    // rvalue
    all_logs.emplace_back(FmtLogs{"[{level}]: {msg}\n"});

    // lvalue
    auto log_feature = FmtLogs{"<{level}>: \"{msg}\"\n"};
    all_logs.emplace_back(log_feature);

    // lvalue reference_wrapper<T>
    all_logs.emplace_back(std::ref(log_feature));

    // lvalue referencce_wrapper<const T>
    all_logs.emplace_back(std::cref(log_feature));

    // raw pointer
    all_logs.emplace_back(&log_feature);

    // shared_ptr<T>
    all_logs.emplace_back(std::make_shared<FmtLogs>(log_feature));

    // Other kind of LogPolicies
    all_logs.emplace_back(CoutLogs{});
    all_logs.emplace_back(NoLogs{});
    all_logs.emplace_back(OstreamLogs{std::cout});
    all_logs.emplace_back(OstreamLogs{std::cerr});

    std::stringstream strstream;
    all_logs.emplace_back(OstreamLogs{strstream});

    for (const auto &log : all_logs) {
        log.debug("CHO");
        log.info("CO");
        log.warn("LA");
        log.error("TI");
        log.critical("NE");
        fmt::print("{}\n", bar);
    }

    fmt::print("Inside strstream:\n{}", strstream.str());

    fmt::print("{}\n", bar);
    return 0;
}

*/
} // namespace log
} // namespace arthoolbox

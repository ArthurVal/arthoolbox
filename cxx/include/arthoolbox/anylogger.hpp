#pragma once

#include <functional> // std::invoke, std::is_invocable
#include <memory>     // unique_ptr
#include <string_view>

namespace arthoolbox {
namespace log {

// Metaprogrammation nightmare, please close your eyes //////////////////////
// Sanity may decrease abnormally the longuer you try to understand/work on this
// s***. I don't even know why I keep doing this... I must be a masochist.
namespace _meta {

// Goal: Given a templated type T<U> and a list of templated type Vn (V1, V2,
// V3,...). If T is included inside Vn, we set ::value to  true. Otherwise
// ::value is false.
// Example:
// - is_one_of<int, std::shared_ptr, std::vector>::value == false;
// - is_one_of<std::vector<int>, std::shared_ptr, std::vector>::value == true;
// - is_one_of<std::vector<int>, std::shared_ptr, std::unique_ptr>::value ==
// false>;
template <class, template <class...> class...>
struct is_one_of : std::false_type {};

template <template <class...> class T, class... U>
struct is_one_of<T<U...>, T> : std::true_type {};

template <template <class...> class T, class... U,
          template <class...> class... V>
struct is_one_of<T<U...>, T, V...> : std::true_type {};

template <template <class...> class T, class... U, template <class...> class V,
          template <class...> class... W>
struct is_one_of<T<U...>, V, W...> : is_one_of<T<U...>, W...> {};

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
                                      std::unique_ptr, std::shared_ptr>::value>>
    : log_policy_traits<U> {};

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

    virtual constexpr std::unique_ptr<LogConcept> clone() const = 0;
    virtual constexpr void debug(std::string_view) const = 0;
    virtual constexpr void info(std::string_view) const = 0;
    virtual constexpr void warn(std::string_view) const = 0;
    virtual constexpr void error(std::string_view) const = 0;
    virtual constexpr void critical(std::string_view) const = 0;
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
  inline void critical(std::string_view msg) const {
    m_wrapper_ptr_->critical(msg);
  }

  AnyLogger() = delete;

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

https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QXACx8BBAKoBnTAAUAHpwAMvAFYTStJg1DIApACYAQuYukl9ZATwDKjdAGFUtAK4sGIAMxcpK4AMngMmAByPgBGmMQSAKwapAAOqAqETgwe3r4BQemZjgJhEdEscQlcybaY9iUMQgRMxAS5Pn6BdQ3Zza0EZVGx8UkpCi1tHfndEwNDFVVjAJS2qF7EyOwc5v7hyN5YANQm/m5UXgwO2WKn2CYaAIK7%2B4eYJ2dsLCQAnncPzzMeyubw%2BbgmxHCwAA%2BgA3PCYADu/yeAIA9GijgBZTAtVLEVDAYisFhMRpHBh4YAIAik4iYUhHVL0JhKI4HDLvH7rYhHTA/TAKI4YkWisXijHozFCQyEH5HUnyrDIems95MGIMEik2i0eUEBDvWgCYBeeJHbleI4EYj61BHS5YYhzBjoNGIkgAayOAmtCDwCilRwUACowwA6I4ASSO6AEYA4BD5sMYR09WsRR0RCHlMc9mEwqVjqChfoD4Yr0YVXgmRziRyYCtZqGQ/om4YBzDYClSTC2R2hbBaJwA7FZUU8MUcAOKoMQgGd4FMMBvWzAsZlkzDoa0/VLvAAqpzccjuDddq9oAaTqCoa43BgI293%2B6OADUVxA30F32ZSEG338UgK3DZZIyjO8DyOANoJBLwsB3cJMmOD9GURd4lCTEAQFhMQzWte1rWIM1IwAeQNeJEQDTAg2w3DvHeGCqDEJQO0nTFsFUVhmXYIMAFpoIUaEBEwYSqGPcICEZCZ0GwhQEFabdoVSG1pIIWScMwBwSDuOi8PeU4ABEjKOZjaCUU5x0eKcBIDYSIjE48ZLorSiGICTBDuNSNPkxT0GU1Tg3UlztPc/xsD0hiPmM/xDKIs1LP4wT7NE28nOCzTQo8ggvKCnyFPpfyVLIPLsMuPAAEczQCsKIpw/ToqMoMzIs8LEqeJ8Hy3MEDlZBRGU6zcnx6gwFAUECz16saJrap4IS8BxkpEsSjgXZyQBa0SCD3AyxxMEcYqsgFBsfAyzhO7rjym8aK0m0ahQPRlrpAo5T1mx55sWuzlrSs4jzOOQZuwRl/uwVbSpAG1qu2199qsA72ueDr1yGs63Au4arvuoH2Xuo5HtxvqXrkf8niOcmKcpjG0ee27wsJ6bKzfFEPqhr6hJ%2B8S/uPQG6eB/HGTfHG1oyqGtp20d4cOgFjpR06wWpkaib5hmHqe7HKxJ%2B9UaVxmO3p6731Jx5KdN7X5ax5X9bB2nIwAdRZz6k2%2Bhzfrcf6TyBwXGTt4Wltdrn3Z5r2jl9lW4f26WJ2szFDK0gx6VXG0mEIE4ADYNGpy2xvxs9IkeLFsGhC5BGyaFk8IQN/CsDOHTZIh2UNZBvTwO8KOay5rgEIV6SqvBCrrfVDSOY1gCZTw8GQeVMFUa8hUMHcZ%2BTxa0A3Oh3kcNgg3CfhiFJRohV9Qg2JjoMDzbIiU4IIU0EEFOGAUEAksi/CIBiVBPGWBco1dSetyFVu%2BNsL50LnyVQ%2BJBSZF9DPOe55EIEATPPUyndGhBjNug024RYQtg1PQBsh9ILAILtgCA60IRQjhAiREyxEaSnYtGO8JgM70QSrXGCmECLxQZNBJMLF7RYBoBEP07x%2BC6lQFRIwT96ECWwiXLuDBy4SwgDDTAX9YzxxURAcwacDxEMLqsYRyCrjkhUbQzEMiQB2S%2BFUYuKDshHDfh/WgaixbQTvHIxoijXwwUbFqFccxHDIAVOuKoaCjHyMZJtH0FFiBUQsuxIMnFuLiClEGJ2%2BNUANwjvQym2C8A7iwDELwwBSEZXIUYShSJljsm7jlauQZI5mLSWzJMB4siSzCeTAJk8jh5IKZgIpJSyE2gofCKpZjRxR1PvQ9JB4ySNmyTHU23Sgl9PUYMiAkkaH1PoY0nZ0ylmzNxLtKWTSZktECdCPq8QCAQC1EmQpxTbHGLLhXa%2Bx4DzHN0vVBi2yrJThWVcsaNy7mZPWU8jxrziSVw%2BfM75LDVFnMORcyeQKlBtAgI8mEkKBDl2he8v6mTUDwv0n85pZJUXXIxVi558i8VXyru7Il3yrEhPiLSxoZLzkUuQGikF607IKFYKJWEx4aU4oUW8xlB5mXhVkXY3FKjjZTgwaqtV5NlS0E0domVRBsJYu2dgLlByAUot5VS254qFWSvxdKrIJLfkTMBRa0FDyBkQutfSmFf17VyssUJax7KJXGt2AI8I7xHiRAAJohFItOaEbgABK2BHgHiLgAMTkJEd2UZSKRGhAeRNjwowHiEBAEBRr1VVqrSYRIbgARUzlpdM4hsCaGxMn0s81bu3dtrfWsmFN0kVt2LsDlUKGXg3WptLxJy9kWB7RgvtDaF0rtXWu9BS6B3k0VtnB6Xb10HurZuk2g6WlHGHUCUdEqvUErcIew9x6MEE3Wn08uYqNE7VKRpTV9EPl3AgGBCtX65IjIqWM5Ee0DrLENWeR96CRYaTFjOyW97UOmzg2bGspZr0qOiuog42qzA6L0UayyaHKYYeWWampD8nzgN5O/TwyVA3EDHb6Ey5GTh1uXWqgVAa2WsevekSS7LRVnBwztf41cV2UcadHSNMa40JuTamjNWac15oLUWktZaDWIwU7G%2BNSaU1puhJm7NB5c35sLcW0tmyGD8GNQZpTxnVNmfU5ZzTNmdMQERK0BgTno2GeUyZtTFmrNads2W%2BIBJiCBcU0ZlTpnzMaes9puzKosiiGcYjGWQJHSYDvM5xLoX3Pha8%2BloQMtUQHXJlOLsgpez9kHLiJg1XrJhgbSGcmfaYiQkK/jW1Po7yNmAEuVMIRCTKAnlPLrXXybnxgm8o49ICAbAfoY0RxoJFjx3tqClzh5sWPSFeKeyHlE7TUefd4k3gDTdO/qCW9Jvgph3FQAkLAjhuDfIyekVB4iMH7AvebqHhOCHNFVMQrcERhSskcbrFjxWfmQDWIgn2pVXfxX6MksF8CiGGoA9uZNutYuAyAcpMJwPVJgmGiIO4F6MkAYQRBhjRBKEZEwdAePGi3GJ7BXe%2B9siHxXBRcJJiJYLzcTwlnjYVnBMqOaCVPp3J8/uYjeHRwLF7eLn4I4Mo2D4OG%2BC7FK4ilJl3oYvbAGxfZHVwj1aIA/PEAUVQXX%2Bv1QEONzrusXhzckEMU7hg1uJV281w7mLJAdcLnd4b28XvXc%2B797yUXEfiDB%2BtaHixmXAliCj3r4Vse7xI8T6Zf3ovs9/1oOnl5Ag7cg77QQXsxJPtQWu0cW793J5/GJ2iWWXVMYtrxm2vGHaSzoEdme0eykZs/BvUKRZlMsNGHHg95DTUt1UY0s91AKZp8idY3U8EGVt%2B77%2BwDq4W10pb/XDv0SyBYRvu5m1Z/9x9mL8yMv4vJkVENYHEOJg%2Bq7q2KnqUqx4J2XeM6UmcOFMS%2Bu2DmqA3u3%2BO0v%2BLWLQ2E2uOGtqYBM%2BkBz%2B0B5MsBWY/miBsUL4jABeqBABjuxBmBDK2Bq%2BpieBPGdcpYqeJBcUP%2BFB/%2B2EbBtB3qbg4BZ2jBr%2B%2BBLBy%2BFe2W7BZBKB3BKAkIOetAbGNqdBZwghs%2BwhuWCM%2By6InWfOXGbgfWCId4Qg%2B4yAeAUOAAXgdiuHHlPmoXPqXryOtGfvSBftCIiMSKkPuGQPNutOVFVKJMVHAhDL5IVDVGQYGMTvXnWo3q0KwPjPjMPB3jPl1r3sjP3mjDuoPn1OrFbO%2BHdH1ALKrK9LkXrPke9OknYTgaAWcMwR7FrELHzMbJTOtOQTEPQNCK3I/m4JQegRzAHH%2BgDILCHM4YVuflsO4Z4d4c0ZxrxhlP4dVMVN5HJAVEpMVA6glC/swQuFUQwVgQDLBpBlMmfBLNgMSAoBsIxA%2BOuIwDeEVkFnGtONgImgCOko8AwD8LdsAOaAvuTPiEuFuFIiel0merdh4BfipB0hvrkv3GtmIEcAAH5gkCBbAqTW5agzyolJgmQCJMBeBaq5bQkUzwhtBeDwm3wTAzz4gQwLGBE2jHjIkQl1I2zGgRDW4UnYmkEaCEnAmmwklwm0A0aUn0a9Jj7G5k4U6VLUJCmclxTclv7oL8lkmCkclUm8hrJW7DKQhgZULVIcl4bymiEwmknkm1Jqmin5JEHO4SmgaU66kykGnq6KmwnKkynmlrKp42nal2lVIOkmSGnMHGkClukilrISFiBemjL2n6n%2BlOk9ZaFHQb5ZFuCGzJEPYswtGgmEh2xTGK7hDwkLiMlYlQm8nv6lj2FSp4a7EQE1FuBpld5QGBkUxoi6GlmUzdb6GGEDbgnpKNi3Y5lMBeHmhjbLjnjyj1lTwrYIpNkUwhhpFtnbpNoD4pl4xvT3BEnkyqkin9m5lp5yAnBEbaKtDADVILgsDT4PZk67xO7j4HHhQQDHnQYdIIzRwYItkhgznkwdm9b9Z3huCsnqgfHt7Zm7mDw1KpA/CliVz1DiQblfk9Z1qrbrY0mUgBE1QMmEjglYn/ohgGgBg0JwXw7zkYJ%2BGoWLH0lnBFmFjMm4wiTsm1I%2BgpjECQjHC/FqpIXO4Qykj5jQi0kYXAADlDm1QQC4VtjGqLovlIxLJxy9SJyS54DXFsCCCGLpr5pAZalRlIgKgKAnmM4Pz5I3YgWDnTFBjZiTwIBZh0AqliCCmi4Tk/DYSqXQhAYsA6UGIxBqg7hHx3jM4cBIIrIdw14bZx6GwSqRGAj%2BC04RoPElZubBZ2wloAAS0IIQ2Ab42AIQ5axC1Ssx6G3GG%2BzqwKGKFZtqJGI6QIUePyZojIaGlGvaZgZgbeI5E2U2M%2BxYgoDACYbqgiG8w8EqC45gZgB5/gFa%2Bhn5aqg1kZOpVSHYDVfyuVZslGay6lZStpUp2lJ5Dpt%2BzFBlKG96dVMBH%2BY8FZeGnBBuJVDK2EFZkm0mva%2BVC5UuW51JEAF1lcZVl6FVrufRLWCugm1q1ScMG691VaAqDA2C%2BYWiRGFZJGjI55ahsNblZGa6B18ZcU9QbIgNC1uVKNm%2B6BYNqAEN2i0NIAFaCNJ5SNsxONkcWNNNFGwNFMcmUlGCxWIWcVSmCVB4yVqV6VmVemcFLNrmpm8VSVKVaVGV9mjmn5AtSWRcwtnNotPNvm/mBFD10tpWctXNYtmVqeKtzNMVrNQt7NIt3N4t4ZOW0cuwBW9xCWBtstRt8tJtIQ7WGC9lCoF5DZ0mLZYAYAx4iRRohI3xxAaIo8ahdYmApYHhxlz4c5zBc6ztp6GktJ6FlFmFKJ1FZ455kdQlNU0IPJFMqQXg7Rk8QJjaGRusQo9lGZFMT1vI7xnxAd8QEArt2i2i1ZU8ut8GbtWd3hOdbF6qEQmYO5UdYUdZrV6Z95U6JAN5/Fnes295bdPw0G1NEcklzB4QV4QiaypOGl01mYrlm1MZe0cFmdu5OdfEdwpO%2B94lDNkllM694aFpiE8BU1Ppe9blfpY4Xdp9xU0I594UVuV9ZGjNd9DAG97waygeL961V9H986J9w9Z9dwkDgD0mwDFM99m9YpnpO9r9G1epDFfdlM8D2dP9f92AnpKDohaD5MGD4DYpZtUD4GeDsDn5xDPdpDdwDDlDsdq9G%2BddXxjdANpBWA9AT4ed6DoDD9VJp2qc/DDdae%2BpcjwAgdB5acmShocWn5Z5kxCDP9EA6j8Q4YbD7KHD4UHIbJT5K9xkr5EjYDRwSjKj2iqA3hZIOksUEAijHxAjvITjMSANR9D1xjrGP9eGBjxARjOjJDNov9nDAFAGcZpsHFK4olAYCTcdNjNDkjQi0jk8sjXj8jEADjPxh5RGYTHdZs2j3dJj0TZOL2mA%2BjMSETVTwT0TljRx8dmTdjRTPjpTLjbkRkhT%2BTyjxTacvjGj/jRpFMQTOdeG60dTDTGjTT39rTCTlMST8OeFVcVDCZHTvSLp8JCJ3T6JqAmJ1FeGuJ%2BJdSR0OzFtQIrw8EaMTgEImArALMLwcErFZwY0NoLzLAbzdzHzaMKYWU70eWwIBwDzB5ZgVALABAaIaA9I4YCAg17WQ6qAt28%2BATmZPKIZ1JW9QBjD9pVjzBcuNdj9/OqAhLvpxLBV1GZLED/mVL0pNLvJpLZpoZWDzFJATLEz1DQUOL9L9D8hlePLz51jzwNzUl6S6asLGLJZmZPka1TDMLBA0IEI4j5M%2BLGyOD0D79h9kz5MKrV12ptyKrargURrIAx5kNZg%2B9g1ZNBilr1rg19AKYtA9rULhk2AFgcg04g1S9CpqNzBGpz9OrTDMD%2Brn5lr/xgg1AsL5rJUTrxAJSg1drDVDrkSsL2EzrDVrr9QHrg1UYkQ6apE/r19QbG%2BDL1pYbVCzDkbcF0bJrcbqrEImbBA2bybNrabf4eDbbHbKbubmAbrBbDVdsjwiakQZbaTt9xJnLsWTLdbBDWL6CjbkkzbCbfbVrnbqbOlHrV9m7ObZgeb7r6bULzxiapELxc15bkywbQrWWEZNbWlEbS7BrpkWbIAMbpr8brb777bW7A7tru7p7%2B7f7/bNrx7I7ZgSaJaUYbgjwTt17074r6TUrZ6HgvucrfdcuWrxSC7MDhDCd2EaAvuYIvtg1GHgwhIj8RwXrPr04A1DVZHZwG1zHd6CYfanVOwgbt7tLOLIb/A%2BH79hHIJGkJHnJ9aLHFH6wVHwANHRbJbjHQ1x4vt%2B9bHRwHHdaXHyHJL1GVbQeT7b9m1InEM4n6n0nmH1HC4Y7E7Sn6nanKnLHmnbg2nPHfLOHc73LhnzDJn60Znjnd6FnsnNH57l7dnAXrHEXznrn2z4r2LgS5LDD3nBHy7ZsfnMn5nDVlHGLC4MHlm8HIQ4XtRLHDnxX7HOwWnHHbnq9krrxZ6pEzzrAWHqXEMGQPz8RTjWzuzuHQyq13purB9r7n5GQmXZgDX7XLAOXtH3rvrRXknd6pX83GnFXLnVXsXd7lpmpfXmlRn%2BDtG8r6CI3EXg143qok3Vn0YxbpEc3qnOl6n0Xa3PDcXs7lpkDyXerQ3cFR3ZXULp3vzU3NnkQN3JXd3UXK3MXT3G3i8XLae73g3%2B3Jn33S3J3jX53cnC4oXiawPC3oPP3D33H63lb97ChQn8PtYiP8%2BP3KPE3U3eXcHCH2PkXeP4Pj3G%2BjNqHdXxEi0uq9oxL2hTwkkTY4Q9mvCybyAT0BU8Ox5oqiQVgiQhkvLG%2BZL3nHlPjpBKLsUTUmv2vMUuvWvevOv%2BvRvhvJvBvZvxv5vmvDVGrYHn7TbKL7TWnHrqvIatLGkwL/TZw3TZ4Nl0Io8XXGTwomIvv/v4YTaExHlzcEAPPVjrvvJU4xA05G%2BIf1HYfXUEffYnoEAMrwXEc1vsvcMkHCM8vA1Y4drxfLnDvCvNvU4tASfvJeJDcU%2B/2ZIlxeGOfGLefZgKnY4RfMUEUY11vZfu7FfKLTvQ/UylMKfcnafm4GfUfzfLza29IcflMtfCKK2YxrhExzTgx65Dfuofvqf4fokkfWfoxVAEAi/rfK/q/zZmIdfDULhgOO/u5V0DFoMzB0/40J/0IZ/ZOKoQrFf0JDFwl%2BlxANomXj6YhiQmYMHE%2BBVwH9FCofX/v/1bogCW%2By/RFPzygHBhViRUCikHHehT9D%2ByA9Pqf0z61MmAPFUItuGPAd9qO/6a/pgIgG7Mpw5EDRmmHCBeU7wE5BEOFWIFIDj%2BZAv/hQOy4MCjiN7b/rPwMDz8s%2BkQdFuIKliSCSBQgufuQKj5/cmuig9Lr7kjjKDBBM/FARQM0Fo9AwY4PzjFj0E28cGqPIKM6Am4JMpBRgjQaj077mCbQqPKwdgMpgW4PGDFRvrzyIyjxwY3/RXg9VHjhhScFHRKqWzmqrMKYEQq3BR1iFmAb2lMCIZA0GohBAQqQ%2BIeTAiGelBqlmf1nkJHiEhwwDDQapEHuBxCeOPgj9l%2BxtbEtx%2BPbF3jp0D6rtY2hbfSscAhCo8n4Tvdpqez6ETdwwEIADHf0NYND7eQ/U5C0MZBtCeO6zAMjVkMgcBVgtATgIkF4B%2BAOAWgUgKgE4D1pLA1gYMDyH7C7AeApAdtnsPWGrBPQIASQJIHDAaBJAZgSQGnAACciQd4VwAAAckgDQIkH0CcBJAOwzQLwEOEcBeAj8FIDcK0CrA4AsAGAIgBQCoA149AMgBQD8EYjRgwALgGYCCA0AtU8QR%2BG/AhGkAYg%2BZW0JwCuGrwlKBAUiKAwcq3DSAWAUkEYHECsj8A9Ia4CmEfisiZ4WkX3NsH2EiZNhrIq8H1laA/APAWACkSMhYC0j1hfAAwHJzfBUJSI%2B4XYVcP4CCARAYgdgFIBkCCBFAKgdQKyN0BBADAkiUwCcMsD6A8AMQR%2BLAAawbRYWpAJipDGIgMBPQ0GA4SpCFycAYR9QVyNkBcCuhpgfgIIKEHDSLBRghQDIFkAEBRi9ARQZMQwAWAjBqgPQMMQIH6BTBPAnQPQHYDzFNBJggwOMdmJLEVjUxQQOYG0CzG/UJAqwBQOcKNHXD6Q2wHgBsK2HgjWRUI1QH8LTh8Q04kgI4MAGQBBICR4YLgA4mOFWAHRRwXAIQH9yXDGQHgXET00CDLBeA8Iu4aQENCc5RgAGUgA8MSD%2BBwwnwyQFwC4AaAuAnw/wIkESCfC04acJ8SCI4BgjSASorgCOHDBvi04BIscf8JqBAhgRuw/YVCJhEgA4REIxESiIgBIAhRKONyOQEoDHkFAygQwPUCEAIBxEOo3gKvFSB0BrC2EiILQDwkESKRxE9eAkHxGEjSAtEzEaRF9xUTEQkE3gChMeDJsBRXE1QFpGaD4BdhvAPUcIGyxGjpAYks0WoApFWj9AhgYwNYGsCOjnR8AO5MKndFSQvRUMX0f6OcYHxgxawDYFsBrHCTyJuE/CRxOVGdiCwNkrOsqN7EcBthpATiQcM4CcRhRbkI4EOJHFjiJxU4o4DOLnEQAFxKk5cfgG8mXDdx1w%2BCasCPFOhKATk78UqL%2BFmBww/gTKX8JHBPi/h/wyQCODHGuSKR0E2wLBNim3D/R54xIFeMSAjgLxrw%2BqXlJfF/DPx/gfsVBKMn7j/REoswB1MhFdS4pno0keGMkBAA%3D%3D%3D

#include <iostream>
#include <sstream>
#include <vector>

#include "fmt/core.h"

struct NoLogs {
    static constexpr void debug(std::string_view) {}
    static constexpr void info(std::string_view) {}
    static constexpr void warn(std::string_view) {}
    static constexpr void error(std::string_view) {}
    static constexpr void critical(std::string_view) {}
};

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

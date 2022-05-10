#pragma once

#include <memory>               // unique_ptr
#include <functional>           // std::invoke, std::is_invocable
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
#define ANYLOG_CREATE_FUNCTION_TRAITS(NAME)                                   \
    template <class T, class = void>                                          \
    struct NAME##_function_traits : std::false_type {};                       \
                                                                              \
    template <class T>                                                        \
    struct NAME##_function_traits<                                            \
        T, std::void_t<decltype(std::declval<T>().NAME(std::string_view{}))>> \
        : std::true_type {                                                    \
        using function_type = decltype(&T::NAME);                             \
        static constexpr bool is_member_function =                            \
            std::is_member_function_pointer_v<function_type>;                 \
    }

ANYLOG_CREATE_FUNCTION_TRAITS(debug);
ANYLOG_CREATE_FUNCTION_TRAITS(info);
ANYLOG_CREATE_FUNCTION_TRAITS(warn);
ANYLOG_CREATE_FUNCTION_TRAITS(error);
ANYLOG_CREATE_FUNCTION_TRAITS(fatal);

#undef ANYLOG_CREATE_FUNCTION_TRAITS

}  // namespace _meta

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
 * - ::fatal_fn: Same as of debug_fn but for the fatal() function;
 *
 *  \tparam T The LogPolicy
 */
template <class T, class = void>
struct log_policy_traits {
    using policy_type =
        std::remove_pointer_t<std::remove_reference_t<std::remove_cv_t<T>>>;
    using debug_fn = typename _meta::debug_function_traits<policy_type>;
    using info_fn = typename _meta::info_function_traits<policy_type>;
    using warn_fn = typename _meta::warn_function_traits<policy_type>;
    using error_fn = typename _meta::error_function_traits<policy_type>;
    using fatal_fn = typename _meta::fatal_function_traits<policy_type>;
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
#define ANYLOG_CREATE_LOG_WITH_LEVEL(NAME)                                  \
    static_assert(policy_traits::NAME##_fn::value,                          \
                  "The given LogPolicy doesn't define the function: " #NAME \
                  "(std::string_view).");                                   \
    void NAME(std::string_view msg) const override {                        \
        using policy_t = typename policy_traits::policy_type;               \
        if constexpr (policy_traits::NAME##_fn::is_member_function) {       \
            std::invoke(&policy_t::NAME, m_policy, msg);                    \
        } else {                                                            \
            std::invoke(&policy_t::NAME, msg);                              \
        }                                                                   \
    }

        ANYLOG_CREATE_LOG_WITH_LEVEL(debug)
        ANYLOG_CREATE_LOG_WITH_LEVEL(info)
        ANYLOG_CREATE_LOG_WITH_LEVEL(warn)
        ANYLOG_CREATE_LOG_WITH_LEVEL(error)
        ANYLOG_CREATE_LOG_WITH_LEVEL(fatal)

#undef ANYLOG_CREATE_LOG_WITH_LEVEL

        LogPolicy m_policy; /*!< The logger/logpolicy being wrapped */
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

https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QXACx8BBAKoBnTAAUAHpwAMvAFYTStJg1DIApACYAQuYukl9ZATwDKjdAGFUtAK4sGIAMykrgAyeAyYAHI%2BAEaYxCAArBqkAA6oCoRODB7evgGp6ZkCoeFRLLHxSbaY9o4CQgRMxAQ5Pn6BdpgOWQ1NBCWRMXGJyQqNza15HeP9YYPlw0kAlLaoXsTI7Bzm/mHI3lgA1Cb%2BblReDN0CYifYJhoAgjt7B5jHp2wsJACet/dPZl2l1e7zcY2IYWAAH0AG54TAAdz%2Bj3%2BAHpUYcALKYRopYioYDEVgsJh1BiHBh4YAIAgk4iYUiHFL0JhKQ77dJvb5rYiHTDfTAKQ7okWisXi9FojFCQyEb6HEnyrDIemst5MaIMEgk2i0eUEBBvWgCYBeOKHbleQ4EYj61CHC5YYhjQzoVEIkgAa0OAmtCDwCilhwUACowwA6Q4ASUO6AEYA4BD5MMYh09WoRhwRCHlMc9mEwKVjqEhfoD4Yr0YVXjGh1ihyYCtZqGQ/rG4f%2BzDYChSTE2hyhbEaxwA7FYUY90YcAOKoMQgGd4FPkxsETAsZmkzDoa3fFJvAAqJzccluDYYO8btADSdQVGt683a53BD3bwAauSIO%2BuIz32ZSCDd9AgrcMlkjKN7wPQ4Axg4EvCwHcwgyI5P0ZBE3iUJMQBAGExDNa17WtYgzUjAB5A04gRANMCDHC8O8N5YKoMQlA7ScMWwVRWGZdggwAWhghQoQETARKoY8wgIRkxnQHCFAQJptyhFIbRkgg5NwroiGIW56Pwt4TgAEWMw4WNoJQTnHB4p0EgMRPCcTj1k%2BjtJISTBFudTNIUpT0BUtTgw01yHHc/xsH0xj3hM/wjOIs0rIEoSHLEu9nOCrTQt004pK8oKfMU%2Bl/NUsh8pwi48AARzNALsoi3CDOi4yg3MyzwsSx41w3Aw11BfZWQURkuqfQzTn6hQFFAs9xsmitkQecEvAcZLRPEw4FxckBWrE1991HKwRxi6z/mGnrRrcU6tz6gwJqm8L2RuoUD0ZGbQMOU92seRblvs1a0tOI9TjkO7sEZQHsHWsqQBtGrdsMscTEOjqnk6x8ztBS7euPV65vumbDmeh6BreuRAMeQ4KcpqnMfOnGOzxx63vfebvqTX7HP%2BtxAZPEGwb/EHIc2mGdrffbEaO/4TrRq7jxp67idxiH8cJun3qG6WsbGxnFaJibDnfMmHip42H26mWtYV%2Bmle1yMAHUWZhn7hL%2BiSAePYHFb/RlbYFhd2dS12ufd3nDh9nXEYOiWJxsjEjK6Ax6QbYimEIY4ADYNBp7HHoJs8IgeTFsChc5BCyKEbRTghA38KwM4dNkiHZQ1kG9PB70olqLiuBghXpaq8CKut9UNQ5jWAJlPDwZB5UwVQbyFV0%2BVUCvlrQDc6DeRw2CDMJ%2BGIEkySFX1CHYmOgwPNtk8IIU0EEFOe5AJLIoIiBolQTwlgXKMLynrchTbgmOF86FyXniQUGRfSz3nueJCBAEwLzMl3MkQYTaoONmEGELYNT0AbEfKCQCC7YAgJtcEkJYTwgREsZGkoOLRnvCYDODEEp11glhQi8UGQwSTKxe0WAaDhD9G8fgupUDUSMI/WhgkcIl27uXUWEA4af1jPHOGEBzBpwPAQwuKxBGIMuGSXc%2B5qEYikSAeynwKjFyQVkQ4r9360CUcLGC94ZFkjkXtWCjYtTkhdI4ZACp1wVBQXo7ujJto%2BkosQailkOJBi4jxcQUogyswJqgRuEdgkU0wXgHcWBoheGAMQjKpCjDkMREsdkAgxjGNHFHM%2BtCUkHkyGLWhVNfFT0ONk3JmB8mFJITaMhcJyk1PFsY5JjskwHlJI2DJrTKbtP8V05RvSIBSSoTXIMoyNmxIaRMgmOJ4aRzGbs0kU8oQDTiAQCAWokx5IKVY/RZcK7X2PAeA5ekGqMXWdZKcCzzkTUudctJyz7muKeUSF5ANpkfKYZgb5SSTl%2BP%2BUoZoEA7nQjBQIcuEKq6vLSagGFBl4WIrORc1F6KHmyOebigG%2BKPnmMCXESlZJiUxyCqc5AyLAWbXsgoVgYkYTHgpZihg2LK7Vy5nS8K0jrFYrhobKcaClXKopsqWgqj1EHnxThdF6zsCspoWyv5ZKrnCtlaK6lEqmmOEJV8mpxqAWopuSCjF5qxWQq5pkelwkLFMpFaynYfC5iHAeBEAAmsEMi04oRuAAErYAeAeIuAAxOQEQuZRjIhEKEB5Y0PCjAeIQEBgH6pVWWstJgEhuH%2BNTDWtMc4qxzqZLpZ5y1trbZW6t5N5l7JLTsHYzLwXisFhlba7jDlbIsO2tBnaa3TvnQuxdqDZ3dopnLbOA1c73SXTujtVa50UxSX2wEA6RXuppW4Xdu6V1G1QYTTaXTy5CpUW%2BIpmk1UMVebcCA4ES1vvkgM0pQykQI0OksPVZ4b1oI2hlYW479pXsQ8bKDqCayljPXDaKyj9garMBorR%2BqrJIapihk2CzKk9zXKoPEdZ7HJV9cQQdvpTLEeOPu1dSqeU%2BsZYxs9aQpJMsFacDDb4/g13naR2pktHihojVGmN8bE0prTRmrNOa80FqLbq5GsnI3RrjQmpNUJU3poPJm7Nub82FtWQwfgrLdPyYM0p4zKmzNqcs5piACImgMHs%2BGvTCnDPKdM%2BZ9TVmi1xHxMQPzcn9OKaMyZ1TFmNPWZYo0BxyNJaAkdJge8Dm4tBZcyF9zKWhDSaeIdCmU4uyCl7P2QcOImDldRGGGtIYKaduiBCXLBMcV4KTsAJcqZggEmUJPaebW2sUwvrBalhx6QEHWD3XRwjjRiPHrvbUpznBTdMWka8094MKLfEoi%2BbwRvADGwd/Uot6RfBTDuKg%2BIWCHDcAbebuW4iMH7K6KbiH%2BOCHNNVMQbd4TZWsocdrpjhVfmQDWIgL3LWnZxX6UkcF8CiF6gAju5N2vov/SAEp0JgMVNgkG8Il4LyMgAYQeBujRBKEZEwdAGOyQ3Fx3BPeB8shH3JJREJBjMOLxp3AjgCDyMMYFzY9yHObnI0h4cUxm3i5%2BEODKNguCfT3hh3WLwSY966M2z%2BqXAh5dQ/WiAbzxBRVUFV%2Br9U/Wdf5P1yQXRVuGDG5FWbxXFvIskBVwue3mu7wupV7rl3vJ%2Bd%2B%2BIJ7813vTFpbEAHtX/Lg/a56aC8kzuzKu/54n2gsfHmm%2B2UbEMf3O0EF7ESF70EzuHAu1dqevxceoilmbTWbhlYvSbbFTpJZ0AOxIstMeKlxvfHPUKWZt6KZoaMBPa78HmocbaRlO7qAUyj4E4xgg6VNJr43/SKgX3Lg7V3zhffYlkAwifW7dqd%2B7gl6prP8eOvTJwxqwOIcTAdUZ9dUXi1fWx4%2B2Te46YmEOlMz%2BnOqAYeb%2Bb4H%2BDWjQOEyuGGgBpwwBh2cMYBB69cpY7uMBve7%2BqeCB3%2BluPmTGAB4qQBY%2BoBd%2B4BM%2BGQc%2B0e%2BBcUhBGuxBOETBKBlBaB1BmBtB2BkB%2BezBhijARBX%2B0i0ytA5BE%2BVBC%2BfBD%2Bx0SMJeaIrWHObGbgXW8I94Qg%2B4yAeAIOAAXttuSCHiPugePpajnryJtIfsfpsFCAiESCkPuGQFNptBVNVGJCVDAlDL5EVLVCIYGLjuXlWpXk0KwATATCPA3mPm1q3qjO3udOuhbBNN3pbPrNNA2mkXrKTLrLNJGMzJ9AtHsmYbwagV2tPluiePzJ7NgZtKIdEPQFCG3Dfm4OwWYs7BzIHNzLke%2BCHDYZ9vSCfg4U4S4YbKxiqu4ZSJ4bVN5PJIVMpCVLaglPftgQuKUXIeUR9BDBkkocdLQgeKLNgESAoOsExN1OuIwLeHlv5lGtONgLGv8Ckg8AwN8BdsAOaFPpTHiEuFuBIpUSkhdh4CfqpC0pUVTHCM0F4GIIcAAH5AkCCbCqTG5aizxIlJimR8JMBeDqqZbL6UyQmLYwm3xjCzw0ZTFVQ1RLGnAIkgk74MyiTG4kkYm94aB4ngkEkDxEm0AUaknUa8hLL479IQhAYUIVLMlYZsmP6oKEnQk8nMlkkCn95QEE5E5lKUK8kslxRSl0HGyynElVJUY0ZLLu6qmAbE5imamSny4ylclymamKl945J8jEBRZmkikWnlJWmmQ6nYEQl2kGmUaOlLL57umDKWkSk%2Bk2kdZ7Hla1qJHyx6wxHXbzQr5D5JgXa2yjHmj8IwkLi0nolgloKQHmET5YYbEgGWrHjJlN5YH4lVaqEcmUztbqGaE9bAkpKNiZnZm8iDbLjnjyg1nTzzawp%2BlUwhjxFNmmwjSJlCjbFjkUwKn8n14EhZlMDOFxAQByDHB4bqJNDAAVILgsCj7XYE57xW4D5Azfr7ngZixIzRxoItZl71kUwtmdbdb3huDGgCKGCDmrk9lDyVIpDfCljXw1ASQvkK7qELZLZQweFUk2jVkEjAnonfohgGgBhUKQUTkLlwXTEIXZRuAFmFj0nWyMnimGk%2Bgpiuk5ITqQWUwwXW5Qwkj5hQjwW0Q0n/nrkuFoUYUKCsozr3koxspxz9SJzC4XFsCCC6LJrZp/rCnhmIgKgKAHnU49y0UrnABrkbmuG0LZhTwIBZh0DyliA8n85DnfA4SyVQh/osAqU6LRBqg7jHz3i05i5JwLKdz/79b4wipBEAj%2BDk5vD5aBbOYBa2wFoAASUIwQ2A742AwQxahCFSExyG7GAJjQSKJqEApZlqBG/agIAenyZojISGkmHaZgZgdefZw2o2Y%2BxYgoDACYtyuWwaee5qC45gZgO5/gJa6huFyqXVYZop5SHYlV3yqVJskmSy8lxS5p6pylB5Vp6%2BkWGlEciG5VEBDB48uVWGrBbwuVfWOEuVom4me6FRaCACS5NGOVZR4q%2BVJ6hVtuSB3GCwvG5qFS61U16VZaPKDAmC%2BYaieGuVBGjIx55hYN9lRGi6m1MZcUNQbIX1k1ExsNxsf1ANmAQNacINIAJakNB50NKNP1y6lWyNZNlMkmoyD5qCIVTmRm4VUVMVcVCVaKv%2BWFU5tN8WRcDNB40VsV8ViVm27NaCnNhWPNfNzNiV7uwtNNtxBWYV8mEVvNTNAtEA0eMtJsotCt0aStEtqt%2Be7NWW/gOWNxsWoV9NitjN/NCVcZqCFlCoJ5tZ4mLWYAYAx4URRoBIHxxAqIY85hdYmAuB2ZO4OFHGk6tth6GU7FtUSFwAKFJFZ4x5jh3FTKJUUI7J3xXgjRU8/x8ZM5G6SZdVKZRRVM11vILxbxXtm59t6i6iFZ08GtxsR5IxKdjGadXx5a4QmY3ZrdsdjeE24UZ5JAF5fdsRg99d3w4G4sd5Jk1NFMYQ14Aigpv%2Bw1npmYdlS1kZCML5SdPZtUUI/Etw%2BOG9AlFNQlVMC9waSyRuClI1699l3pY4DtydOl%2B9h94URuJ9RGVNlRl9S9ypppt9a9i1FFlGCGu9rdb9twppX94mP9F9DAi9bwSy0eq9C1J9j9upVMEDr9ad792AqDsDup8DlMf9yDypoZQD6DD9W9WDlMODLhUD4UoZRD2BP92BFd7xm5n1veWA9Aa4GdpDiDwaZJB2qcnDVdMeEpEjwA3tO5acaSho0WuFzdL9jDadEAijcQ4YDDqdNoB9twHI4QP6n1oGs9wlCDSDIarxXDvI6iqALhpIYURkEA0jNjkj8jWj0WRZqCujbd%2BjWGXjOjLduD%2Bj%2BDRjmNp9JsjF5I6FbY0ZUmyhHGZDoCYjSYMj3tEAGTnxu5eGXjjdVMqje9Gjm092mNQTfj%2B9t5ux5j2BKT2TdjeTjjOkxkWT7jsjOTac9jkSpjdDFMlTadWGpTK1mjkSwTajejjGUTxsMTkOfFCT4dc9nSAZPJsJDTKJqAaJJFWGWJOJ9JihdSRtLwCE50Tg4ImArA80zw8ERwzk5zlzRR1z%2BwJzoIKYWUVzKIgIxztzlVVALABAqIaA9I4YCAXV5WR6qAF2k%2B29GVHKDpy5y9KyVDJOM9dRmVHSZdTpSEtmqAaDKLNTaLcLmLJpPmeLlpBLHG5GxLypqDyL5LZjhLfi8LxpFDkhZLXpFLFWhzX0eyya/zULPjkdPk81wGZk/zUI4IgjWSypQpc1Hp1Dm9lFHdJsfzBAx1IpVyqrErgUqrOE%2B5QNZgG9XV%2BNOiurIA%2BrXV9AKYtAxrO5ZgRk2AFgcg04XVU90pcN2B19OL7L99irYDyrxsZrPxgg1A4r4IYS/zerxAhSXVRrlVJrEbar5r0bBrVrNQtrXVUYEQyaZErr0zJD0rzpgDcrilvroDtYAbVMQbGrobBA2rpUZrFrlVcbAEIDibUbMblVabNr8bdrtsDwsaEQebCz59BJNLrpJAPrIDmDuF1bUktb9b7bybnbhrKltrJ9S7TbZg3bGblVDxsaZEjx41%2Bbo7hbj2bLdLSlGDtDs7kbIAwbmrYbOrd7W7LbCbYrSbW7O7vbXVyaiaDwwQw77rUmKIsZ0cKSHgeuArAb5GiLBSU7GDlbPamkaAeuoI7tXVkH/QBICgC4DrTr04nVlV6Hpwi1JHl6CYnaTV2wwHBb7KTLXr/ACHD9SHQrOEqHLJ1apHmHaw2HwAuH0Y2bZERH3Vx47tG95HhwlHVa1HI75jK%2BcLJL1uzHS1rHUMHHknPHUHOHC4/bg7InknEnYnpH0nbgsntHp79HHSKDE7Mel7ZbgraNGUGnxnl6WnfHAn%2B7h7BnrnZHvnpn5nxDlnsHrL6WKnvTuFm0LnpwGHlVWHULC4f7B4AHPnMXpHRnaXFH2wMnlHFntTYHwlKSZE9zLA0HMLK%2Bmk6QNoFzL29j1c%2BxlRcHfSJbd907N7L56QmnlVxX1XrACXhw%2BHzrqXXHl6GXI3Un2XZnuXQX8nY7zpN9LXwD17Sr5XqCnXvnXVPXqopXOngnObw34nKlknAX03bDlnSnHu9nbXK3fTPok%2BmXdrW3NX/XenEQB36XR3/nk3gXZ3s3Z7LpbpV3y3/rq3Js63D3m3JX/XXnsa73o3n3D3J3NHM3nroXYg4XM7HX9343kPvXO3/HiX/7wQcPfniP33p3YdQlizhXeyWq6SDLSTDwUkTYYQNm3C0byAL0hUkO%2B5gqCQVgCQRkEXHGmLV3jldjveYLsUzU0vsvMU8vMvCvcvivKvyvavSvGvqvmv0vlVUrH76r87YLDLMntr4vAalLGUbzLTpwDTZ4plUIY89XEdwoGI9vjv4Y0s9hjlLcEAdPBKZj5vlRU4xAo5HGbvOHHvZsXvfYnoEAfLHnGSuv/PEcO7SMgvnVY4RrafZnRvQvevU4tAoflR2JjcI%2BR%2BpIZxWG8fULifZgYnY4qfMUEU/Vuvmfa72fYLJvrfdSVM4f/Hkfm40fPvZfFzi29IgfVMBfsKH2R%2BQx9hEzhF4M2Bffk0nvYk3vsfAxVAEAI/Ff4/E/lMU/jUthc/8/PZ2MlFS/YfuoDvEfa/UIG/BOKouWO/BIxco/ZxbrDXk/GIRImYAOa4XSNfykLu97%2Bj/Oum/3L5j84U%2BfDEH4UWKIVb8dwYAbf375gCY%2BBOFimJHgGXk3A1fHDt%2Bl37QCv%2BzvKcBRCUZpgwgzle8EOXhD%2BVe%2BN/UAVH3X4YD4uBAgPgkxX4D8DAQ/WPhEEhbsDI40zLgegJ95Pc%2BuggqLrx3FjCDGBd/ZgQ/wwHiD8egYMcFF0iwyC9eQDErkFGdB49OBcgtAQoMf7KCa%2Bagm0CV00GM8q2ruVxpRRL72gIB48BcCv2F5Tkx44YfHJh0iq5txqCTKmB4KNyYdfBZgaZgEIJDhhTSXVYIACFCH%2BDKYHg1Bl1TMyut4hFMDwaGS6oRA7gfg4DlWzvYPsDWBLLvq2zN5ydnec7ENpm3UpHBwQJXR%2BCbwZa9s6hePcMOCBMZ69KhVyI3kchKGMgyhwHWZr6VA5GQOAKwWgJwASC8A/AHALQKQFQCcBq0lgawMGB5D9gdgPAUgGq1mFjCVgnoAIGnHDAJARwGgAAJwjhThXABIAAA5JAI4SQGYASD6BOAkgaYZoF4ALCOAvAXDskG2FaAVgcAWADAEQAoBUA68egGQAoB2DwRwwYAFwDMC/gaA6qOILh1fjvDSA0QMIE0EsrcBeAa8KSgQDIiIMcRcwrACSCMDiAdhpAfAPSG6AphcOVI2eF0D1xbA5hAmCYVSOvBdZsRHgLAOiIGQsBOAPAFYFQAMD8d3wFCMiPuBmGbD%2BAggEQGIHYBSAZAggRQCoHUBUjdAv4AwOIlMDLDLA%2BgPANEFw6wAasW0f5qQGorQwSIDAT0OBnmGqQecnAb4TUG0hZAXAF4SYH4F/AhA5gZQCoHoDSAZAyQXowMYUDJADB/RwwX8J0G7i9AJgngNoHoFjFkh4xswUoEMHiAxiZgoY7MX0EjGZiJAKwBQGsKVFbD6QWwYUc8I4BTDSAMwuYZ8NUDXC04/ENOJIEODABkA/ieEeGC4C2IlhVgA0YcFwCEBXcGwxkB4BhGNN/AXAJYLwD%2BG7DSAhoZnMMB/SkB9hCQfwOGFOGSAuAXADQFwFOH%2BAEgCQU4WnDTjHjqxrw0gIKKuG9jNx5wjQNcLww3CNAlVNOHWPRGfDvhIAX4e8IBHAiIASAJkXDh0jkBKA%2B5BQMoEMA1AhACAURDKLxFgiUgdAYwtBPCC0A4JCE9EWvBQkQiQAcIhEaQFwkbxiAZEPXFhIRD1jeAIEh4NGwZE0TVAXQBoPgBmG8A5RwgUQOIGVEcS1RagdEVqP0CGBjA1gawIaONHwBrk/Kc0dJCtEwxbR9ohxofGdGrB1gmwZMRpDCDoTYJ8EqiUKPnEVj9JpAF%2BvpPGGTC3hVIz4VxGZE6RDgTYlsW2I7FdjDgPYvsRAAHFiThx%2BAWyRsLnFbD/xKwZcU6EoBmSOA14wUbcPDC3CM4ZgS4SOH8C7jzhI4T8ZZJUk/D/JOw%2B0fsIvHhgHhpwt8U%2BN3EJBHhI4ZKRyP8AWSGxKkhcfaI5FmBKpHw6qQFMtEoj3RkgIAA%3D%3D%3D

#include <iostream>
#include <sstream>
#include <vector>

#include "fmt/core.h"

struct NoLogs {
    static constexpr void debug(std::string_view) {}
    static constexpr void info(std::string_view) {}
    static constexpr void warn(std::string_view) {}
    static constexpr void error(std::string_view) {}
    static constexpr void fatal(std::string_view) {}
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
    void fatal(std::string_view msg) const {
        fmt::print(fmt_str, fmt::arg("msg", msg), fmt::arg("level", "FATAL"));
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
    static void fatal(std::string_view msg) {
        std::cout << "CoutLogs: FATAL: " << msg << '\n';
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
    void fatal(std::string_view msg) const {
        os << "OstreamLogs: FATAL: " << msg << '\n';
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
        log.fatal("NE");
        fmt::print("{}\n", bar);
    }

    fmt::print("Inside strstream:\n{}", strstream.str());

    fmt::print("{}\n", bar);
    return 0;
}

*/
} // namespace log
} // namespace arthoolbox

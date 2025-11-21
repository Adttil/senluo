#ifndef SENLUO_MATH_HPP
#define SENLUO_MATH_HPP

#include <cmath>

#include "adaptor.hpp"

#include "macro_define.hpp"

//Exponential functions
namespace senluo 
{
    namespace detail::exp_fn_ns
    {
        void exp();

        struct exp_fn : adaptor_closure<exp_fn>
        {
            template<class T>
            constexpr decltype(auto) operator()(T&& t) const
            {
                using std::exp;
                return exp(FWD(t));
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::exp_fn_ns::exp_fn exp{};
    }

    namespace detail::log_fn_ns
    {
        void log();

        struct log_fn : adaptor_closure<log_fn>
        {
            template<class T>
            constexpr decltype(auto) operator()(T&& t) const
            {
                using std::log;
                return log(FWD(t));
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::log_fn_ns::log_fn log{};
    }

    namespace detail::log10_fn_ns
    {
        void log10();

        struct log10_fn : adaptor_closure<log10_fn>
        {
            template<class T>
            constexpr decltype(auto) operator()(T&& t) const
            {
                using std::log10;
                return log10(FWD(t));
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::log10_fn_ns::log10_fn log10{};
    }
}

//Power functions
namespace senluo 
{
    namespace detail::pow_fn_ns
    {
        void pow();

        struct pow_fn : adaptor_closure<pow_fn>
        {
            template<class T, class U>
            constexpr decltype(auto) operator()(T&& t, U&& u) const
            {
                using std::pow;
                return pow(FWD(t), FWD(u));
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::pow_fn_ns::pow_fn pow{};
    }

    namespace detail::sqrt_fn_ns
    {
        void sqrt();

        struct sqrt_fn : adaptor_closure<sqrt_fn>
        {
            template<class T>
            constexpr decltype(auto) operator()(T&& t) const
            {
                using std::sqrt;
                return sqrt(FWD(t));
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::sqrt_fn_ns::sqrt_fn sqrt{};
    }
}

//Trigonometric functions
namespace senluo 
{
    namespace detail::sin_fn_ns
    {
        void sin();

        struct sin_fn : adaptor_closure<sin_fn>
        {
            template<class T>
            constexpr decltype(auto) operator()(T&& t) const
            {
                using std::sin;
                return sin(FWD(t));
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::sin_fn_ns::sin_fn sin{};
    }

    namespace detail::cos_fn_ns
    {
        void cos();

        struct cos_fn : adaptor_closure<cos_fn>
        {
            template<class T>
            constexpr decltype(auto) operator()(T&& t) const
            {
                using std::cos;
                return cos(FWD(t));
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::cos_fn_ns::cos_fn cos{};
    }

    namespace detail::tan_fn_ns
    {
        void tan();

        struct tan_fn : adaptor_closure<tan_fn>
        {
            template<class T>
            constexpr decltype(auto) operator()(T&& t) const
            {
                using std::tan;
                return tan(FWD(t));
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::tan_fn_ns::tan_fn tan{};
    }

    namespace detail::asin_fn_ns
    {
        void asin();

        struct asin_fn : adaptor_closure<asin_fn>
        {
            template<class T>
            constexpr decltype(auto) operator()(T&& t) const
            {
                using std::asin;
                return asin(FWD(t));
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::asin_fn_ns::asin_fn asin{};
    }

    namespace detail::acos_fn_ns
    {
        void acos();

        struct acos_fn : adaptor_closure<acos_fn>
        {
            template<class T>
            constexpr decltype(auto) operator()(T&& t) const
            {
                using std::acos;
                return acos(FWD(t));
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::acos_fn_ns::acos_fn acos{};
    }

    namespace detail::atan_fn_ns
    {
        void atan();

        struct atan_fn : adaptor_closure<atan_fn>
        {
            template<class T>
            constexpr decltype(auto) operator()(T&& t) const
            {
                using std::atan;
                return atan(FWD(t));
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::atan_fn_ns::atan_fn atan{};
    }
}

//Hyperbolic functions
namespace senluo 
{
    namespace detail::sinh_fn_ns
    {
        void sinh();

        struct sinh_fn : adaptor_closure<sinh_fn>
        {
            template<class T>
            constexpr decltype(auto) operator()(T&& t) const
            {
                using std::sinh;
                return sinh(FWD(t));
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::sinh_fn_ns::sinh_fn sinh{};
    }

    namespace detail::cosh_fn_ns
    {
        void cosh();

        struct cosh_fn : adaptor_closure<cosh_fn>
        {
            template<class T>
            constexpr decltype(auto) operator()(T&& t) const
            {
                using std::cosh;
                return cosh(FWD(t));
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::cosh_fn_ns::cosh_fn cosh{};
    }

    namespace detail::tanh_fn_ns
    {
        void tanh();

        struct tanh_fn : adaptor_closure<tanh_fn>
        {
            template<class T>
            constexpr decltype(auto) operator()(T&& t) const
            {
                using std::tanh;
                return tanh(FWD(t));
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::tanh_fn_ns::tanh_fn tanh{};
    }

    namespace detail::asinh_fn_ns
    {
        void asinh();

        struct asinh_fn : adaptor_closure<asinh_fn>
        {
            template<class T>
            constexpr decltype(auto) operator()(T&& t) const
            {
                using std::asinh;
                return asinh(FWD(t));
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::asinh_fn_ns::asinh_fn asinh{};
    }

    namespace detail::acosh_fn_ns
    {
        void acosh();

        struct acosh_fn : adaptor_closure<acosh_fn>
        {
            template<class T>
            constexpr decltype(auto) operator()(T&& t) const
            {
                using std::acosh;
                return acosh(FWD(t));
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::acosh_fn_ns::acosh_fn acosh{};
    }

    namespace detail::atanh_fn_ns
    {
        void atanh();

        struct atanh_fn : adaptor_closure<atanh_fn>
        {
            template<class T>
            constexpr decltype(auto) operator()(T&& t) const
            {
                using std::atanh;
                return atanh(FWD(t));
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::atanh_fn_ns::atanh_fn atanh{};
    }
}

namespace senluo
{
    namespace detail::is_elementary_fn_ns
    {
        void is_elementary();

        struct is_elementary_fn : adaptor_closure<is_elementary_fn>
        {
            template<class T>
            consteval bool operator()(std::type_identity<T>) const
            {
                if constexpr(requires{ requires (not T::is_elementary(custom_t{})); })
                {
                    return false;
                }
                else if constexpr(requires{ requires (not is_elementary(std::type_identity<T>{}, custom_t{})); })
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::is_elementary_fn_ns::is_elementary_fn is_elementary{};
    }

    template<class T>
    concept elementary = is_elementary(std::type_identity<std::remove_cvref_t<T>>{});
}

#include "macro_undef.hpp"
#endif
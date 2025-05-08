#ifndef SENLUO_MATH_HPP
#define SENLUO_MATH_HPP

#include <cmath>

#include "adaptor.hpp"

#include "macro_define.hpp"

namespace senluo 
{
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
}

#include "macro_undef.hpp"
#endif
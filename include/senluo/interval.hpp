#ifndef SENLUO_INTERVAL_HPP
#define SENLUO_INTERVAL_HPP

#include <functional>
#include <utility>

#include "tools/constant.hpp"
#include "tools/math.hpp"
#include "core.hpp"
#include "vec.hpp"

#include "tools/macro_define.hpp"

namespace senluo
{
    template<class T>
    struct interval;

    template<class T>
    interval(T) -> interval<T>;

    namespace detail 
    {
        struct as_interval_fn : tree_adaptor_closure<as_interval_fn>
        {
            template<class T>
            constexpr interval<T> operator()(T&& t) const noexcept
            {
                return { FWD(t) };
            }

            template<class T>
            constexpr interval<T&> operator()(interval<T>& t) const noexcept
            {
                return { t.raw_base() };
            }

            template<class T>
            constexpr interval<const T&> operator()(const interval<T>& t) const noexcept
            {
                return { t.raw_base() };
            }

            template<class T>
            constexpr interval<T> operator()(interval<T>&& t) const noexcept
            {
                return { FWD(t).raw_base() };
            }

            template<class T>
            constexpr interval<T> operator()(const interval<T>&& t) const noexcept
            {
                return { FWD(t).raw_base() };
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::as_interval_fn as_interval{};
    }

    template<class T>
    struct interval : based_on<T>
    {
        template<size_t I, unwarp_derived_from<interval> Self>
        friend constexpr decltype(auto) tree_get(Self&& self)
        {
            return tree_get<I>(FWD(self).unwrap_base());
        }

        template<class U, class Self> requires (size<T> == size<U>)
        explicit constexpr operator interval<U>(this Self&& self)
        {
            return interval<U>{ FWD(self).raw_base() | make<U> };
        }

        constexpr decltype(auto) operator[](this auto&& self, size_t i)
        noexcept(noexcept(FWD(self).raw_base()[i]))
        requires requires{FWD(self).raw_base()[i];}
        {
            return FWD(self).raw_base()[i];
        }

        constexpr decltype(auto) min(this auto&& self)
        {
            return tree_get<0uz>(FWD(self).raw_base());
        }

        constexpr decltype(auto) max(this auto&& self)
        {
            return tree_get<1uz>(FWD(self).raw_base());
        }

        template<class U>
        constexpr bool contains(const U& value) const
        {
            return value >= min() && value < max();
        }
    };

    template<class T>
    struct tree_size<interval<T>>
    {
        static constexpr size_t value = size<T>;
    };
}

#include "tools/macro_undef.hpp"
#endif
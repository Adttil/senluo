#ifndef SENLUO_RECT_HPP
#define SENLUO_RECT_HPP

#include <functional>
#include <utility>

#include "tools/constant.hpp"
#include "tools/math.hpp"
#include "core.hpp"
#include "vec.hpp"
#include "mat.hpp"
#include "interval.hpp"

#include "tools/macro_define.hpp"

namespace senluo
{
    template<class T>
    struct rect;

    template<class T>
    rect(T) -> rect<T>;

    namespace detail 
    {
        struct as_rect_fn : adaptor_closure<as_rect_fn>
        {
            template<class T>
            constexpr rect<T> operator()(T&& t) const noexcept
            {
                return { FWD(t) };
            }

            template<class T>
            constexpr rect<T&> operator()(rect<T>& t) const noexcept
            {
                return { t.raw_base() };
            }

            template<class T>
            constexpr rect<const T&> operator()(const rect<T>& t) const noexcept
            {
                return { t.raw_base() };
            }

            template<class T>
            constexpr rect<T> operator()(rect<T>&& t) const noexcept
            {
                return { FWD(t).raw_base() };
            }

            template<class T>
            constexpr rect<T> operator()(const rect<T>&& t) const noexcept
            {
                return { FWD(t).raw_base() };
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::as_rect_fn as_rect{};
    }

    template<class T>
    struct rect : based_on<T>
    {
        template<size_t I, unwarp_derived_from<rect> Self>
        friend constexpr decltype(auto) tree_get(Self&& self)
        {
            return tree_get<I>(FWD(self).unwrap_base()) | as_interval;
        }

        template<class U, class Self> requires (size<T> == size<U>)
        explicit constexpr operator rect<U>(this Self&& self)
        {
            return rect<U>{ FWD(self).raw_base() | make<U> };
        }

        constexpr decltype(auto) operator[](this auto&& self, size_t i)
        noexcept(noexcept(FWD(self).raw_base()[i]))
        requires requires{FWD(self).raw_base()[i];}
        {
            return FWD(self).raw_base()[i];
        }

        constexpr decltype(auto) min(this auto&& self)
        {
            return (FWD(self).raw_base() | refer | as_mat).template r<0>();
        }

        constexpr decltype(auto) max(this auto&& self)
        {
            return (FWD(self).raw_base() | refer | as_mat).template r<1>();
        }

        template<class U> requires (size<U> == size<T>)
        constexpr bool contains(const U& point) const
        {
            return [&]<size_t...I>(std::index_sequence<I...>)
            { 
                return (... && tree_get<I>(*this).contains(tree_get<I>(point)));
            }(std::make_index_sequence<size<T>>{});
        }
    };

    template<class T>
    struct tree_size<rect<T>>
    {
        static constexpr size_t value = size<T>;
    };
}

#include "tools/macro_undef.hpp"
#endif
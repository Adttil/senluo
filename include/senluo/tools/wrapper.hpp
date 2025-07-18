#ifndef SENLUO_WRAPPER_HPP
#define SENLUO_WRAPPER_HPP

#include <utility>
#include <type_traits>

#include "general.hpp"

#include "macro_define.hpp"

namespace senluo
{
    template<class T>
    struct ideal_unwrap
     : std::conditional<std::is_rvalue_reference_v<T&&>, std::remove_reference_t<T>, T>
    {};

    template<class T>
    using ideal_unwrap_t = ideal_unwrap<T>::type;

    template<class T>
    using ideal_store_t = std::remove_const_t<ideal_unwrap_t<T>>;

    template<class T, class U>
    concept unwarp_derived_from = derived_from<ideal_unwrap_t<T>, U>;

    template<class T>
    struct wrapper
    {
        using value_type = T;

        T value_;

        constexpr T& get()& noexcept
        {
            return value_;
        }

        constexpr const T& get()const& noexcept
        {
            return value_;
        }

        constexpr T&& get()&& noexcept
        {
            return (T&&)value_;
        }

        constexpr const T&& get()const&& noexcept
        {
            return (const T&&)value_;
        }

        template<class Self>
        constexpr ideal_store_t<Self> value(this Self&& self) 
            noexcept(noexcept((ideal_store_t<Self>)std::forward<Self>(self).get()))
        {
            return std::forward<Self>(self).get();
        }

        template<class Self>
        constexpr ideal_store_t<Self> materialize(this Self&& self, custom_t = {}) 
            noexcept(noexcept((ideal_store_t<Self>)std::forward<Self>(self).get()))
        {
            return std::forward<Self>(self).get();
        }

#ifdef _MSC_VER
        constexpr operator ideal_store_t<wrapper&>()&
        AS_EXPRESSION(value())

        constexpr operator ideal_store_t<const wrapper&>()const&
        AS_EXPRESSION(value())

        constexpr operator ideal_store_t<wrapper&&>()&&
        AS_EXPRESSION(std::move(*this).value())

        constexpr operator ideal_store_t<const wrapper&&>()const&&
        AS_EXPRESSION(std::move(*this).value())

#else

        template<class Self>
        constexpr operator ideal_store_t<Self>(this Self&& self)
            noexcept(noexcept((ideal_store_t<Self>)std::forward<Self>(self).value()))
        {
            return std::forward<Self>(self).value();
        }

#endif

        friend bool operator==(const wrapper& l, const wrapper& r)
        AS_EXPRESSION(
            l.get() == r.get()
        )
    };

    template<class T>
    struct ideal_unwrap<wrapper<T>>
     : std::type_identity<T>
    {};

    template<class T>
    struct ideal_unwrap<const wrapper<T>>
     : std::type_identity<const T>
    {};

    template<class T>
    struct ideal_unwrap<wrapper<T>&> : std::type_identity<T&> {};

    template<class T>
    struct ideal_unwrap<const wrapper<T>&> : std::type_identity<const T&> {};

    template<class T>
    struct ideal_unwrap<wrapper<T>&&>
     : std::type_identity<T>
    {};

    template<class T>
    struct ideal_unwrap<const wrapper<T>&&>
     : std::type_identity<const T>
    {};

    namespace detail
    {
        struct unwrap_fwd_fn
        {
            template<class T>
            static constexpr T& operator()(wrapper<T>& t) noexcept
            {
                return t.get();
            }

            template<class T>
            static constexpr const T& operator()(const wrapper<T>& t) noexcept
            {
                return t.get();
            }

            template<class T>
            static constexpr decltype(auto) operator()(wrapper<T>&& t) noexcept
            {
                return std::move(t).get();
            }

            template<class T>
            static constexpr decltype(auto) operator()(const wrapper<T>&& t) noexcept
            {
                return std::move(t).get();
            }

            template<class T>
            static constexpr T&& operator()(T&& t) noexcept
            {
                return std::forward<T>(t);
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::unwrap_fwd_fn unwrap_fwd{};
    }
}

#include "macro_undef.hpp"
#endif
#ifndef SENLUO_MAKE_HPP
#define SENLUO_MAKE_HPP

#include <tuple>
#include <utility>

#include "../tools/array.hpp"
#include "../tools/adaptor.hpp"
#include "../tools/tuple.hpp"
#include "tree.hpp"

#include "../tools/macro_define.hpp"

namespace senluo
{
    namespace detail 
    {
        template<class T>
        struct tuple_make_fn;
    }

    inline namespace functors
    {
        template<class T>
        inline constexpr detail::tuple_make_fn<T> tuple_make{};
    }

    namespace detail::make_from_ns
    {
        enum class strategy_t
        {
            none,
            convert,
            member_make,
            adl_make
        };

        void make_from();

        template<class...T, class Src>
        constexpr std::tuple<T...> make_from(std::type_identity<std::tuple<T...>>, Src&& src, custom_t = {})
        {
            return tuple_make<std::tuple<T...>>(FWD(src));
        }

        template<class T1, class T2, class Src>
        constexpr std::pair<T1, T2> make_from(std::type_identity<std::pair<T1, T2>>, Src&& src, custom_t = {})
        {
            return tuple_make<std::pair<T1, T2>>(FWD(src));
        }

#if __has_include(<array>)
        template<class T, size_t N, class Src>
        constexpr std::array<T, N> make_from(std::type_identity<std::array<T, N>>, Src&& src, custom_t = {})
        {
            return tuple_make<std::array<T, N>>(FWD(src));
        }
#endif

#ifndef SENLUO_USE_STD_ARRAY
        template<class T, size_t N, class Src>
        constexpr array<T, N> make_from(std::type_identity<array<T, N>>, Src&& src, custom_t = {})
        {
            return tuple_make<array<T, N>>(FWD(src));
        }
#endif

        struct make_from_fn : adaptor_closure<make_from_fn>
        {
            template<class T, class U>
            static consteval choice_t<strategy_t> choose()
            {
                if constexpr((terminal<U> || terminal<T> || std::same_as<std::remove_cvref_t<U>, T>)
                    && requires{ (T)std::declval<U>(); })
                {
                    return { strategy_t::convert, noexcept((T)std::declval<U>()) };
                }
                else if constexpr(requires{ T::make_from(std::declval<U>(), custom_t{}); })
                {
                    return { strategy_t::member_make, noexcept(T::make_from(std::declval<U>(), custom_t{})) };
                }
                else if constexpr(requires{ make_from(std::type_identity<T>{}, std::declval<U>(), custom_t{}); })
                {
                    return { strategy_t::adl_make, noexcept(make_from(std::type_identity<T>{}, std::declval<U>(), custom_t{})) };
                }
                else
                {
                    return { strategy_t::none };
                }
            }

            template<class T, class U>
            static constexpr choice_t<strategy_t> choice = choose<T, U>();

            template<class T, class U> requires (choice<T, U>.strategy != strategy_t::none)
            constexpr T operator()(std::type_identity<T>, U&& u) const noexcept(choice<T, U>.nothrow)
            {
                constexpr strategy_t strategy = choice<T, U>.strategy;
                if constexpr(strategy == strategy_t::convert)
                {
                    return (T)FWD(u);
                }
                else if constexpr(strategy == strategy_t::member_make)
                {
                    return T::make_from(FWD(u), custom_t{});
                }
                else if constexpr(strategy == strategy_t::adl_make)
                {
                    return make_from(std::type_identity<T>{}, FWD(u), custom_t{});
                }
                else
                {
                    static_assert(false, "Invalid strategy for make");
                }
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::make_from_ns::make_from_fn make_from{};
    }

    namespace detail 
    {
        template<class T>
        struct make_fn : adaptor_closure<make_fn<T>>
        {
            template<class U>
            static constexpr T operator()(U&& u)
            AS_EXPRESSION(
                make_from(std::type_identity<T>{}, FWD(u))
            )
        };
    }

    inline namespace functors
    {
        template<class T>
        inline constexpr detail::make_fn<T> make{};
    }

    namespace detail 
    {
        template<class T>
        struct tuple_make_fn
        {
            template<class U, size_t...I>
            static constexpr T impl(U&& u, std::index_sequence<I...>)
            AS_EXPRESSION(
                T{ tree_get<I>(FWD(u)) | make<std::tuple_element_t<I, T>>... }
            )

            template<class U>
            static constexpr T operator()(U&& u)
            AS_EXPRESSION(
                impl(FWD(u), std::make_index_sequence<std::tuple_size_v<T>>{})
            )
        }; 
    }
}

namespace senluo::tuple_ns
{
    template<class...T, class Src>
    constexpr tuple<T...> make_from(std::type_identity<tuple<T...>>, Src& src, custom_t = {})
    {
        return tuple_make<tuple<T...>>(src);
    }
}

#include "../tools/macro_undef.hpp"
#endif // SENLUO_MAKE_HPP
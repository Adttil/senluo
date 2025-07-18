#ifndef SENLUO_MAKE_HPP
#define SENLUO_MAKE_HPP

#include "../tools/adaptor.hpp"
#include "tree.hpp"
#include "pretreat.hpp"

#include "../tools/macro_define.hpp"

namespace senluo
{
    template<class T>
    struct maker
    {
        template<class U>
        static constexpr T make_from(U&& u)
        AS_EXPRESSION(
            T::make_from(FWD(u))
        )
    };

    namespace detail 
    {
        enum class make_strategy_t
        {
            none,
            convert,
            make
        };

        template<size_t I, class T>
        struct sub_make_fn : tree_adaptor_closure<sub_make_fn<I, T>>
        {
            using strategy_t = make_strategy_t;

            template<class U>
            static consteval choice_t<strategy_t> choose()
            {
                if constexpr((terminal<tree_get_t<I, U>> || terminal<T> || std::same_as<std::remove_cvref_t<tree_get_t<I, U>>, T>)
                    && requires{ (T)tree_get<I>(std::declval<U>()); })
                {
                    return { strategy_t::convert, noexcept((T)tree_get<I>(std::declval<U>())) };
                }
                else if constexpr(requires{ { maker<T>::make_from(tree_get<I>(std::declval<U>())) } -> std::same_as<T>; })
                {
                    return {  strategy_t::make, noexcept(maker<T>::make_from(tree_get<I>(std::declval<U>()))) };
                }
                else
                {
                    return { strategy_t::none, false };
                }
            }

            template<class U>
            static constexpr choice_t<strategy_t> choice = choose<U>();

            template<class U> requires (choice<U>.strategy != strategy_t::none)
            constexpr T operator()(U&& u) const noexcept(choice<U>.nothrow)
            {
                constexpr strategy_t strategy = choice<U>.strategy;
                if constexpr(strategy == strategy_t::convert)
                {
                    return (T)tree_get<I>(FWD(u));
                }
                else if constexpr(strategy == strategy_t::make)
                {
                    return maker<T>::make_from(tree_get<I>(FWD(u)));
                }
                else
                {
                    static_assert(false, "Invalid strategy for sub_make");
                }
            }
        };

        template<class T>
        struct make_fn : tree_adaptor_closure<make_fn<T>>
        {
            using strategy_t = make_strategy_t;

            template<class U>
            static consteval choice_t<strategy_t> choose()
            {
                if constexpr((terminal<U> || terminal<T> || std::same_as<std::remove_cvref_t<U>, T>)
                    && requires{ (T)std::declval<U>(); })
                {
                    return { strategy_t::convert, noexcept((T)std::declval<U>()) };
                }
                else if constexpr(requires{ maker<T>::make_from(std::declval<U>()); })
                {
                    return { strategy_t::make, noexcept(maker<T>::make_from(std::declval<U>())) };
                }
                else
                {
                    return { strategy_t::none };
                }
            }

            template<class U>
            static constexpr choice_t<strategy_t> choice = choose<U>();

            template<class U> requires (choice<U>.strategy != strategy_t::none)
            constexpr T operator()(U&& u) const noexcept(choice<U>.nothrow)
            {
                constexpr strategy_t strategy = choice<U>.strategy;
                if constexpr(strategy == strategy_t::convert)
                {
                    return (T)FWD(u);
                }
                else if constexpr(strategy == strategy_t::make)
                {
                    return maker<T>::make_from(FWD(u));
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
        template<size_t I, class T>
        inline constexpr detail::sub_make_fn<I, T> sub_make{};

        template<class T>
        inline constexpr detail::make_fn<T> make{};
    }

    template<class T>
    struct tuple_maker
    {
        template<class U, size_t...I>
        static constexpr T make_from_impl(U&& u, std::index_sequence<I...>)
        AS_EXPRESSION(
            T{ FWD(u) | sub_make<I, std::tuple_element_t<I, T>>... }
        )

        template<class U>
        static constexpr T make_from(U&& u)
        AS_EXPRESSION(
            make_from_impl(FWD(u) | refer | sequence, std::make_index_sequence<std::tuple_size_v<T>>{})
        )
    };
}

namespace senluo
{
    template<class T, size_t N>
    struct maker<array<T, N>> : tuple_maker<array<T, N>>{};

    template<class...T>
    struct maker<tuple<T...>> : tuple_maker<tuple<T...>>{};

    template<class F, class S>
    struct maker<std::pair<F, S>> : tuple_maker<std::pair<F, S>>{};
}

#include "../tools/macro_undef.hpp"
#endif // SENLUO_MAKE_HPP
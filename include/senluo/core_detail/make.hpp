#ifndef RUZHOUXIE_MAKE_HPP
#define RUZHOUXIE_MAKE_HPP

#include "../tree.hpp"
#include "../general.hpp"
#include "principle.hpp"

#include "../macro_define.hpp"

namespace senluo 
{
    namespace detail::make_t_ns
    {
        template<typename T, indexical_array auto indexes>
        struct make_t;
    }

    template<typename T, indexical auto...indexes>
    inline constexpr auto make = detail::make_t_ns::make_t<T, to_indexes(indexes...)>{};
    
    template<typename T>
    struct sequence_maker
    {
        template<typename Arg>
        constexpr T operator()(Arg&& arg)const
        {
            return [&]<size_t...I>(std::index_sequence<I...>)
            {
                auto&& seq = FWD(arg) | refer | sequence;
                return T{ FWD(seq) | make<std::tuple_element_t<I, T>, I>... };
            }(std::make_index_sequence<std::tuple_size_v<T>>{});
        }
    };

    template<typename T>
    struct inverse_sequence_maker
    {
        template<typename Arg>
        constexpr T operator()(Arg&& arg)const
        {
            return [&]<size_t...I>(std::index_sequence<I...>)
            {
                auto&& seq = FWD(arg) | refer | inverse_sequence;
                return T{ FWD(seq) | make<std::tuple_element_t<I, T>, I>... };
            }(std::make_index_sequence<std::tuple_size_v<T>>{});
        }
    };

    template<typename T>
    struct children_maker
    {
        template<typename Arg>
        constexpr T operator()(Arg&& arg)const
        {
            return [&]<size_t...I>(std::index_sequence<I...>)
            {
                auto&& seq = FWD(arg) | refer | seperate;
                return T{ FWD(seq) | make<std::tuple_element_t<I, T>, I>... };
            }(std::make_index_sequence<std::tuple_size_v<T>>{});
        }
    };

    namespace detail
    {
        template<class L, class CL, class R, class RL>
        struct storage_type;

        template<class T>
        struct storage_type<T&, const T&, T&&, const T&&>
        {
            using type = T;
        };

        template<class T>
        struct storage_type<T, T, T, T>
        {
            using type = T;
        };

        template<class T>
        struct storage_type<T&, T&, T&, T&>
        {
            using type = T&;
        };

        template<class T>
        struct storage_type<const T&, const T&, const T&, const T&>
        {
            using type = const T&;
        };

        template<class T>
        struct storage_type<T&, T&, T&&, T&&>
        {
            using type = T&&;
        };

        template<class T>
        struct storage_type<const T&, const T&, const T&&, const T&&>
        {
            using type = const T&&;
        };

        template<size_t I, class T>
        struct tuple_element_by_child
        : storage_type<subtree_t<T&, I>, subtree_t<const T&, I>, subtree_t<T&&, I>, subtree_t<const T&&, I>>
        {};

        template<size_t I, class T>
        using tuple_element_t_by_child = tuple_element_by_child<I, T>::type;

        template<class T>
        consteval bool is_valid_for_tuple_element_by_child()
        {
            return []<size_t...I>(std::index_sequence<I...>)
            {
                return (... && requires{ typename tuple_element_by_child<I, T>::type; });
            }(std::make_index_sequence<size<T>>{});
        }
    }

    template<typename T>
    struct aggregate_maker
    {
        template<typename Arg>
        constexpr T operator()(Arg&& arg)const
        {
            return [&]<size_t...I>(std::index_sequence<I...>)
            {
                auto&& seq = FWD(arg) | refer | sequence;
                return T{ { FWD(seq) | make<detail::tuple_element_t_by_child<I, T>, I> }... };
            }(std::make_index_sequence<size<T>>{});
        }
    };

    namespace detail
    {
        template<class T>
        constexpr bool is_aggregate_tree()
        {
            return []<size_t...I>(std::index_sequence<I...>) -> bool
            {
                return (... && (detail::get_ns::get_t<I>::template choose<T>().strategy == detail::get_ns::strategy_t::aggregate));
            }(std::make_index_sequence<size<T>>{});
        }
    }

    template<class T>
    concept aggregate_tree = std::is_aggregate_v<std::remove_cvref_t<T>> && detail::is_aggregate_tree<T>();

    namespace detail::make_t_ns
    {        
        // template<typename T>
        // constexpr auto get_maker(type_tag<T>)noexcept
        // {
        //     if constexpr(aggregate_tree<T>)
        //     {
        //         return aggregate_maker<T>{};
        //     }
        //     else if constexpr(requires{ std::tuple_size<T>::value; })
        //     {
        //         return children_maker<T>{};
        //     }
        //     else
        //     {

        //     }
        // }
    }

    template<typename T, indexical_array auto indexes>
    struct detail::make_t_ns::make_t : adaptor_closure<make_t<T, indexes>>
    {
        template<typename Arg>
        constexpr T operator()(Arg&& arg)const
        {
            if constexpr(terminal<subtree_t<Arg, indexes>> || terminal<T>)
            {
                return FWD(arg) | subtree<indexes>;
            }
            else if constexpr(std::same_as<std::remove_cvref_t<subtree_t<Arg, indexes>>, T> && requires{ T{ FWD(arg) | subtree<indexes> }; })
            {
                return FWD(arg) | subtree<indexes>;
            }
            else if constexpr(requires{ get_maker(type_tag<T>{}); })
            {
                return get_maker(type_tag<T>{})(FWD(arg) | subtree<indexes>);
            }
            else if constexpr(aggregate_tree<T>)
            {
                return aggregate_maker<T>{}(FWD(arg) | subtree<indexes>);
            }
            else if constexpr(requires{ std::tuple_size<T>::value; })
            {
                return children_maker<T>{}(FWD(arg) | subtree<indexes>);
            }
            else
            {
                static_assert(false, "maker for T not found.");
            }
        }
    };
}

namespace senluo 
{
    template<class To>
    constexpr const auto& to()
    {
        return make<To>;
    }
}

namespace senluo 
{
    template<class...T>
    constexpr auto get_maker(type_tag<tuple<T...>>)
    {
        return sequence_maker<tuple<T...>>{};
    }

    template<class...T>
    constexpr auto get_maker(type_tag<std::tuple<T...>>)
    {
        return inverse_sequence_maker<std::tuple<T...>>{};
    }

    template<class T, size_t N>
    constexpr auto get_maker(type_tag<array<T, N>>)
    {
        return sequence_maker<array<T, N>>{};
    }
}

#include "../macro_undef.hpp"
#endif
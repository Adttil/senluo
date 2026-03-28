#ifndef SENLUO_SUBVIEW_HPP
#define SENLUO_SUBVIEW_HPP

#include <tuple>

#include "../tools/general.hpp"
#include "../tools/array.hpp"
#include "../tools/tuple.hpp"
#include "../tools/adaptor.hpp"

#include "../tools/macro_define.hpp"

namespace senluo::detail
{
    constexpr auto&& min(auto&& arg0, auto&&...rest)
    {
        if constexpr(sizeof...(rest) == 0)
        {
            return FWD(arg0);
        }
        else
        {
            auto&& rest_min = detail::min(FWD(rest)...);
            if(arg0 < rest_min)
            {
                return FWD(arg0);
            }
            else
            {
                return FWD(rest_min);
            }
        }
    }

    template<class T>
    concept indexical_array = requires(std::remove_cvref_t<T> t, size_t i)
    {
        t[i];
        requires std::integral<std::remove_cvref_t<decltype(t[i])>>;
    };

    constexpr size_t normalize_index(std::integral auto index, size_t size)noexcept
    {
        if(index >= 0)
        {
            return static_cast<size_t>(index % size);
        }
        else
        {
            return static_cast<size_t>((size - -index % size) % size);
        }
    }

    template<class...TIndexes>
    constexpr auto to_indexes(const TIndexes&...indexes)noexcept
    {
        if constexpr(sizeof...(TIndexes) == 0uz)
        {
            return array<size_t, 0uz>{};
        }
        else if constexpr(sizeof...(indexes) > 1uz)
        {
            return detail::array_cat(detail::to_indexes(indexes)...);
        }
        else if constexpr(requires{ (..., indexes[0]); })
        {
            return (..., indexes);
        }
        else
        {
            return array{ indexes... };
        }
    }
}

namespace senluo
{
    template<class T>
    concept indexical = std::integral<T> || detail::indexical_array<T>;

    inline constexpr array<size_t, 0uz> indexes_of_whole{};

    inline constexpr size_t auto_supported_aggregate_max_size = 16uz;

    namespace detail
    {
        struct universal_type
        {
            universal_type(size_t){}

            //Can not use "requires" in clang here.
            //https://github.com/llvm/llvm-project/issues/76415
            template <class T, class = std::enable_if_t<std::is_copy_constructible_v<T>>>
            operator T&();

            template <class T, class = std::enable_if_t<std::is_move_constructible_v<T>>>
            operator T&&();

            template <class T, class = std::enable_if_t<!std::is_copy_constructible_v<T> && !std::is_move_constructible_v<T>>>
            operator T();
        };

        template<class T, size_t N>
        constexpr bool aggregate_member_count_is()
        {
            return []<size_t...I>(std::index_sequence<I...>){
                return requires{ T{ {universal_type{0uz}}, universal_type{I}... }; };
            }(std::make_index_sequence<N - 1>{});
        }

        template <class T>
        inline constexpr size_t aggregate_member_count = []<size_t...I>(std::index_sequence<I...>){
            auto tags = array{ aggregate_member_count_is<T, I + 1>()... };
            for(size_t i = auto_supported_aggregate_max_size - 1; i < auto_supported_aggregate_max_size; --i)
            {
                if(tags[i])
                {
                    return i + 1uz;
                }
            }
            return 0uz;
        }(std::make_index_sequence<auto_supported_aggregate_max_size>{});

        template<size_t I, class T>
        constexpr decltype(auto) aggregate_get(T&& t) noexcept
        {
            constexpr size_t n = aggregate_member_count<std::remove_cvref_t<T>>;
#include "../code_generate/aggregate_getter_invoker.code"
        };

        template<size_t I, class T>
        using aggregate_get_t = decltype(detail::aggregate_get<I>(std::declval<T>()));

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
        struct aggregate_member
        : storage_type<aggregate_get_t<I, T&>, aggregate_get_t<I, const T&>, aggregate_get_t<I, T&&>, aggregate_get_t<I, const T&&>>
        {};

        template<size_t I, class T>
        using aggregate_member_t = aggregate_member<I, T>::type;
    }

    template<class T>
    inline constexpr size_t child_count = []()
    {
        if constexpr(not std::same_as<T, std::remove_cvref_t<T>>)
        {
            return child_count<std::remove_cvref_t<T>>;
        }
        else if constexpr (std::is_bounded_array_v<T>)
        {
            return std::extent_v<T>;
        }
        else if constexpr(requires{ T::get_child_count(custom_t{}); })
        {
            return T::get_child_count(custom_t{});
        }
        else if constexpr(requires{ std::tuple_size<T>::value; })
        {
            return std::tuple_size_v<T>;
        }
        else if constexpr(std::is_aggregate_v<T> && detail::aggregate_member_count<T> != 0uz)
        {
            return detail::aggregate_member_count<T>;
        }
        else
        {
            return 0uz;
        }
    }();

    namespace detail
    {
        struct child_count_of_fn
        {
            template<class T>
            static consteval size_t operator()(T&&) noexcept
            {
                return child_count<T>;
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::child_count_of_fn child_count_of{};
    }

    namespace detail::subview_ns
    {
        template<size_t I>
        void subview();

        template<size_t I>
        void get();

        enum class strategy_t
        {
            none,
            array,
            member_subview,
            adl_subview,
            member_get,
            adl_get,
            aggregate,
        };

        template<auto...I>
        struct subview_fn;

        template<>
        struct subview_fn<> : adaptor_closure<subview_fn<>>
        {
            template<class T>
            static constexpr decltype(auto) operator()(T&& t) noexcept
            {
                return FWD(t);
            }
        };

        template<std::integral auto I>
        struct subview_fn<I> : adaptor_closure<subview_fn<I>>
        {
            template<class T>
            static consteval choice_t<strategy_t> choose()
            {
                using type = std::remove_cvref_t<T>;
                //array
                if constexpr(std::is_bounded_array_v<type>)
                {
                    return { strategy_t::array, true };
                }
                //custom
                else if constexpr (requires{ std::declval<T>().template subview<I>(custom_t{}); })
                {
                    return { strategy_t::member_subview, noexcept(std::declval<T>().template subview<I>(custom_t{})) };
                }
                else if constexpr (requires{ subview<I>(std::declval<T>(), custom_t{}); })
                {
                    return { strategy_t::adl_subview, noexcept(subview<I>(std::declval<T>(), custom_t{})) };
                }
                //tuple-like
                else if constexpr (requires{ std::tuple_size<type>::value; })
                {
                    if constexpr(requires{ std::declval<T>().template get<I>(); })
                    {
                        return { strategy_t::member_get, noexcept(std::declval<T>().template get<I>()) };
                    }
                    else if constexpr(requires{ get<I>(std::declval<T>()); })
                    {
                        return { strategy_t::adl_get, noexcept(get<I>(std::declval<T>())) };
                    }
                    else
                    {
                        return { strategy_t::none };
                    }
                }
                else if constexpr(std::is_aggregate_v<type> && aggregate_member_count<type> != 0uz)
                {
                    return { strategy_t::aggregate, true };
                }
                else
                {
                    return { strategy_t::none };
                }
            }

            template<class T>
            static constexpr choice_t<strategy_t> choice = choose<T>();

            
            template<class T> requires (choice<T>.strategy != strategy_t::none)
            static constexpr decltype(auto) operator()(T&& t) noexcept(choice<T>.nothrow)
            {
                constexpr strategy_t strategy = choice<T>.strategy;
                if constexpr(strategy == strategy_t::array)
                {
                    return std::forward_like<T>(t[I]);
                }
                else if constexpr (strategy == strategy_t::member_subview)
                {
                    return FWD(t).template subview<I>(custom_t{});
                }
                else if constexpr (strategy == strategy_t::adl_subview)
                {
                    return subview<I>(FWD(t), custom_t{});
                }
                else if constexpr (strategy == strategy_t::member_get)
                {
                    return FWD(t).template get<I>();
                }
                else if constexpr (strategy == strategy_t::adl_get)
                {
                    return get<I>(FWD(t));
                }
                else if constexpr(strategy == strategy_t::aggregate)
                {
                    return detail::aggregate_get<I>(FWD(t));
                }
                else
                {
                    static_assert(false, "No suitable tree_get or get function found.");
                }
            }
        };

        template<array Indexes>
        struct subview_fn<Indexes> : adaptor_closure<subview_fn<Indexes>>
        {
            template<class T>
            static constexpr decltype(auto) operator()(T&& t)
            {
                return [&]<size_t...I>(std::index_sequence<I...>) -> decltype(auto)
                {
                    return (FWD(t) | ... | subview_fn<Indexes[I]>{});
                }(std::make_index_sequence<Indexes.size()>{});
            }
        };

        template<auto...I>
        struct subview_fn : adaptor_closure<subview_fn<I...>>
        {
            template<class T>
            static constexpr decltype(auto) operator()(T&& t)
            {
                return (FWD(t) | ... | subview_fn<I>{});
            }
        };
    }

    inline namespace functors
    {
        template<indexical auto...I>
        inline constexpr detail::subview_ns::subview_fn<I...> subview{};

        template<indexical auto...I>
        inline constexpr auto subtree = subview<I...> | pass;
    }

    template<class T, indexical auto...I>
    using subview_t = decltype(subview<I...>(std::declval<T>()));

    template<class T, indexical auto...I>
    using subtree_t = decltype(subtree<I...>(std::declval<T>()));

    template<class T>
    concept terminal = child_count<T> == 0;

    template<class T>
    concept branched = not terminal<T>;

    template<class T>
    inline constexpr size_t leaf_count = []
    {
        if constexpr (terminal<T>)
        {
            return 1uz;
        }
        else
        {
            return[]<size_t...I>(std::index_sequence<I...>)
            {
                return (0uz + ... + leaf_count<subview_t<T, I>>);
            }(std::make_index_sequence<child_count<T>>{});
        }
    }();

    template<class T>
    inline constexpr auto shape = [] 
    {
        if constexpr (terminal<T>)
        {
            return tuple{};
        }
        else return[]<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(shape<subview_t<T, I>>...);
        }(std::make_index_sequence<child_count<T>>{});
    }();

    template<class T>
    using shape_t = std::remove_const_t<decltype(shape<T>)>;

    template<class T>
    inline constexpr size_t tensor_rank = []
    {
        if constexpr (terminal<T>)
        {
            return 0uz;
        }
        else return[]<size_t...I>(std::index_sequence<I...>)
        {
            return 1uz + detail::min(tensor_rank<subview_t<T, I>>...);
        }(std::make_index_sequence<child_count<T>>{});
    }();

    template<class T>
    inline constexpr auto tensor_shape = []
    {
        if constexpr (terminal<T>)
        {
            return array<size_t, 0uz>{};
        }
        else return[]<size_t...I>(std::index_sequence<I...>)
        {
            constexpr size_t rank = tensor_rank<T>;
            array<size_t, rank> result{ child_count<T> };

            constexpr auto subshapes = tuple{ tensor_shape<subview_t<T, I>>... };
            for (size_t i = 0uz; i < rank - 1uz; ++i)
            {
                result[i + 1uz] = detail::min(get<I>(subshapes)[i]...);
            }

            return result;
        }(std::make_index_sequence<child_count<T>>{});
    }();

    template<class T>
    inline constexpr array<size_t, 2uz> mat_shape = detail::array_take<2>(tensor_shape<T>);

    template<class...T>
    struct default_container;

    template<class...T>
    using default_container_t = default_container<T...>::type;

    template<>
    struct default_container<>
    {
        using type = tuple<>;
    };

    template<class T, class...Rest>
    struct default_container<T, Rest...>
     : std::conditional<(std::is_object_v<T> && ... && std::same_as<T, Rest>), 
            array<T, sizeof...(Rest) + 1>, 
            tuple<T, Rest...>
        >
    {};

    namespace detail
    {
        struct apply_fn : adaptor<apply_fn>
        {
            template<class T, class Fn>
            static constexpr decltype(auto) adapt(T&& t, Fn&& fn)
            {
                return [&]<size_t...I>(std::index_sequence<I...>) -> decltype(auto)
                {
                    return FWD(fn)(tree_get<I>(FWD(t))...);
                }(std::make_index_sequence<child_count<T>>{});
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::apply_fn apply{};
    }
}

namespace senluo::detail 
{
    template<class S, class T>
    constexpr auto replicate(const T& value, S shape = {})
    {
        if constexpr (terminal<S>)
        {
            return value;
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(detail::replicate(value, get<I>(shape))...);
        }(std::make_index_sequence<child_count<S>>{});
    }
}

#include "../tools/macro_undef.hpp"
#endif // SENLUO_SUBTREE_HPP
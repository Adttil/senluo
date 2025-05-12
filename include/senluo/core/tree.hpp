#ifndef SENLUO_TREE_HPP
#define SENLUO_TREE_HPP

#include <tuple>

#include "../tools/general.hpp"
#include "../tools/array.hpp"
#include "../tools/tuple.hpp"
#include "wrap.hpp"

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
}

namespace senluo
{
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
    struct tree_size
    {
        static constexpr size_t value = []()
        {
            if constexpr (std::is_bounded_array_v<T>)
            {
                return std::extent_v<T>;
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
    };

    template<class T>
    inline constexpr size_t tree_size_v = tree_size<T>::value;

    template<class T>
    inline constexpr size_t size = tree_size_v<std::remove_cvref_t<T>>;

    namespace detail::subtree_ns
    {
        template<size_t I>
        void tree_get();

        template<size_t I>
        void get();

        enum class strategy_t
        {
            none,

            array,
            array_copy,
            
            member_tree_get,
            adl_tree_get,
            
            member_get,
            member_get_copy,
            
            adl_get,
            adl_get_copy,

            aggregate,
            aggregate_copy
        };

        template<size_t I>
        struct tree_get_fn : adaptor_closure<tree_get_fn<I>>
        {
        private:
            template<class T>
            static consteval choice_t<strategy_t> choose()
            {
                using utype = ideal_unwrap_t<T>;
                using type = std::remove_cvref_t<utype>;
                //array
                if constexpr(std::is_bounded_array_v<type>)
                {
                    using etype = std::remove_cvref_t<decltype(std::declval<type>()[0])>;
                    if constexpr(std::is_object_v<utype> && std::is_move_constructible_v<etype> )
                    {
                        
                        return { strategy_t::array_copy, noexcept(std::is_nothrow_move_constructible_v<etype>) };
                    }
                    else
                    {
                        return { strategy_t::array, true };
                    }
                }
                //custom
                else if constexpr (requires{ std::declval<T>().template tree_get<I>(); })
                {
                    return { strategy_t::member_tree_get, noexcept(std::declval<T>().template tree_get<I>()) };
                }
                else if constexpr (requires{ tree_get<I>(std::declval<T>()); })
                {
                    return { strategy_t::adl_tree_get, noexcept(tree_get<I>(std::declval<T>())) };
                }
                //tuple-like
                else if constexpr (requires{ std::tuple_size<type>::value; std::declval<utype>().template get<I>(); })
                {
                    using etype = std::tuple_element_t<I, type>;
                    if constexpr(std::is_object_v<utype> && std::is_object_v<etype> && std::is_move_constructible_v<etype>)
                    {
                        return { strategy_t::member_get_copy, noexcept((etype)std::declval<utype>().template get<I>()) };
                    }
                    else
                    {
                        return { strategy_t::member_get, noexcept(std::declval<utype>().template get<I>()) };
                    }
                }
                else if constexpr(requires{ std::tuple_size<type>::value; get<I>(std::declval<utype>()); })
                {
                    using etype = std::tuple_element_t<I, type>;
                    if constexpr(std::is_object_v<utype> && std::is_object_v<etype> && std::is_move_constructible_v<etype>)
                    {
                        return { strategy_t::adl_get_copy, noexcept((etype)get<I>(std::declval<utype>())) };
                    }
                    else
                    {
                        return { strategy_t::adl_get, noexcept(get<I>(std::declval<utype>())) };
                    }
                }
                else if constexpr(std::is_aggregate_v<type> && aggregate_member_count<type> != 0uz)
                {
                    using mtype = aggregate_member_t<I, type>;
                    if constexpr(std::is_object_v<utype> && std::is_object_v<mtype> && std::is_move_constructible_v<mtype>)
                    {
                        return { strategy_t::aggregate_copy, std::is_nothrow_move_constructible_v<mtype> };
                    }
                    else
                    {
                        return { strategy_t::aggregate, true };
                    }
                }
                else
                {
                    return { strategy_t::none };
                }
            }

            template<class T>
            static constexpr choice_t<strategy_t> choice = choose<T>();

            
        public:
            template<class T> requires (choice<T>.strategy != strategy_t::none)
            constexpr decltype(auto) operator()(T&& t) const noexcept(choice<T>.nothrow)
            {
                constexpr strategy_t strategy = choice<T>.strategy;
                if constexpr(strategy == strategy_t::array)
                {
                    return std::forward_like<unwrap_t<T>>(unwrap_fwd(t)[I]);
                }
                else if constexpr (strategy == strategy_t::array_copy)
                {
                    return decay_copy(std::forward_like<unwrap_t<T>>(unwrap_fwd(t)[I]));
                }
                else if constexpr (strategy == strategy_t::member_tree_get)
                {
                    return FWD(t).template tree_get<I>();
                }
                else if constexpr (strategy == strategy_t::adl_tree_get)
                {
                    return tree_get<I>(FWD(t));
                }
                else if constexpr (strategy == strategy_t::member_get)
                {
                    return unwrap_fwd(FWD(t)).template get<I>();
                }
                else if constexpr (strategy == strategy_t::member_get_copy)
                {
                    return decay_copy(unwrap_fwd(FWD(t)).template get<I>());
                }
                else if constexpr (strategy == strategy_t::adl_get)
                {
                    return get<I>(unwrap_fwd(FWD(t)));
                }
                else if constexpr (strategy == strategy_t::adl_get_copy)
                {
                    return decay_copy(get<I>(unwrap_fwd(FWD(t))));
                }
                else if constexpr(strategy == strategy_t::aggregate)
                {
                    return detail::aggregate_get<I>(unwrap_fwd(FWD(t)));
                }
                else if constexpr(strategy == strategy_t::aggregate_copy)
                {
                    return decay_copy(detail::aggregate_get<I>(unwrap_fwd(FWD(t))));
                }
                else
                {
                    static_assert(strategy != strategy_t::none, "No suitable tree_get or get function found.");
                }
            }
        };
    }

    inline namespace functors
    {
        template<size_t I>
        inline constexpr detail::subtree_ns::tree_get_fn<I> tree_get{};
    }

    template<size_t I, class T>
    using tree_get_t = decltype(tree_get<I>(std::declval<T>()));

    template<class T>
    concept terminal = size<T> == 0;

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
                return (0uz + ... + leaf_count<tree_get_t<I, T&>>);
            }(std::make_index_sequence<size<T>>{});
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
            return make_tuple(shape<tree_get_t<I, T&>>...);
        }(std::make_index_sequence<size<T>>{});
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
            return 1uz + detail::min(tensor_rank<tree_get_t<I, T>>...);
        }(std::make_index_sequence<size<T>>{});
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
            array<size_t, rank> result{ size<T> };

            constexpr auto subshapes = tuple{ tensor_shape<tree_get_t<I, T>>... };
            for (size_t i = 0uz; i < rank - 1uz; ++i)
            {
                result[i + 1uz] = detail::min(get<I>(subshapes)[i]...);
            }

            return result;
        }(std::make_index_sequence<size<T>>{});
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
    {
        using type = std::conditional_t<(std::is_object_v<T> && ... && std::same_as<T, Rest>), 
            array<T, sizeof...(Rest) + 1>, 
            tuple<T, Rest...>
        >;
    };
}

namespace senluo 
{
    // template<class T, size_t N>
    // struct tree_size<T[N]> : std::integral_constant<size_t, N> {};

    template<class T>
    struct tree_size<wrapper<T>> : tree_size<std::remove_cvref_t<T>> {};
}

#include "../tools/macro_undef.hpp"
#endif // SENLUO_SUBTREE_HPP
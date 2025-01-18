#ifndef SENLUO_TREE_HPP
#define SENLUO_TREE_HPP

#include "adaptor.hpp"
#include "array.hpp"
#include "math.hpp"
#include "tuple.hpp"

#include "macro_define.hpp"

namespace senluo::detail
{
    constexpr size_t normalize_index(std::integral auto index, size_t size)noexcept
    {
        if(index >= 0)
        {
            return static_cast<size_t>(index % size);
        }
        else
        {
            return static_cast<size_t>(size - -index % size);
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

    template<class S, class T>
    constexpr auto make_tree_of_same_value(const T& value, S = {})
    {
        if constexpr(std::same_as<S, tuple<>>)
        {
            return value;
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(detail::make_tree_of_same_value(value, get<I>(S{}))...);
        }(std::make_index_sequence<std::tuple_size_v<S>>{});
    }
    
    template<class T>
    concept indexical_array = requires(std::remove_cvref_t<T> t, size_t i)
    {
        requires std::integral<typename std::remove_cvref_t<T>::value_type>;
        std::tuple_size_v<std::remove_cvref_t<T>>;
        { t[i] } -> std::same_as<typename std::remove_cvref_t<T>::value_type&>;
    };
}

namespace senluo
{
    template<class T>
    concept indexical = std::integral<T> || detail::indexical_array<T>;

    inline constexpr array<size_t, 0uz> indexes_of_whole{};

    struct end_t
    {
        end_t() = delete;
    };

    end_t end();
    
    inline constexpr size_t auto_supported_aggregate_max_size = 64uz;

    namespace detail::get_ns
    {
        struct universal_type
        {
            //Can not use "requires" in clang here.
            //https://github.com/llvm/llvm-project/issues/76415
            template <class T, class = std::enable_if_t<std::is_copy_constructible_v<T>>>
            operator T&();

            template <class T, class = std::enable_if_t<std::is_move_constructible_v<T>>>
            operator T&&();

            template <class T, class = std::enable_if_t<!std::is_copy_constructible_v<T> && !std::is_move_constructible_v<T>>>
            operator T();
        };

        template <class T>
        inline constexpr size_t aggregate_member_count = []<bool had_success = false>(this auto && self, auto...args)
        {
            using type = std::remove_cvref_t<T>;
            if constexpr (sizeof...(args) > auto_supported_aggregate_max_size)
            {
                return 0uz;
            }
            else if constexpr(sizeof...(args) == 0 && requires{ type{ {universal_type{}} }; })
            {
                return self.template operator()<true>(universal_type{});
            }
            else if constexpr (sizeof...(args) != 0 && requires{ type{ universal_type{}, universal_type{args}... }; })
            {
                return self.template operator()<true>(universal_type{ args }..., universal_type{});
            }
            else if constexpr (had_success)
            {
                return sizeof...(args);
            }
            else
            {
                return self.template operator()<false>(universal_type{ args }..., universal_type{});
            }
        }();

        template<size_t I, class T>
        constexpr decltype(auto) aggregate_get(T&& t) noexcept
        {
            constexpr size_t n = aggregate_member_count<T>;
            if constexpr(I >= n)
            {
                return end();
            }
            else
            {
#include "code_generate/aggregate_getter_invoker.code"
            }
        };

        template<size_t I>
        void get();

        enum strategy_t
        {
            none,
            array,
            tagged_member,
            tagged_adl,
            member,
            adl,
            aggregate
        };

        template<size_t I>
        struct get_t
        {
            //consteval coused error in msvc.
            template<class T>
            static constexpr choice_t<strategy_t> choose()
            {
                using type = std::remove_cvref_t<T>;
                if constexpr(std::is_bounded_array_v<type>)
                {
                    if constexpr(I < std::extent_v<type>)
                    {
                        return { strategy_t::array, true };
                    }
                    else
                    {
                        return { strategy_t::none, true };
                    }
                }
                else if constexpr(requires{ std::declval<T>().template get<I>(custom_t{}); })
                {
                    return { strategy_t::tagged_member, noexcept(std::declval<T>().template get<I>(custom_t{})) };
                }
                else if constexpr(requires{ get<I>(std::declval<T>(), custom_t{}); })
                {
                    return { strategy_t::tagged_adl, noexcept(get<I>(std::declval<T>(), custom_t{})) };
                }
                else if constexpr(requires{ requires (I >= std::tuple_size<type>::value); })
                {
                    return { strategy_t::none, true };
                }
                else if constexpr(requires{ std::declval<T>().template get<I>(); })
                {
                    return { strategy_t::member, noexcept(std::declval<T>().template get<I>()) };
                }
                else if constexpr(requires{ get<I>(std::declval<T>()); })
                {
                    return { strategy_t::adl, noexcept(get<I>(std::declval<T>())) };
                }
                else if constexpr(std::is_aggregate_v<type> && aggregate_member_count<T> != 0uz)
                {
                    return { strategy_t::aggregate, true };
                }
                else
                {
                    return { strategy_t::none, true };
                }
            }

            template<class T>
            constexpr decltype(auto) operator()(T&& t)const
            noexcept(choose<T>().nothrow)
            {
                constexpr strategy_t strategy = choose<T>().strategy;

                if constexpr(strategy == strategy_t::none)
                {
                    return end();
                }
                else if constexpr(strategy == strategy_t::array)
                {
                    return FWD(t)[I];
                }
                else if constexpr(strategy == strategy_t::tagged_member)
                {
                    return FWD(t).template get<I>(custom_t{});
                }
                else if constexpr(strategy == strategy_t::tagged_adl)
                {
                    return get<I>(FWD(t), custom_t{});
                }
                else if constexpr(strategy == strategy_t::member)
                {
                    return FWD(t).template get<I>();
                }
                else if constexpr(strategy == strategy_t::adl)
                {
                    return get<I>(FWD(t));
                }
                else if constexpr(strategy == strategy_t::aggregate)
                {
                    return aggregate_get<I>(FWD(t));
                }
            }
        };
    }

    namespace detail 
    {
        template<size_t I>
        inline constexpr get_ns::get_t<I> raw_get{};
    }

    template<class T>
    inline constexpr size_t size = []<size_t N = 0uz>(this auto&& self)
    {
        if constexpr (std::same_as<decltype(detail::raw_get<N>(std::declval<T>())), end_t>)
        {
            return N;
        }
        else
        {
            return self.template operator()<N + 1uz>();
        }
    }();
    
    namespace detail
    {
        template<auto Indexes>
        struct subtree_fn;
    }

    // msvc bug: https://developercommunity.visualstudio.com/t/MSVC-cannot-correctly-recognize-NTTP-in/10722592
    // template<indexical auto...I>
    // inline constexpr detail::subtree_t<senluo::to_indexes(I...)> subtree{};

    template<indexical auto...I>
    inline constexpr auto subtree = detail::subtree_fn<detail::to_indexes(I...)>{};
    
    template<auto Indexes>
    struct detail::subtree_fn : adaptor_closure<subtree_fn<Indexes>>
    {
        template<class T> requires (Indexes.size() == 0uz)
        constexpr T&& operator()(T&& t) const noexcept
        {
            return FWD(t);
        }

        template<class T> requires (Indexes.size() == 1uz && size<T> > 0)
        constexpr auto operator()(T&& t) const
        AS_EXPRESSION(
            raw_get<detail::normalize_index(Indexes[0], size<T>)>(FWD(t))
        )

        template<class T> requires (Indexes.size() > 1uz && size<T> > 0)
        constexpr auto operator()(T&& t) const
        AS_EXPRESSION(
            raw_get<detail::normalize_index(Indexes[0], size<T>)>(FWD(t)) | subtree<detail::array_drop<1uz>(Indexes)>
        )
    };

    template<class T, indexical auto...I>
    using subtree_t = decltype(subtree<I...>(std::declval<T>()));

    template<class T>
    concept terminal = (size<T> == 0uz);

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
                return (0uz + ... + leaf_count<subtree_t<T, I>>);
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
            return senluo::make_tuple(shape<subtree_t<T, I>>...);
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
            return 1uz + detail::min(tensor_rank<subtree_t<T, I>>...);
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

            constexpr auto subshapes = tuple{ tensor_shape<subtree_t<T, I>>... };
            for (size_t i = 0uz; i < rank - 1uz; ++i)
            {
                result[i + 1uz] = detail::min(get<I>(subshapes)[i]...);
            }

            return result;
        }(std::make_index_sequence<size<T>>{});
    }();
}

#include "macro_undef.hpp"
#endif
#ifndef SENLUO_RELAYOUT_HPP
#define SENLUO_RELAYOUT_HPP

#include "../tools/adaptor.hpp"

#include "subtree.hpp"
#include "make.hpp"

#include "../tools/macro_define.hpp"

namespace senluo::detail
{
    template<size_t I>
    void get();

    template<size_t I, class L>
    constexpr auto layout_get(const L& layout)
    {
        if constexpr(indexical<L>)
        {
            return detail::array_cat(layout, array{ I });
        }
        else if constexpr(I < std::tuple_size_v<L>)
        {
            return get<I>(layout);
        }
        // else
        // {
        //     return invalid_index;
        // }
    }

    template<indexical auto Indices, class Shape>
    constexpr auto normalize_indices(Shape shape = {})
    {
        if constexpr(std::integral<decltype(Indices)>)
        {
            return array{ detail::normalize_index(Indices, size<Shape>) };
        }
        else return []<size_t...I>(std::index_sequence<I...>)
        {
            return array<size_t, Indices.size()>
            {
                detail::normalize_index(Indices[I], size<subtree_t<Shape, detail::array_take<I>(Indices)>>)...
            };
        }(std::make_index_sequence<Indices.size()>{});
    }

    template<auto Layout, class Shape>
    constexpr auto fold_layout(Shape shape = {})
    {
        if constexpr(indexical<decltype(Layout)>)
        {
            return detail::normalize_indices<Layout>(shape);
        }
        else return []<size_t...I>(std::index_sequence<I...>)
        {
            constexpr auto child_relayout = make_tuple(detail::fold_layout<subtree<I>(Layout)>(Shape{})...);
            constexpr size_t n = size<decltype(get<0uz>(child_relayout))>;

            if constexpr(n > 0uz
                && (... && detail::indexical_array<decltype(get<I>(child_relayout))>)
                && (... && (n == size<decltype(get<I>(child_relayout))>))
            )
            {
                constexpr auto prefix = detail::array_take<n - 1uz>(get<0uz>(child_relayout));
                if constexpr(size<subtree_t<Shape, prefix>> == size<decltype(Layout)>
                    && (... && (prefix == detail::array_take<n - 1uz>(get<I>(child_relayout))))
                    && (... && (get<I>(child_relayout)[n - 1uz] == I))
                )
                {
                    return prefix;
                }
                else
                {
                    return child_relayout;
                }
            }
            else
            {
                return child_relayout;
            }
        }(std::make_index_sequence<size<decltype(Layout)>>{});
    }

    template<auto Layout, class BaseShape>
    constexpr auto unfold_layout(BaseShape base_shape = {})
    {
        if constexpr(indexical<decltype(Layout)>)
        {
            constexpr auto indexes = detail::normalize_indices<Layout, BaseShape>();
            using subshape_t = subtree_t<BaseShape, indexes>;
            if constexpr(terminal<subshape_t>)
            {
                return indexes;
            }
            else return [&]<size_t...I>(std::index_sequence<I...>)
            {
                constexpr auto indexes = detail::normalize_indices<Layout, BaseShape>();
                return make_tuple(detail::unfold_layout<detail::array_cat(indexes, array{ I })>(base_shape)...);
            }(std::make_index_sequence<size<subshape_t>>{});
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(detail::unfold_layout<get<I>(Layout)>(base_shape)...);
        }(std::make_index_sequence<size<decltype(Layout)>>{});
    }

    template<typename TLayout, size_t N>
    constexpr auto layout_add_prefix(const TLayout& layout, const array<size_t, N>& prefix)
    {
        if constexpr(detail::indexical_array<TLayout>)
        {
            return detail::array_cat(prefix, layout);
        }
        else return[&]<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(detail::layout_add_prefix(get<I>(layout), prefix)...);
        }(std::make_index_sequence<size<TLayout>>{});
    }

    template<typename T>
    constexpr auto default_unfolded_layout()
    {
        if constexpr (terminal<T>)
        {
            return indexes_of_whole;
        }
        else return[]<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(detail::layout_add_prefix(detail::default_unfolded_layout<subtree_t<T&, I>>(), array{I})...);
        }(std::make_index_sequence<size<T>>{});    
    };

    enum class layout_mapping_type_t
    {
        multi_shot,
        injective,
        bijective
    };

    template<auto Layout, size_t Depth = 0uz, typename T>
    constexpr bool inverse_layout_index_len_at(T& result_index_len, size_t& setted_count)
    {
        if constexpr(detail::indexical_array<decltype(Layout)>)
        {
            if(subtree<Layout>(result_index_len) == invalid_index)
            {
                subtree<Layout>(result_index_len) = Depth;
                ++setted_count;
                return true;
            }
            else
            {
                return false;
            }
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {   
            return (... && detail::inverse_layout_index_len_at<get<I>(Layout), Depth + 1uz>(result_index_len, setted_count));
        }(std::make_index_sequence<size<decltype(Layout)>>{});
    }

    template<class S, class T>
    constexpr auto make_tree_of_same_value_and_set_leaf_count(const T& value, size_t& count, S = {})
    {
        if constexpr(terminal<S>)
        {
            ++count;
            return value;
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(detail::make_tree_of_same_value_and_set_leaf_count(value, count, get<I>(S{}))...);
        }(std::make_index_sequence<size<S>>{});
    }

    template<auto Layout, typename S>
    constexpr auto inverse_layout_index_len(S = {})
    {
        size_t leaf_count = 0uz;
        auto index_len_tree = detail::make_tree_of_same_value_and_set_leaf_count(invalid_index, leaf_count, S{});
        size_t setted_count = 0uz;
        bool is_injective = detail::inverse_layout_index_len_at<Layout>(index_len_tree, setted_count);

        layout_mapping_type_t mapping_type;
        if(not is_injective)
        {
            mapping_type = layout_mapping_type_t::multi_shot;
        }
        else if(leaf_count != setted_count)
        {
            mapping_type = layout_mapping_type_t::injective;
        }
        else
        {
            mapping_type = layout_mapping_type_t::bijective;
        }

        struct result_t
        {
            layout_mapping_type_t mapping_type;
            decltype(index_len_tree) index_len_tree;
        };
        return result_t{ mapping_type, index_len_tree  };
    }

    template<auto IndexLenTree>
    constexpr auto init_layout()
    {
        if constexpr(std::same_as<decltype(IndexLenTree), size_t>)
        {
            return array<size_t, IndexLenTree>{};
        }
        else return []<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(detail::init_layout<get<I>(IndexLenTree)>()...);
        }(std::make_index_sequence<size<decltype(IndexLenTree)>>{});
    }

    template<auto Layout, auto CurIndex = indexes_of_whole, typename R>
    constexpr void inverse_layout_at(R& result)
    {
        if constexpr(detail::indexical_array<decltype(Layout)>)
        {
            subtree<Layout>(result) = CurIndex;
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {   
            return (..., detail::inverse_layout_at<get<I>(Layout), detail::array_cat(CurIndex, array{ I })>(result));
        }(std::make_index_sequence<size<decltype(Layout)>>{});
    }

    template<auto UnFoldedLayout, typename S>
    constexpr auto get_layout_cache(S = {})
    {
        constexpr auto inverse_layout_index_len_result = detail::inverse_layout_index_len<UnFoldedLayout, S>();
        
        if constexpr(inverse_layout_index_len_result.mapping_type == layout_mapping_type_t::bijective)
        {
            auto inverse_layout = detail::init_layout<inverse_layout_index_len_result.index_len_tree>();
            detail::inverse_layout_at<UnFoldedLayout>(inverse_layout);
            struct layout_chache_t
            {
                layout_mapping_type_t mapping_type;
                decltype(inverse_layout) inverse_layout;
            };
            return layout_chache_t{ inverse_layout_index_len_result.mapping_type, inverse_layout };
        }
        else
        {
            struct layout_chache_t
            {
                layout_mapping_type_t mapping_type;
            };
            return layout_chache_t{ inverse_layout_index_len_result.mapping_type };
        }
    }
}

namespace senluo
{
    namespace detail::relayout_ns 
    {
        template<class T, auto FoldedLayout>
        struct relayout_tree;
    }

    namespace detail
    {
        template<auto Layout>
        struct relayout_fn;
    }

    inline namespace functors
    {
        template<auto Layout>
        inline constexpr detail::relayout_fn<Layout> relayout{};
    }

    template<class T, auto FoldedLayout>
    struct detail::relayout_ns::relayout_tree
    {
        T base;

        template<size_t I, class Self>
        constexpr decltype(auto) tree_get(this Self&& self, custom_t = {})
        {
            constexpr auto layout = detail::layout_get<I>(FoldedLayout);
            if constexpr(indexical<decltype(layout)>)
            {
                return FWD(self, base) | subtree<detail::layout_get<I>(FoldedLayout)>;
            }
            else
            {
                return relayout_tree<decltype(pass(FWD(self, base))), detail::layout_get<I>(FoldedLayout)>{
                    FWD(self, base)
                };
            }
        }
        
        static consteval size_t get_size(custom_t = {})
        {
            if constexpr(detail::indexical_array<decltype(FoldedLayout)>)
            {
                return size<subtree_t<T, FoldedLayout>>;
            }
            else
            {
                return std::tuple_size_v<std::remove_const_t<decltype(FoldedLayout)>>;
            }
        }

        static constexpr auto unfold_layout = detail::unfold_layout<FoldedLayout>(shape<T>);
        static constexpr auto layout_cache = detail::get_layout_cache<unfold_layout>(shape<T>);
        
        template<class U>
        static constexpr relayout_tree make_from(U&& u, custom_t = {})
        AS_EXPRESSION(
            relayout_tree{ FWD(u) | relayout<layout_cache.inverse_layout> | make<T> }
        )

        friend constexpr bool operator==(const relayout_tree&, const relayout_tree&) = default;
    };

    namespace detail 
    {
        template<auto FoldedLayout, class T>
        constexpr decltype(auto) relayout_unchecked(T&& t)
        {
            if constexpr(detail::indexical_array<decltype(FoldedLayout)>)
            {
                return FWD(t) | subtree<FoldedLayout>;
            }
            else
            {
                return detail::relayout_ns::relayout_tree<decltype(pass(FWD(t))), FoldedLayout>{
                    FWD(t)
                };
            }
        }

        template<auto Layout>
        struct relayout_fn : adaptor_closure<relayout_fn<Layout>>
        {
            template<class T>
            constexpr decltype(auto) operator()(T&& t) const noexcept
            {
                constexpr auto unfold_layout = detail::unfold_layout<Layout>(shape<T>);
                constexpr auto layout_cache = detail::get_layout_cache<unfold_layout>(shape<T>);
                static_assert(
                    layout_cache.mapping_type != layout_mapping_type_t::multi_shot, 
                    "multi-shot layout is not supported yet.");

                constexpr auto folded_layout = detail::fold_layout<Layout>(shape<T>);
                return detail::relayout_unchecked<folded_layout>(FWD(t));
            }
        };
    }
}

namespace senluo
{
    template<class Relayouter>
    struct relayouter_interface;

    template<class Relayouter>
    struct relayouter_interface : adaptor_closure<Relayouter>
    {
        template<class T>
        static consteval auto layout() noexcept
        {
            constexpr auto tree = detail::default_unfolded_layout<T>();
            return detail::fold_layout<Relayouter::relayout(tree)>(shape<T>);
        }

        template<typename T>
        constexpr decltype(auto) operator()(T&& tree) const
        {
            constexpr auto layout = Relayouter::template layout<T>();
            return detail::relayout_unchecked<layout>(FWD(tree));
        }
    };

    namespace detail
    {
        template<size_t I, size_t Axis>
        struct component_t : relayouter_interface<component_t<I, Axis>>
        {
            template<typename T>
            static constexpr auto relayout(const T& tree)
            {
                if constexpr (Axis == 0uz)
                {
                    static_assert(I < size<T>, "Component index out of range.");
                    return subtree<I>(tree);
                }
                else
                {
                    static_assert(branched<T>, "Axis index out of range.");
                    return[&]<size_t...J>(std::index_sequence<J...>)
                    {
                        return make_tuple(component_t<I, Axis - 1uz>::relayout(subtree<J>(tree))...);
                    }(std::make_index_sequence<size<T>>{});
                }
            }
        };
    }

    inline namespace functors
    {
        template<size_t I, size_t Axis>
        inline constexpr detail::component_t<I, Axis> component{}; 
    }

    namespace detail
    {
        template<size_t Axis1, size_t Axis2>
        struct transpose_t : relayouter_interface<transpose_t<Axis1, Axis2>>
        {
            template<typename T>
            static constexpr auto relayout(const T& tree)
            {
                if constexpr (Axis1 == 0uz)
                {
                    constexpr size_t N = tensor_shape<T>[Axis2];
                    return[&]<size_t...I>(std::index_sequence<I...>)
                    {
                        return senluo::make_tuple(component_t<I, Axis2>::relayout(tree)...);
                    }(std::make_index_sequence<N>{});
                }
                else return[&]<size_t...I>(std::index_sequence<I...>)
                {
                    return senluo::make_tuple(transpose_t<Axis1 - 1uz, Axis2 - 1uz>::relayout(subtree<I>(tree))...);
                }(std::make_index_sequence<size<T>>{});
            }
        };
    }

    inline namespace functors
    {
        template<size_t Axis1 = 0uz, size_t Axis2 = Axis1 + 1uz>
        inline constexpr detail::transpose_t<Axis1, Axis2> transpose{}; 
    }

    namespace detail
    {
        struct inverse_t : relayouter_interface<inverse_t>
        {
            template<typename T>
            static constexpr auto relayout(const T& tree)
            {
                return[&]<size_t...I>(std::index_sequence<I...>)
                {
                    constexpr auto last_index = size<T> - 1uz;
                    return make_tuple(subtree<last_index - I>(tree)...);
                }(std::make_index_sequence<size<T>>{});
            }

            template<class T>
            static consteval auto layout() noexcept
            {
                return [&]<size_t...I>(std::index_sequence<I...>)
                {
                    constexpr auto last_index = size<T> - 1uz;
                    return tuple{ array{ last_index - I }... };
                }(std::make_index_sequence<size<T>>{});
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::inverse_t inverse{}; 
    }

    namespace detail
    {
        template<size_t N>
        struct take_fn : relayouter_interface<take_fn<N>>
        {
            template<typename T>
            static consteval auto layout()
            {
                if constexpr(N == size<T>)
                {
                    return indexes_of_whole;
                }
                else return[&]<size_t...I>(std::index_sequence<I...>)
                {
                    return tuple{ array<size_t, 1uz>{ I }... };
                }(std::make_index_sequence<N>{});
            }

            template<typename T>
            static constexpr auto relayout(const T& tree)
            {
                return[&]<size_t...I>(std::index_sequence<I...>)
                {
                    return make_tuple(subtree<I>(tree)...);
                }(std::make_index_sequence<N>{});
            }
        };

        template<size_t N>
        struct drop_fn : relayouter_interface<drop_fn<N>>
        {
            template<typename T>
            static consteval auto layout()
            {
                if constexpr(N == 0uz)
                {
                    return indexes_of_whole;
                }
                else return[&]<size_t...I>(std::index_sequence<I...>)
                {
                    return tuple{ array<size_t, 1uz>{ I + N }... };
                }(std::make_index_sequence<size<T> - N>{});
            }

            template<typename T>
            static constexpr auto relayout(const T& tree)
            {
                return[&]<size_t...I>(std::index_sequence<I...>)
                {
                    return make_tuple(subtree<I + N>(tree)...);
                }(std::make_index_sequence<size<T> - N>{});
            }
        };

        template<size_t N, size_t M>
        struct drop_take_fn : relayouter_interface<drop_take_fn<N, M>>
        {
            template<typename T>
            static consteval auto layout()
            {
                if constexpr(N == 0uz && M == size<T>)
                {
                    return indexes_of_whole;
                }
                else return[&]<size_t...I>(std::index_sequence<I...>)
                {
                    return tuple{ array<size_t, 1uz>{ I + N }... };
                }(std::make_index_sequence<M>{});
            }

            template<typename T>
            static constexpr auto relayout(const T& tree)
            {
                return[&]<size_t...I>(std::index_sequence<I...>)
                {
                    return make_tuple(subtree<I + N>(tree)...);
                }(std::make_index_sequence<M>{});
            }
        };
    }

    inline namespace functors
    {
        template<size_t N>
        inline constexpr detail::take_fn<N> take{}; 

        template<size_t N>
        inline constexpr detail::drop_fn<N> drop{}; 

        template<size_t N, size_t M>
        inline constexpr detail::drop_take_fn<N, M> drop_take{}; 
    }

    namespace detail
    {
        struct combine_fn
        {
            template<typename...T>
            constexpr auto operator()(T&&...t) const
            {
                return tuple<pass_t<T>...>{ FWD(t)... };
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::combine_fn combine{};  
    }

    template<class...T>
    using combine_t = decltype(combine(std::declval<T>()...));

    namespace detail
    {
        struct zip_fn
        {
            template<typename...T>
            constexpr auto operator()(T&&...t) const
            {
                return combine(FWD(t)...) | transpose<>;
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::zip_fn zip{};  
    }

    template<class...T>
    using zip_t = decltype(zip(std::declval<T>()...));

    namespace detail
    {
        struct mat_zip_fn
        {
            template<typename...T>
            constexpr auto operator()(T&&...t) const
            {
                return tuple<T...>{ FWD(t)... } | transpose<> | transpose<1, 2>;
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::mat_zip_fn mat_zip{};  
    }

    template<class...T>
    using mat_zip_t = decltype(mat_zip(std::declval<T>()...));
}

#include "../tools/macro_undef.hpp"
#endif // SENLUO_RELAYOUT_HPP
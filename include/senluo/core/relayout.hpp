#ifndef SENLUO_RELAYOUT_HPP
#define SENLUO_RELAYOUT_HPP

#include "../tools/adaptor.hpp"

#include "subtree.hpp"

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
}

namespace senluo
{
    namespace detail::relayout_ns 
    {
        template<class T, auto FoldedLayout>
        struct relayout_tree;
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
                constexpr auto folded_layout = detail::fold_layout<Layout>(shape<T>);
                return detail::relayout_unchecked<folded_layout>(FWD(t));
            }
        };
    }

    inline namespace functors
    {
        template<auto Layout>
        inline constexpr detail::relayout_fn<Layout> relayout{};
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
#ifndef RUZHOUXIE_RELAYOUT_HPP
#define RUZHOUXIE_RELAYOUT_HPP

#include "../general.hpp"
#include "subtree.hpp"
#include "principle.hpp"
#include "wrap.hpp"
#include "make.hpp"

#include "../macro_define.hpp"

namespace senluo 
{
    template<auto indexes, class L>
    constexpr auto sublayout(const L& layout)
    {
        if constexpr(detail::equal(indexes, indexes_of_whole))
        {
            return layout;
        }
        else
        {
            return sublayout<detail::array_drop<1uz>(indexes)>(detail::layout_get<indexes[0]>(layout));
        }
    }

    template<indexical auto Indices, class Shape>
    constexpr auto normalize_indices(Shape shape)
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
            return normalize_indices<Layout>(shape);
        }
        else return []<size_t...I>(std::index_sequence<I...>)
        {
            constexpr auto child_relayout = make_tuple(fold_layout<subtree<I>(Layout)>(Shape{})...);
            constexpr size_t n = size<decltype(get<0uz>(child_relayout))>;

            // Use subtree instead of get for msvc adl bug: https://gcc.godbolt.org/z/GYvdrbooW.
            if constexpr(n > 0uz
                && (... && detail::indexical_array<decltype(subtree<I>(child_relayout))>)
                && (... && (n == size<decltype(subtree<I>(child_relayout))>))
            )
            {
                constexpr auto prefix = detail::array_take<n - 1uz>(get<0uz>(child_relayout));
                if constexpr((... && (prefix == detail::array_take<n - 1uz>(get<I>(child_relayout))))
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

    template<auto Layout, class Shape>
    constexpr auto unfold_layout(Shape shape = {})
    {
        if constexpr(indexical<decltype(Layout)>)
        {
            constexpr auto indexes = normalize_indices<Layout>(Shape{});
            using subshape_t = subtree_t<Shape, indexes>;
            if constexpr(terminal<subshape_t>)
            {
                return indexes;
            }
            else return [&]<size_t...I>(std::index_sequence<I...>)
            {
                constexpr auto indexes = normalize_indices<Layout>(Shape{});
                return senluo::make_tuple(unfold_layout<detail::array_cat(indexes, array{ I })>(shape)...);
            }(std::make_index_sequence<size<subshape_t>>{});
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return senluo::make_tuple(unfold_layout<get<I>(Layout)>(shape)...);
        }(std::make_index_sequence<size<decltype(Layout)>>{});
    }

    //unfold_layout<layout, shape>() == unfold_layout_by_relayouted_shape(layout, apply_layout<layout>(shape{}))
    template<class Shape, class Layout>
    constexpr auto unfold_layout_by_relayouted_shape(const Layout& layout, Shape shape = {})
    {
        if constexpr(terminal<Shape>)
        {
            static_assert(detail::indexical_array<Layout>, "Invalid layout.");
            return layout;
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            if constexpr(not detail::indexical_array<Layout>)
            {
                static_assert(size<Shape> == size<Layout>, "Invalid layout.");
                return make_tuple(unfold_layout_by_relayouted_shape(get<I>(layout), get<I>(shape))...);
            }
            else
            {
                return make_tuple(
                    unfold_layout_by_relayouted_shape(detail::array_cat(layout, array{ I }) , get<I>(shape))...
                );
            }
        }(std::make_index_sequence<size<Shape>>{});
    }

    template<auto Layout>
    constexpr auto apply_layout(const auto& view)
    {
        using layout_type = decltype(Layout);
        if constexpr(indexical<layout_type>)
        {
            return subtree<Layout>(view);
        }
        else return[&]<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(apply_layout<get<I>(Layout)>(view)...);
        }(std::make_index_sequence<size<layout_type>>{});
    }

    template<auto Layout, class T>
    constexpr auto relayout_tag_tree(const T tag_tree)
    {
        if constexpr(indexical<decltype(Layout)>)
        {
            return detail::tag_subtree<Layout>(tag_tree);
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return senluo::make_tuple(senluo::relayout_tag_tree<get<I>(Layout)>(tag_tree)...);
        }(std::make_index_sequence<size<decltype(Layout)>>{});
    }

    template<auto UnfoldedLayout, typename U, typename R>
    constexpr void inverse_relayout_usage_tree_at(const U& usage_tree, R& result)
    {
        if constexpr(indexical<decltype(UnfoldedLayout)>)
        {
            auto&& subresult = subtree<UnfoldedLayout>(result);
            if constexpr(terminal<decltype(subresult)>)
            {
                subresult = subresult & usage_tree;
            }
            else return [&]<size_t...I>(std::index_sequence<I...>)
            {
                (..., inverse_relayout_usage_tree_at<indexes_of_whole>(usage_tree, get<I>(subresult)));
            }(std::make_index_sequence<size<decltype(subresult)>>{});
        }
        else return[&]<size_t...I>(std::index_sequence<I...>)
        {
            (..., inverse_relayout_usage_tree_at<subtree<I>(UnfoldedLayout)>(detail::tag_tree_get<I>(usage_tree), result));
            //                                   ^^^^^^^ instead of get for msvc adl bug: https://gcc.godbolt.org/z/GYvdrbooW
        }(std::make_index_sequence<size<decltype(UnfoldedLayout)>>{});
    }

    template<auto UnfoldedLayout, typename U, typename S>
    constexpr auto inverse_relayout_usage_tree(const U& usage_tree, const S& shape)
    {
        auto result = detail::make_tree_of_same_value(usage_t::none, shape);
        inverse_relayout_usage_tree_at<UnfoldedLayout>(usage_tree, result);
        return result;
    }
    
    //template<auto Layout, class L>
    // constexpr auto relayout_layout(const L layout)
    // {
    //     if constexpr(indexical<decltype(Layout)>)
    //     {
    //         return sublayout<Layout>(layout);
    //     }
    //     else return [&]<size_t...I>(std::index_sequence<I...>)
    //     {
    //         return senluo::make_tuple(senluo::relayout_layout<get<I>(Layout)>(layout)...);
    //     }(std::make_index_sequence<size<decltype(Layout)>>{});
    // }

    //todo... should change to discard repeat.
    template<auto FoldedLayout, class O>
    constexpr bool is_enable_to_relayout_operation_tree(const O& operation_tree)
    {
        if constexpr(indexical<decltype(FoldedLayout)>)
        {
            if constexpr(FoldedLayout.size() == 0uz)
            {
                return true;
            }
            else
            {
                //msvc bug
                constexpr auto i = detail::array_take<FoldedLayout.size() - 1uz>(FoldedLayout);
                return not std::same_as<decltype(detail::tag_subtree<i>(operation_tree)), operation_t>
                || detail::equal(detail::tag_subtree<FoldedLayout>(operation_tree), operation_t::none);
            }
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return (... && senluo::is_enable_to_relayout_operation_tree<subtree<I>(FoldedLayout)>(operation_tree));
            //                                                          ^^^^^^^ instead of get for msvc adl bug: https://gcc.godbolt.org/z/GYvdrbooW
        }(std::make_index_sequence<size<decltype(FoldedLayout)>>{});
    };
}

namespace senluo::detail::relayout_ns
{
    template<typename TBasePrinciple, auto FoldedLayout>
    struct principle_t : detail::based_on<TBasePrinciple>, principle_interface<principle_t<TBasePrinciple, FoldedLayout>>
    {
        friend constexpr decltype(auto) data(unwarp_derived_from<principle_t> auto&& self)
        {
            return data(FWD(self) | base);
        }
        
        static constexpr auto layout()
        {
            constexpr auto data_shape = shape<decltype(data(std::declval<TBasePrinciple>()))>;
            constexpr auto base_unfolded_layout = unfold_layout<TBasePrinciple::layout()>(data_shape);
            return fold_layout<apply_layout<FoldedLayout>(base_unfolded_layout)>(data_shape); 
        }
        
        static constexpr auto stricture_tree()
        { 
            return detail::fold_tag_tree<relayout_tag_tree<FoldedLayout>(TBasePrinciple::stricture_tree())>();
        }

        static constexpr auto operation_tree()
        {
            return detail::fold_operation_tree<relayout_tag_tree<FoldedLayout>(TBasePrinciple::operation_tree())>();
        }
    };

    template<typename T, auto FoldedLayout>
    struct tree_t : detail::based_on<T>, standard_interface<tree_t<T, FoldedLayout>>
    {
        template<size_t I, unwarp_derived_from<tree_t> Self> 
        friend constexpr decltype(auto) subtree(Self&& self)
        {
            constexpr auto sublayout = detail::layout_get<I>(FoldedLayout);
            if constexpr(detail::equal(sublayout, invalid_index))
            {
                return end();
            }
            else if constexpr(indexical<decltype(sublayout)>)
            {
                return FWD(self) | base | senluo::subtree<sublayout>;
                
            }
            else
            {
                return tree_t<unwrap_t<decltype(FWD(self) | base)>, sublayout>{ unwrap_fwd(FWD(self) | base) };
            }
        }

        template<auto UsageTree, unwarp_derived_from<tree_t> Self>
        friend constexpr decltype(auto) principle(Self&& self)
        {
            constexpr auto unfolded_layout = senluo::unfold_layout<FoldedLayout>(shape<T>);
            constexpr auto base_usage = inverse_relayout_usage_tree<unfolded_layout>(UsageTree, shape<T>);

            using base_principle_t = decltype(FWD(self) | base | senluo::principle<base_usage>);

            if constexpr(is_enable_to_relayout_operation_tree<FoldedLayout>(base_principle_t::operation_tree()))
            {
                return principle_t<base_principle_t, FoldedLayout>{ FWD(self) | base | senluo::principle<base_usage> };
            }
            else
            {
                using base_plain_principle_t = decltype(FWD(self) | base | plainize_principle<UsageTree>);

                return principle_t<base_plain_principle_t, FoldedLayout>{ 
                    FWD(self) | base | plainize_principle<UsageTree>
                };
            }           
        }

        friend constexpr auto get_maker(type_tag<tree_t>)
        requires (not std::same_as<decltype(detail::inverse_layout<unfold_layout<FoldedLayout>(shape<T>)>(shape<T>)), tuple<>>)
        {
            return []<class U>(U&& tree)
            {
                return tree_t{ 
                    FWD(tree) 
                    | relayout<detail::inverse_layout<unfold_layout<FoldedLayout>(shape<T>)>(shape<T>)> 
                    | senluo::make<T> 
                };
            };
        }
    };
}

namespace senluo
{
    template<auto Layout>
    struct detail::relayout_t : adaptor_closure<relayout_t<Layout>>
    {
        template<typename T>
        constexpr decltype(auto) operator()(T&& t)const
        {
            constexpr auto folded_layout = senluo::fold_layout<Layout>(shape<T>);
            if constexpr(indexical<decltype(folded_layout)>)
            {
                return decltype(wrap(subtree<folded_layout>(FWD(t)))){ unwrap_fwd(subtree<folded_layout>(FWD(t))) };
            }
            else
            {
                return relayout_ns::tree_t<senluo::unwrap_t<T>, folded_layout>{ unwrap_fwd(FWD(t)) };
            }
        }
    };

    template<typename T>
    constexpr auto default_unfolded_layout = []()
    {
        if constexpr (terminal<T>)
        {
            return indexes_of_whole;
        }
        else return[]<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(detail::layout_add_prefix(default_unfolded_layout<subtree_t<T, I>>, array{I})...);
        }(std::make_index_sequence<size<T>>{});    
    }();

    template<class Relayouter>
    struct relayouter_interface;

    template<class Relayouter>
    struct relayouter_interface : adaptor_closure<Relayouter>
    {
        template<typename T, derived_from<Relayouter> Self>
        constexpr auto operator()(this Self&& self, T&& tree)
        {
            constexpr auto layout = Relayouter::relayout(default_unfolded_layout<T>);
            return FWD(tree) | relayout<layout>;
        }
    };

    namespace detail
    {
        template<size_t N>
        struct repeat_t : relayouter_interface<repeat_t<N>>
        {
            static constexpr auto relayout(const auto&)
            {
                return []<size_t...I>(std::index_sequence<I...>)
                {
                    return tuple{ array<size_t, I - I>{}... };
                }(std::make_index_sequence<N>{});
            }
        };
    }

    template<size_t N>
    inline constexpr detail::repeat_t<N> repeat{};

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

    template<size_t I, size_t Axis>
    inline constexpr detail::component_t<I, Axis> component{}; 

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

    template<size_t Axis1 = 0uz, size_t Axis2 = Axis1 + 1uz>
    inline constexpr detail::transpose_t<Axis1, Axis2> transpose{}; 

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
        };
    }

    inline constexpr detail::inverse_t inverse{}; 

    namespace detail
    {
        struct combine_t
        {
            template<typename...T>
            constexpr auto operator()(T&&...t) const
            {
                return tuple<T...>{ FWD(t)... };
            }
        };
    }

    inline constexpr detail::combine_t combine{};  

    namespace detail
    {
        struct zip_t
        {
            template<typename...T>
            constexpr auto operator()(T&&...t) const
            {
                return tuple<T...>{ FWD(t)... } | transpose<>;
            }
        };
    }

    inline constexpr detail::zip_t zip{};    
}

#include "../macro_undef.hpp"
#endif
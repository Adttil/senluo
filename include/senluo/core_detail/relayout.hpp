#ifndef RUZHOUXIE_RELAYOUT_HPP
#define RUZHOUXIE_RELAYOUT_HPP

#include "../tree.hpp"
#include "../general.hpp"
#include "principle.hpp"
#include "wrap.hpp"

#include "../macro_define.hpp"

namespace senluo 
{
    template<size_t I, class L>
    constexpr auto layout_get(const L& layout)
    {
        if constexpr(indexical<L>)
        {
            return senluo::array_cat(layout, array{ I });
        }
        else if constexpr(I < size<L>)
        {
            return layout | subtree<I>;
        }
        else
        {
            return invalid_index;
        }
    }

    template<indexical_array auto indexes, class L>
    constexpr auto sublayout(const L& layout)
    {
        if constexpr(senluo::equal(indexes, indexes_of_whole))
        {
            return layout;
        }
        else
        {
            return sublayout<array_drop<1uz>(indexes)>(layout_get<indexes[0]>(layout));
        }
    }

    template<indexical auto Indices, class Shape>
    constexpr auto normalize_indices(Shape shape)
    {
        if constexpr(std::integral<decltype(Indices)>)
        {
            return array{ normalize_index(Indices, size<Shape>) };
        }
        else return []<size_t...I>(std::index_sequence<I...>)
        {
            return array<size_t, Indices.size()>
            {
                normalize_index(Indices[I], size<subtree_t<Shape, senluo::array_take<I>(Indices)>>)...
            };
        }(std::make_index_sequence<Indices.size()>{});
    }

    template<auto Layout, class Shape>
    constexpr auto fold_layout(Shape shape = {})
    {
        if constexpr(indexical_array<decltype(Layout)> || std::integral<decltype(Layout)>)
        {
            return normalize_indices<Layout>(shape);
        }
        else return []<size_t...I>(std::index_sequence<I...>)
        {
            constexpr auto child_relayout = senluo::make_tuple(fold_layout<Layout | subtree<I>>(Shape{})...);
            constexpr size_t n = size<decltype(child_relayout | subtree<0uz>)>;

            if constexpr(n > 0uz
                && (... && indexical_array<decltype(child_relayout | subtree<I>)>)
                && (... && (n == size<decltype(child_relayout | subtree<I>)>))
            )
            {
                constexpr auto prefix = senluo::array_take<n - 1uz>(child_relayout | subtree<0uz>);
                if constexpr((... && (prefix == senluo::array_take<n - 1uz>(child_relayout | subtree<I>)))
                    && (... && ((child_relayout | subtree<I>)[n - 1uz] == I))
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
                return senluo::make_tuple(unfold_layout<array_cat(indexes, array{ I })>(shape)...);
            }(std::make_index_sequence<size<subshape_t>>{});
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return senluo::make_tuple(unfold_layout<Layout | subtree<I>>(shape)...);
        }(std::make_index_sequence<size<decltype(Layout)>>{});
    }

    //unfold_layout<layout, shape>() == unfold_layout_by_relayouted_shape(layout, apply_layout<layout>(shape{}))
    template<class Shape, class Layout>
    constexpr auto unfold_layout_by_relayouted_shape(const Layout& layout, Shape shape = {})
    {
        if constexpr(terminal<Shape>)
        {
            static_assert(indexical<Layout>, "Invalid layout.");
            return to_indexes(layout);
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            if constexpr(not indexical<Layout>)
            {
                static_assert(size<Shape> == size<Layout>, "Invalid layout.");
                return senluo::make_tuple(unfold_layout_by_relayouted_shape(layout | subtree<I>, shape | subtree<I>)...);
            }
            else
            {
                auto indexes = to_indexes(layout);
                return senluo::make_tuple(unfold_layout_by_relayouted_shape(array_cat(indexes, array{ I }) , shape | subtree<I>)...);
            }
        }(std::make_index_sequence<size<Shape>>{});
    }

    template<auto Layout>
    constexpr auto apply_layout(const auto& view)
    {
        using layout_type = decltype(Layout);
        if constexpr(indexical<layout_type>)
        {
            return view | subtree<Layout>;
        }
        else return[&]<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(apply_layout<Layout | subtree<I>>(view)...);
        }(std::make_index_sequence<size<layout_type>>{});
    }

    template<auto Layout, class T>
    constexpr auto relayout_tag_tree(const T tag_tree)
    {
        if constexpr(indexical<decltype(Layout)>)
        {
            return tag_subtree<Layout>(tag_tree);
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return senluo::make_tuple(senluo::relayout_tag_tree<Layout | subtree<I>>(tag_tree)...);
        }(std::make_index_sequence<size<decltype(Layout)>>{});
    }

    template<auto UnfoldedLayout, typename U, typename R>
    constexpr void inverse_relayout_usage_tree_at(const U& usage_tree, R& result)
    {
        if constexpr(indexical<decltype(UnfoldedLayout)>)
        {
            auto&& subresult = result | subtree<UnfoldedLayout>;
            if constexpr(terminal<decltype(subresult)>)
            {
                subresult = subresult & usage_tree;
            }
            else return [&]<size_t...I>(std::index_sequence<I...>)
            {
                (..., inverse_relayout_usage_tree_at<indexes_of_whole>(usage_tree, subresult | subtree<I>));
            }(std::make_index_sequence<size<decltype(subresult)>>{});
        }
        else return[&]<size_t...I>(std::index_sequence<I...>)
        {
            (..., inverse_relayout_usage_tree_at<UnfoldedLayout | subtree<I>>(tag_tree_get<I>(usage_tree), result));
        }(std::make_index_sequence<size<decltype(UnfoldedLayout)>>{});
    }

    template<auto UnfoldedLayout, typename U, typename S>
    constexpr auto inverse_relayout_usage_tree(const U& usage_tree, const S& shape)
    {
        auto result = senluo::make_tree_of_same_value(usage_t::none, shape);
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
    //         return senluo::make_tuple(senluo::relayout_layout<Layout | subtree<I>>(layout)...);
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
                constexpr auto i = array_take<FoldedLayout.size() - 1uz>(FoldedLayout);
                return not std::same_as<decltype(senluo::tag_subtree<i>(operation_tree)), operation_t>
                || equal(tag_subtree<FoldedLayout>(operation_tree), operation_t::none);
            }
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return (... && senluo::is_enable_to_relayout_operation_tree<FoldedLayout | subtree<I>>(operation_tree));
        }(std::make_index_sequence<size<decltype(FoldedLayout)>>{});
    };
}

namespace senluo 
{
    template<typename TBasePrinciple, auto FoldedLayout>
    struct relayout_principle : based_on<TBasePrinciple>, principle_interface<relayout_principle<TBasePrinciple, FoldedLayout>>
    {
        constexpr decltype(auto) data(this auto&& self)
        {
            return FWD(self, base).data();
        }
        
        static constexpr auto layout()
        {
            constexpr auto data_shape = shape<decltype(std::declval<TBasePrinciple>().data())>;
            constexpr auto base_unfolded_layout = unfold_layout<TBasePrinciple::layout()>(data_shape);
            return fold_layout<apply_layout<FoldedLayout>(base_unfolded_layout)>(data_shape); 
        }
        
        static constexpr auto stricture_tree()
        { 
            return fold_tag_tree<relayout_tag_tree<FoldedLayout>(TBasePrinciple::stricture_tree())>();
        }

        static constexpr auto operation_tree()
        {
            return fold_operation_tree<relayout_tag_tree<FoldedLayout>(TBasePrinciple::operation_tree())>();
        }
    };

    template<typename T, auto FoldedLayout>
    struct relayout_tree : based_on<T>, standard_interface<relayout_tree<T, FoldedLayout>>
    {
    private:
        static constexpr auto unfolded_layout = senluo::unfold_layout<FoldedLayout>(shape<T>);
    
    public:
        template<size_t I, typename Self> 
        constexpr decltype(auto) get(this Self&& self)
        {
            constexpr auto sublayout = layout_get<I>(FoldedLayout);
            if constexpr(senluo::equal(sublayout, invalid_index))
            {
                return end();
            }
            else if constexpr(indexical<decltype(sublayout)>)
            {
                return FWD(self, base) | subtree<sublayout>;
                
            }
            else
            {
                return relayout_tree<decltype(FWD(self, base)), sublayout>{ FWD(self, base) };
            }
        }

        template<auto UsageTree, bool NoCopy, typename Self>
        constexpr decltype(auto) principle(this Self&& self)
        {
            constexpr auto base_usage = inverse_relayout_usage_tree<unfolded_layout>(UsageTree, shape<T>);

            using base_principle_t = decltype(FWD(self, base) | senluo::principle<base_usage, NoCopy>);

            if constexpr(is_enable_to_relayout_operation_tree<FoldedLayout>(base_principle_t::operation_tree()))
            {
                return relayout_principle<base_principle_t, FoldedLayout>{ FWD(self, base) | senluo::principle<base_usage, NoCopy> };
            }
            else
            {
                using base_plain_principle_t = plain_principle<decltype(FWD(self, base) | plainize<UsageTree>)>;

                return relayout_principle<base_plain_principle_t, FoldedLayout>{ 
                    base_plain_principle_t{ FWD(self, base) | plainize<UsageTree> }
                };
            }           
        }
    };

    namespace detail
    {
        template<auto Layout>
        struct relayout_t : adaptor_closure<relayout_t<Layout>>
        {
            template<typename T>
            constexpr decltype(auto) operator()(T&& t)const
            {
                constexpr auto folded_layout = senluo::fold_layout<Layout>(shape<T>);
                if constexpr(indexical<decltype(folded_layout)>)
                {
                    return decltype(wrap(FWD(t) | subtree<folded_layout>)){ unwrap_fwd(FWD(t) | subtree<folded_layout>) };
                }
                else
                {
                    return relayout_tree<senluo::unwrap_t<T>, folded_layout>{ unwrap_fwd(FWD(t)) };
                }
            }
        };
    }

    template<auto Layout>
    inline constexpr detail::relayout_t<Layout> relayout{};

    template<typename T>
    constexpr auto default_unfolded_layout = []()
    {
        if constexpr (terminal<T>)
        {
            return indexes_of_whole;
        }
        else return[]<size_t...I>(std::index_sequence<I...>)
        {
            return senluo::make_tuple(senluo::layout_add_prefix(default_unfolded_layout<subtree_t<T, I>>, array{I})...);
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
                    return tree | subtree<I>;
                }
                else
                {
                    static_assert(branched<T>, "Axis index out of range.");
                    return[&]<size_t...J>(std::index_sequence<J...>)
                    {
                        return make_tuple(component_t<I, Axis - 1uz>::relayout(tree | subtree<J>)...);
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
                    return senluo::make_tuple(transpose_t<Axis1 - 1uz, Axis2 - 1uz>::relayout(tree | subtree<I>)...);
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
                    return senluo::make_tuple(tree | subtree<last_index - I> ...);
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